#ifndef __LIRENA_CONFIG_H__
#define __LIRENA_CONFIG_H__


// Parse CLI options:

#include <stdlib.h>
#include <argp.h> // CLI arguments parsing


enum LirenaCaptureDeviceType
{
  LIRENA_CAPTURE_DEVICE_TYPE_invalid            = 0,
  LIRENA_CAPTURE_DEVICE_TYPE_gstTestSrc         = 1,
  LIRENA_CAPTURE_DEVICE_TYPE_XimeaCamera        = 2,
  LIRENA_CAPTURE_DEVICE_TYPE_MagewellEcoCapture = 3,
  LIRENA_CAPTURE_DEVICE_TYPE_MagewellProCapture = 4,
  LIRENA_CAPTURE_DEVICE_TYPE_NUM_DEVICE_TYPES   = 5
};





struct LirenaConfig
{
  // host and IP are mandatory params; host's IP has different
  // meaning in TCP than in UDP.
  char const *IP;
  char const *port;


  LirenaCaptureDeviceType captureDeviceType;


  // FOR DEBUGGING ONLY! Slows down pipeline, 
  // SEVERELY messing up UDP streaming to the point
  // of gstreamer refusing playback of dumped file
  // and even worse, on lost video frames possibly
  // destroying the precious 1:1 association of
  // video frames <--> KLV buffers !!
  // So REALLY ONLY USE FOR DEBUGGING, NOT IN PRODUCTION!!
  // default: false
  bool doLocalDisplay;
  // haveLocalGUI;

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

 
  // device dependent stuff:
  
  struct XimeaParams
  {
    //may be ignored in final implementation;
    // currently it is mostly a hacky way of 
    // roughly controlling capture framerate
    int exposure_ms;
    bool useCudaDemosaic;
  } ximeaparams;



};

//{ forwards:
LirenaConfig parseArguments(int argc, char **argv);
//}


//TODO outsource

class LirenaCaptureDevice;
// Factory function
LirenaCaptureDevice* lirena_createCaptureDevice(
  LirenaCaptureDeviceType type
);

class LirenaCaptureDevice
{
  public:

    explicit LirenaCaptureDevice(LirenaConfig const* config);
    virtual ~LirenaCaptureDevice();
    virtual LirenaCaptureDeviceType getType() = 0;
    virtual bool openDevice() = 0;
    virtual bool setupParams() = 0;
    virtual bool startVideoAquisition() = 0;
    virtual bool getFrame();
    virtual bool calcTiming();
    
    bool acquireFrameMetadata();
    bool pushFrameMetaDataToGstreamer();
    virtual bool pushVideoFrameToGstreamer();


  protected: 
    LirenaConfig const * m_config;
};

class LirenaXimeaCaptureDevice : public LirenaCaptureDevice
{
  virtual ~LirenaXimeaCaptureDevice();
  virtual LirenaCaptureDeviceType getType();

};

class LirenaMagewellEcoCaptureDevice : public LirenaCaptureDevice
{
  virtual ~LirenaMagewellEcoCaptureDevice();
  virtual LirenaCaptureDeviceType getType();

};


#endif //__LIRENA_CONFIG_H__
