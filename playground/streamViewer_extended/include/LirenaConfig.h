#ifndef __LIRENA_CONFIG_H__
#define __LIRENA_CONFIG_H__


// Parse CLI options:

#include <stdlib.h>
#include <argp.h> // CLI arguments parsing


// https://www.ximea.com/support/wiki/apis/xiapi_manual#XI_PRM_GPI_SELECTOR-or-gpi_selector
// Some kind of general purpose input state:
// don't really get it yet, was 0 for GPI selector 1-4 on test cam, threw error on
// selector 4, hinting at "0-3". Awkward, useless so far, but keep in mind that
// with new cam and active synchronized triggering, this may become very important!
#define LIRENA_XIMEA_MAX_GPI_SELECTORS 4



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



class LirenaConfig
{
  public:

  explicit LirenaConfig(int argc, char **argv);

  
  int getStreamingResolutionX() const;
  int getStreamingResolutionY() const;


  //keep to init gtk, gst etc..
  int argc;
  char **argv;

  // host and IP are mandatory params; host's IP has different
  // meaning in TCP than in UDP.
  char const *IP;
  char const *port;

  // default: XimeaCamera
  LirenaCaptureDeviceType captureDeviceType;

  // add KLV meta data to each H264 encoded video frame
  // via GStreamer muxing into MP2TS container format
  // default: true
  bool injectKLVmeta;

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

    // Warning! set only after opening and configuring device!
    int activeSensorResolutionX = 0;
    int activeSensorResolutionY = 0;
  } 
  ximeaparams;



};

//{ forwards:

//}





#endif //__LIRENA_CONFIG_H__