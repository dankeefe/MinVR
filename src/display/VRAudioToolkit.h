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

#ifndef VRAUDIOTOOLKIT_H
#define VRAUDIOTOOLKIT_H

#include <main/VRError.h>


namespace MinVR {

/** Abstract base class for audio toolkits (OpenAL, PortAudio, SDL, etc.) that are 
    implemented in plugins.
 */
class VRAudioToolkit {
public:
  virtual ~VRAudioToolkit() {}

  static std::string getAttributeName() { return "audiotoolkitType"; };

  /// name of the toolkit implementation (OpenAL, PortAudio, SDL, ...)
  virtual std::string getName() const = 0;


  /** LISTENER PROPERTIES **/

  /// Position of the virtual listener, often set by head tracking data.
  virtual void setListenerPos(float x, float y, float z) {
    VRWARNINGNOADV("setListenerPos() not enabled in this VRAudioToolkit.");
  }

  /// Velocity of the virtual listener.
  virtual void setListenerVel(float x, float y, float z) {
    VRWARNINGNOADV("setListenerVel() not enabled in this VRAudioToolkit.");
  }

  /// Gain for the virtual listener, like an overall volume knob.
  virtual void setListenerGain(float g) {
    VRWARNINGNOADV("setListenerGain() not enabled in this VRAudioToolkit.");
  }


  /** SOURCE PROPERTIES **/

  /// Load an audio file into memory.  Returns an id for the audio file.
  virtual int loadSource(const std::string &filename) {
    VRWARNINGNOADV("loadSource() not enabled in this VRAudioToolkit.");
    return -1;
  };

  /// Start playing an audio file -- must already be loaded.
  virtual void playSource(int id, bool restart = true) {
    VRWARNINGNOADV("playSource() not enabled in this VRAudioToolkit.");
  }

  /// Position of the source -- source audio file must already be loaded.
  virtual void setSourcePos(int id, float x, float y, float z) {
    VRWARNINGNOADV("setSourcePos() not enabled in this VRAudioToolkit.");
  }

  /// Velocity of the source -- source audio file must already be loaded.
  virtual void setSourceVel(int id, float x, float y, float z) {
    VRWARNINGNOADV("setSourceVel() not enabled in this VRAudioToolkit.");
  }

  /// Set the source gain (the volume knob for this source) -- audio file must already be loaded.
  virtual void setSourceGain(int id, float gain) {
    VRWARNINGNOADV("setSourceGain() not enabled in this VRAudioToolkit.");
  }

  /// Stop playing the source's audio file.
  virtual void stopSource(int id) {
    VRWARNINGNOADV("stopSource() not enabled in this VRAudioToolkit.");
  }

  /// Remove the source, and delete the data read in from its audio file.
  virtual void deleteSource(int id) {
    VRWARNINGNOADV("deleteSource() not enabled in this VRAudioToolkit.");
  }

};

} // end namespace

#endif
