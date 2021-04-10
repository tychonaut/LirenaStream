#include "LirenaCaptureDevice/LirenaCaptureDevice.h"

#include <glib.h> //g_assert


LirenaCaptureDevice* LirenaCaptureDevice::createCaptureDevice(
    LirenaConfig * configPtr 
)
{
    switch (configPtr->captureDeviceType)
    {
    case LIRENA_CAPTURE_DEVICE_TYPE_XimeaCamera:
        return new LirenaXimeaCaptureDevice(configPtr);
        break;

    case LIRENA_CAPTURE_DEVICE_TYPE_MagewellEcoCapture:
        return new LirenaMagewellEcoCaptureDevice(configPtr);
        break;

    default:
        g_assert(0 && "Capture device type unsupported yet! Exiting");
        exit(1);
        break;
    }
}

LirenaCaptureDevice::LirenaCaptureDevice(LirenaConfig *configPtr)
    : configPtr(configPtr)
{
}

LirenaCaptureDevice::~LirenaCaptureDevice()
{
}



//TODO outsource
LirenaXimeaCaptureDevice::LirenaXimeaCaptureDevice(LirenaConfig * configPtr)
    : LirenaCaptureDevice(configPtr)
{
}


LirenaXimeaCaptureDevice::~LirenaXimeaCaptureDevice()
{

}


LirenaMagewellEcoCaptureDevice::LirenaMagewellEcoCaptureDevice(
    LirenaConfig * configPtr)
    : LirenaCaptureDevice(configPtr)
{
}


LirenaMagewellEcoCaptureDevice::~LirenaMagewellEcoCaptureDevice()
{

}

