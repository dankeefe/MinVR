
#include "VRPortAudioAudioToolkit.h"
#include <main/VRError.h>
#include <sndfile.h>



namespace MinVR {


#define SAMPLE_RATE   (44100)
#define FRAMES_PER_BUFFER  (64)

struct InitSingleton {
  InitSingleton(): mCleanUp(paNoError == Pa_Initialize()) {}
  ~InitSingleton() { if (mCleanUp) { Pa_Terminate(); } }
  bool mCleanUp;
};


// https://stackoverflow.com/questions/216823/whats-the-best-way-to-trim-stdstring

// trim from start (in place)
static inline void ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(),
                                    std::not1(std::ptr_fun<int, int>(std::isspace))));
}

// trim from end (in place)
static inline void rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(),
                         std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
}

// trim from both ends (in place)
static inline void trim(std::string &s) {
    ltrim(s);
    rtrim(s);
}
    

VRPortAudioAudioToolkit::VRPortAudioAudioToolkit(VRMainInterface *vrMain, const std::string &audioDeviceName, const std::vector<VRPoint3> &speakerPositions) : _vrMain(vrMain) {
  for (int i=0; i<speakerPositions.size(); i++) {
    _channelGains.push_back(1.0);
  }

  // Used to make sure we only init and terminate portaudio once
  static InitSingleton dummy;

  VRLOG_H2("Creating VRPortAudioToolkit");

  // Find which index the desired audio device is attached to. 
  // Make sure that you pick the ASIO version of the device to get all of the output channels
  int numDevices = Pa_GetDeviceCount();
  _devID = -1;
  VRLOG_STATUS("Found " + std::to_string(numDevices) + " AudioDevices:");
  for (int i=0; i<numDevices; i++ ) {
    const PaDeviceInfo* deviceInfo = Pa_GetDeviceInfo(i);
    string name = deviceInfo->name;
    trim(name);
    VRLOG_STATUS(std::to_string(i) + ": " + deviceInfo->name);
    if (name == audioDeviceName) {
      _devID = i;
      break;
    }
  }

  if (_devID == -1) {
    VRERROR("No audio device named: " + audioDeviceName, "See the list of available devices above.");
  }

  PaStreamParameters outputParameters;
  outputParameters.device = _devID;
  outputParameters.channelCount = speakerPositions.size();
  outputParameters.sampleFormat = paFloat32; /* 32 bit floating point output */
  outputParameters.suggestedLatency = Pa_GetDeviceInfo(outputParameters.device)->defaultLowOutputLatency;
  outputParameters.hostApiSpecificStreamInfo = NULL;

  PaError err = Pa_OpenStream(
      &_stream,
      NULL, /* no input */
      &outputParameters,
      SAMPLE_RATE,
      FRAMES_PER_BUFFER,
      paClipOff,      /* we won't output out of range samples so don't bother clipping them */
      &VRPortAudioAudioToolkit::staticPACallback,
      this            /* Using 'this' for userData so we can cast to SpatialAudio* in paCallback method */
  );

  if (err != paNoError) {
    /* Failed to open stream to device !!! */
    VRERRORNOADV("OpenStream failed for the audio device.");
  }
}
  


VRPortAudioAudioToolkit::~VRPortAudioAudioToolkit() {
  if (_stream != 0) {
    PaError err = Pa_StopStream(_stream);
    if (err != paNoError) {
      VRERRORNOADV("StopStream failed for the audio device.");
    }

    err = Pa_CloseStream(_stream);
    if (err != paNoError) {
      VRERRORNOADV("CloseStream failed for the audio device.");
    }
  }

  for (int i=0;i<_dataPtr.size();i++) {
    delete [] _dataPtr[i];
  }

  Pa_Terminate();
}



/** LISTENER PROPERTIES **/

void VRPortAudioAudioToolkit::setListenerPos(float x, float y, float z) {
    _lpos = VRPoint3(x,y,z);
}

