#ifndef __LIRENA_CAPTURE_DEVICE_H__
#define __LIRENA_CAPTURE_DEVICE_H__

#include "LirenaConfig.h"


class LirenaCaptureDevice;
class LirenaConfig;








//Abstract base class
class LirenaCaptureDevice
{
  public:
    // Factory function
    static LirenaCaptureDevice* createCaptureDevice(
      LirenaConfig * configPtr 
    );

  protected:
    // do not instanciate directly 
    // (pure abstractness omitted during refactoring)
    explicit LirenaCaptureDevice(LirenaConfig * configPtr);
  
  public:
    virtual ~LirenaCaptureDevice();

    LirenaCaptureDeviceType getType() 
    {
      return deviceType;
    }

    // needed for GStreamer pipeline management:
    bool typeImpliesGstAppSrc()
    {
      return 
        (deviceType == LIRENA_CAPTURE_DEVICE_TYPE_XimeaCamera)
        ||
        (deviceType == LIRENA_CAPTURE_DEVICE_TYPE_MagewellEcoCapture)
        ||
        (deviceType == LIRENA_CAPTURE_DEVICE_TYPE_MagewellProCapture)
        ;
    }

    // dummy implementations to check workflow
    
    //setup
    virtual bool openDevice() {return false;}
    virtual bool setupParams()  {return false;}
    virtual bool startVideoAquisition() {return false;}

    // acquisition, processing (e.g. debayer), enhancing (w/ metadata),
    virtual bool getFrame() {return false;}
    virtual bool postProcessFrame() {return false;}
    virtual bool calcTiming() {return false;}
    virtual bool acquireFrameMetadata() {return false;}
    
    // "publishing"
    bool pushFrameMetaDataToGstreamer() {return false;}
    virtual bool pushVideoFrameToGstreamer() {return false;}


  protected: 
    LirenaConfig  * configPtr = nullptr;

  private:
    LirenaCaptureDeviceType deviceType = LIRENA_CAPTURE_DEVICE_TYPE_invalid;
};


//TODO outsource
class LirenaXimeaCaptureDevice : public LirenaCaptureDevice
{
  public:

    explicit LirenaXimeaCaptureDevice(LirenaConfig * configPtr);

    virtual ~LirenaXimeaCaptureDevice();

    virtual bool openDevice();
    virtual bool setupParams();
    virtual bool startVideoAquisition();

    // acquisition, processing (e.g. debayer), enhancing (w/ metadata),
    virtual bool captureFrame();
    virtual bool postProcessFrame();
    virtual bool getFrame();
    //virtual bool calcTiming();
    virtual bool acquireFrameMetadata();
    
    // "publishing"
    bool pushFrameMetaDataToGstreamer();
    virtual bool pushVideoFrameToGstreamer();

};



//LirenaGstVideotestsrcCaptureDevice
class LirenaGstVideotestsrcCaptureDevice : public LirenaCaptureDevice
{
  public:

    explicit LirenaGstVideotestsrcCaptureDevice(LirenaConfig * configPtr);

    virtual ~LirenaGstVideotestsrcCaptureDevice();



    virtual bool openDevice();
    virtual bool setupParams();
    virtual bool startVideoAquisition();

    // acquisition, processing (e.g. debayer), enhancing (w/ metadata),
    virtual bool getFrame();
    virtual bool postProcessFrame();
    virtual bool calcTiming();
    virtual bool acquireFrameMetadata();
    
    // "publishing"
    bool pushFrameMetaDataToGstreamer();
    virtual bool pushVideoFrameToGstreamer();

};



//TODO outsource
class LirenaMagewellEcoCaptureDevice : public LirenaCaptureDevice
{
  public:
  
    explicit LirenaMagewellEcoCaptureDevice(LirenaConfig * configPtr);
  
    virtual ~LirenaMagewellEcoCaptureDevice();

    //TODO add virtual functions

};

#endif // __LIRENA_CAPTURE_DEVICE_H__