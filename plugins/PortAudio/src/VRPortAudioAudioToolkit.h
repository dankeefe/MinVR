/** 
This file is part of the MinVR Open Source Project, which is developed and 
maintained collaboratively by the University of Minnesota and Brown University.

Copyright (c) 2016 Regents of the University of Minnesota and Brown University.
This software is distributed under the BSD-3 Clause license, which can be found
at: MinVR/LICENSE.txt.

Original Author(s) of this File: 
  Dan Keefe, 2017, University of Minnesota
  
Author(s) of Significant Updates/Modifications to the File:
  ... 
*/

#ifndef VRPORTAUDIOAUDIOTOOLKIT_H
#define VRPORTAUDIOAUDIOTOOLKIT_H

#include <display/VRAudioToolkit.h>
#include <main/VRError.h>
#include <main/VRFactory.h>
#include <main/VRMainInterface.h>
#include <plugin/VRPlugin.h>

#include <portaudio.h>


namespace MinVR {

/** Abstract base class for audio toolkits (OpenAL, PortAudio, SDL, etc.) that are 
    implemented in plugins.
 */
class VRPortAudioAudioToolkit : public VRAudioToolkit {
public:

  PLUGIN_API VRPortAudioAudioToolkit(VRMainInterface *vrMain,
                                     const std::string &audioDeviceName,
                                     const std::vector<VRPoint3> &speakerPositions);
    
  PLUGIN_API virtual ~VRPortAudioAudioToolkit();

  /// name of the toolkit implementation
  PLUGIN_API std::string getName() const { return "VRPortAudioAudioToolkit"; }


  PLUGIN_API void mainloop();
  

  /** LISTENER PROPERTIES **/

  /// Position of the virtual listener, often adjusted due to head tracking data, but
  /// the position set here should be in world space, i.e., the same coordinate system
  /// as what is used to set the positions of the audio sources. 
  PLUGIN_API void setListenerPos(float x, float y, float z);

  /// Velocity of the virtual listener.
  PLUGIN_API void setListenerVel(float x, float y, float z);

  /// Gain for the virtual listener, like an overall volume knob.
  PLUGIN_API void setListenerGain(float g);


  /** SOURCE PROPERTIES **/

  /// Load an audio file into memory.  Returns an id for the audio file.
  PLUGIN_API int loadSource(const std::string &filename);

  /// Start playing an audio file -- must already be loaded.  If the source is already playing
  /// then it will restart at the beginning of the audio clip if restart=true.
  PLUGIN_API void playSource(int id, bool restart = true);

  /// Position of the source -- source audio file must already be loaded.
  PLUGIN_API void setSourcePos(int id, float x, float y, float z);

  /// Velocity of the source -- source audio file must already be loaded.
  PLUGIN_API void setSourceVel(int id, float x, float y, float z);

  /// Set the source gain (the volume knob for this source) -- audio file must already be loaded.
  PLUGIN_API void setSourceGain(int id, float gain);

  /// Stop playing the source's audio file.
  PLUGIN_API void stopSource(int id);



  PLUGIN_API int paCallback(const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer,
                            const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags);
    
  PLUGIN_API static int staticPACallback(const void *inputBuffer, void *outputBuffer,
                                         unsigned long framesPerBuffer,
                                         const   PaStreamCallbackTimeInfo* timeInfo,
                                         PaStreamCallbackFlags statusFlags,
                                         void *userData);

  PLUGIN_API static VRAudioToolkit* create(VRMainInterface *vrMain, VRDataIndex *config, const std::string &nameSpace);

    
  PLUGIN_API void renderSpatialAudio(float *modelMatrix);
    
    
    
private:

  VRMainInterface *_vrMain;

  int _devID;
  PaStream *_stream;

  // one output channel goes to each physical speaker, positions are specified in room space
  std::vector<VRPoint3> _speakerPositions;
  std::vector<float> _channelGains;

  // data for the listener:
  float     _lgain;
  VRPoint3  _lpos;
  VRVector3 _lvel;

  // data for each source:
  std::vector<float*>    _dataPtr;
  std::vector<int>       _dataSize;
  std::vector<int>       _frame;
  std::vector<float>     _gain;
  std::vector<VRPoint3>  _pos;
  std::vector<VRVector3> _vel;

};

} // end namespace

#endif
