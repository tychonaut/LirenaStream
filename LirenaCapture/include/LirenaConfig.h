#ifndef __LIRENA_CONFIG_H__
#define __LIRENA_CONFIG_H__


// Parse CLI options:

#include <stdlib.h>
#include <argp.h> // CLI arguments parsing


// LirenaCaptureDeviceType has te be defined before 
// LirenaConfig if not wanting to use ugly typedefs to int
enum LirenaCaptureDeviceType
{
  LIRENA_CAPTURE_DEVICE_TYPE_invalid            = 0,
  
  //gstreamer dummy (untested yet)
  LIRENA_CAPTURE_DEVICE_TYPE_videotestsrc       = 1,
  //gstreamer dummy (untested yet)
  LIRENA_CAPTURE_DEVICE_TYPE_XimeaCamera        = 2,
  //gstreamer dummy (untested yet)
  LIRENA_CAPTURE_DEVICE_TYPE_MagewellEcoCapture = 3,
  //gstreamer dummy (not supported yet)
  LIRENA_CAPTURE_DEVICE_TYPE_MagewellProCapture = 4,

  LIRENA_CAPTURE_DEVICE_TYPE_NUM_DEVICE_TYPES   = 5
};



struct LirenaConfig
{
  // host and IP are mandatory params; host's IP has different
  // meaning in TCP than in UDP.
  char const *IP;
  char const *port;

  // default: XimeaCamera
  LirenaCaptureDeviceType captureDeviceType;

  // Resolution params may be ignored depending 
  // on device type and capturing mode
  // default: -1, meaning automatic
  int targetResolutionX;
  int targetResolutionY;

  // FPS param may be ignored depending on capturing mode
  // default: -1, meaning automatic
  int targetFPS;


  // FOR DEBUGGING ONLY! Slows down pipeline, 
  // SEVERELY messing up UDP streaming to the point
  // of gstreamer refusing playback of dumped file
  // and even worse, on lost video frames possibly
  // destroying the precious 1:1 association of
  // video frames <--> KLV buffers !!
  // So REALLY ONLY USE FOR DEBUGGING, NOT IN PRODUCTION!!
  // default: false
  bool doLocalDisplay;
  bool haveLocalGUI;

  // FOR DEBUGGING ONLY! Same warning as with local display!
  // default: NULL
  char const *outputFile;
  
  // experimental, in case UDP makes too many problems with 
  // corrupt/lost frames, possibly prohibiting video replay
  // and especially destroying the implicit 1:1 association 
  // of video frames and KLV buffers (they are treated as 
  // independent entities by the current implementation
  // of mpegtsmux and tsdemux!)
  // default: false
  bool useTCP; 

 



  // device dependent stuff to come: ---
  
  struct XimeaParams
  {
    //may be ignored in final implementation;
    // currently it is mostly a hacky way of 
    // roughly controlling capture framerate
    int exposure_ms;
    // default: false (at the moment,
    // but will change as soon as implemented)
    bool useCudaDemosaic;
  } ximeaparams;



};

//{ forwards:
LirenaConfig parseArguments(int argc, char **argv);
//}





#endif //__LIRENA_CONFIG_H__
