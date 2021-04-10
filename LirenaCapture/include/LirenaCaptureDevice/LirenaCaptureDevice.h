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

    LirenaCaptureDeviceType getType() {
      return deviceType;
    }

    // dummy implementations to check workflow
    
    //setup
    virtual bool openDevice() {return false;}
    virtual bool setupParams()  {return false;}
    virtual bool startVideoAquisition()  {return false;}

    // acquisition, proecessing, enhancing (w/ metadata), "publishing"
    virtual bool getFrame() {return false;}
    virtual bool postProcessFrame() {return false;}
    virtual bool calcTiming() {return false;}
    virtual bool acquireFrameMetadata() {return false;}
    
    bool pushFrameMetaDataToGstreamer() {return false;}
    virtual bool pushVideoFrameToGstreamer() {return false;}


  protected: 
    LirenaConfig  * configPtr = nullptr;
    LirenaCaptureDeviceType deviceType = LIRENA_CAPTURE_DEVICE_TYPE_invalid;
};


//TODO outsource
class LirenaXimeaCaptureDevice : public LirenaCaptureDevice
{
  public:

    explicit LirenaXimeaCaptureDevice(LirenaConfig * configPtr);


    virtual ~LirenaXimeaCaptureDevice();


};

//TODO outsource
class LirenaMagewellEcoCaptureDevice : public LirenaCaptureDevice
{
  public:
  
    explicit LirenaMagewellEcoCaptureDevice(LirenaConfig * configPtr);
  

    virtual ~LirenaMagewellEcoCaptureDevice();


};

#endif // __LIRENA_CAPTURE_DEVICE_H__