void VRPortAudioAudioToolkit::setListenerVel(float x, float y, float z) {
    _lvel = VRVector3(x,y,z);
}

void VRPortAudioAudioToolkit::setListenerGain(float g) {
    _lgain = g;
}


/** SOURCE PROPERTIES **/

int VRPortAudioAudioToolkit::loadSource(const std::string &filename) {
  
  SF_INFO sfInfo;
  SNDFILE* sndFile = sf_open(filename.c_str(), SFM_READ, &sfInfo);
  if (!sndFile) {
    VRERROR("Error opening sound file: " + filename, "Make sure the file exists.");
  }

  if (sfInfo.channels != 1) {
    VRERROR("The VRPortAudio AudioToolkit only reads mono audio files.",
      "File " + filename + " appears to contain more than one audio channel.");
  }

  int fileDataSize = sfInfo.channels * sfInfo.frames;
  float *fileData = new float[fileDataSize];
  sf_seek(sndFile, 0, SEEK_SET);
  sf_read_float(sndFile, fileData, fileDataSize);
  sf_close(sndFile);

  _dataPtr.push_back(fileData);
  _dataSize.push_back(fileDataSize);
  _frame.push_back(-1);
  _gain.push_back(1.0);
  _pos.push_back(VRPoint3(0,0,0));
  _vel.push_back(VRVector3(0,0,0));

  return _dataPtr.size()-1;
}

void VRPortAudioAudioToolkit::playSource(int id, bool restart) {
  if (_stream == 0) {
    PaError err = Pa_StartStream(_stream);
    if (err != paNoError) {
      VRERRORNOADV("VRPortAudio AudioToolkit error on StartStream.");
    }
  }

  if ((_frame[id] == -1) || (restart)) {
    _frame[id] = 0;
  }
}

void VRPortAudioAudioToolkit::setSourcePos(int id, float x, float y, float z) {
    _pos[id] = VRPoint3(x,y,z);
}

void VRPortAudioAudioToolkit::setSourceVel(int id, float x, float y, float z) {
    _vel[id] = VRVector3(x,y,z);
}

void VRPortAudioAudioToolkit::setSourceGain(int id, float gain) {
    _gain[id] = gain;
}

void VRPortAudioAudioToolkit::stopSource(int id) {
    _frame[id] = -1;
}


// modelmatrix = virtual points to room space points
void VRPortAudioAudioToolkit::renderSpatialAudio(float *modelMatrix) {
  // Update gains based on current virtual positions relative to roomspace speakers

  VRMatrix4 worldToRoom = VRMatrix4(modelMatrix).inverse();
  VRPoint3 roomSpaceListener = _lpos;
    
  for (int i=0; i<_speakerPositions.size(); i++) {
    VRPoint3 roomSpaceSpeaker = _speakerPositions[i];

    for (int j=0; j<_pos.size(); j++) {
      VRPoint3 roomSpaceSource = worldToRoom * _pos[j];
        
      VRVector3 toSpeaker = (roomSpaceSpeaker - roomSpaceListener).normalize();
      VRVector3 toSource = (roomSpaceSource - roomSpaceListener).normalize();

      float alignmentMeasure = toSpeaker.dot(toSource);
      float gain = 0.0;
      if (alignmentMeasure > 0.0) {
        float dist = (roomSpaceSource - roomSpaceListener).length();

        const float maxSoundDist = 500.0f;
      
        /*
        //scale to 1-50 range
        dist = (49*dist)/maxSoundDist+1;
        // gain adjusted based on some combination of 1/(dist^2) and also the direction to the source
        // might need to tweak this...
        if (dist == 0.0) {
          gain = 1.0;
        }else {
          gain = 1 / (dist * dist);
        }
        */

        //just scale linearly
        gain = std::max(1.0 - dist/maxSoundDist, 0.0);


        //float boost = 10.0;
        gain *= alignmentMeasure;

        _channelGains[i] = gain;
      }
      else {
        _channelGains[i] = 0.0;
      }
    }
  }
}



