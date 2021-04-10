#ifndef __LIRENA_CAPTURE_DEVICE_H__
#define __LIRENA_CAPTURE_DEVICE_H__

#include "LirenaConfig.h"


class LirenaCaptureDevice;
class LirenaConfig;






// Factory function
LirenaCaptureDevice* lirena_createCaptureDevice(
  LirenaCaptureDeviceType type
);

//Abstract base class
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


//TODO outsource
class LirenaXimeaCaptureDevice : public LirenaCaptureDevice
{
  virtual ~LirenaXimeaCaptureDevice();
  virtual LirenaCaptureDeviceType getType();

};

//TODO outsource
class LirenaMagewellEcoCaptureDevice : public LirenaCaptureDevice
{
  virtual ~LirenaMagewellEcoCaptureDevice();
  virtual LirenaCaptureDeviceType getType();

};

#endif // __LIRENA_CAPTURE_DEVICE_H__