int VRPortAudioAudioToolkit::paCallback(const void *inputBuffer, void *outputBuffer,
                                        unsigned long framesPerBuffer,
                                        const PaStreamCallbackTimeInfo* timeInfo,
                                        PaStreamCallbackFlags statusFlags)
{
    float *out = (float*)outputBuffer;
  
    (void) timeInfo; /* Prevent unused variable warnings. */
    (void) statusFlags;
    (void) inputBuffer;


    // add framesPerBuffer worth of new sound data to each output buffer
    for (unsigned long f = 0; f < framesPerBuffer; f++) {
        
        // loop through all of the output channels
        for (int c = 0; c < _channelGains.size(); c++) {

            // add contributions from each sound source
            for (int s = 0; s < _dataPtr.size(); s++) {
                if (s == 0) {
                    *out = _dataPtr[s][_frame[s]] * _gain[s] * _channelGains[c];
                }
                else {
                    *out += _dataPtr[s][_frame[s]] * _gain[s] * _channelGains[c];
                }
            }

            // kill pesky nans so we don't hurt anyone's ears
            if (*out != *out) {
                *out = 0.f; // portable isnan; only nans do not equal themselves
            }
            // clip
            if (*out < -1.f) {
                *out = -1.f;
            }
            else if (*out > 1.f) {
                *out = 1.f;
            }

            // move to the next channel
            out++;
        }
        
        // advance the frame counter for each source
        for (int s = 0; s < _frame.size(); s++) {
            _frame[s]++;
            if (_frame[s] >= _dataSize[s]) {
                _frame[s] = 0;
            }
        }
    }
  
    return paContinue;
}


/* This routine will be called by the PortAudio engine when audio is needed.
** It may called at interrupt level on some machines so don't do anything
** that could mess up the system like calling malloc() or free().
*/
int VRPortAudioAudioToolkit::staticPACallback(const void *inputBuffer,
                                              void *outputBuffer,
                                              unsigned long framesPerBuffer,
                                              const PaStreamCallbackTimeInfo* timeInfo,
                                              PaStreamCallbackFlags statusFlags,
                                              void *userData)
{
    /* Here we cast userData to VRPortAudioAudioToolkit* type so we can call the instance method paCallbackMethod, we can do that since 
        we called Pa_OpenStream with 'this' for userData */
    return ((VRPortAudioAudioToolkit*)userData)->paCallback(inputBuffer, outputBuffer, framesPerBuffer, timeInfo, statusFlags);
}



// Example:  The IV/LAB Cave speakers are mapped to channels 0-7.
// 
//    6-------7
//   /|      /|
//  1-------0 |
//  | |     | |
//  | |5----|-|4
//  |/      |/
//  3-------2
//    front
//
//  AudioDeviceName = "ASIO Hammerfall DSP"
//
//  SpeakerPos_0 =  4, 4, 4
//  SpeakerPos_1 = -4, 4, 4
//  SpeakerPos_2 =  4,-4, 4
//  SpeakerPos_3 = -4,-4, 4
//  SpeakerPos_4 =  4,-4,-4
//  SpeakerPos_5 = -4,-4,-4
//  SpeakerPos_6 = -4, 4,-4
//  SpeakerPos_7 =  4, 4,-4 


VRAudioToolkit*
VRPortAudioAudioToolkit::create(VRMainInterface *vrMain, VRDataIndex *config, const std::string &nameSpace) {
    std::string audioDeviceName = config->getValue("AudioDeviceName", nameSpace);
    int nSpeakers = config->getValue("NumSpeakers", nameSpace);

    std::vector<VRPoint3> speakerPositions;
    for (int i=0; i<nSpeakers; i++) {
        VRPoint3 pos = config->getValue("SpeakerPos_" + std::to_string(i), nameSpace);
        speakerPositions.push_back(pos);
    }

    return new VRPortAudioAudioToolkit(vrMain, audioDeviceName, speakerPositions);
}




} // end namespace
