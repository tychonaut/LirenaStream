#include "LirenaCaptureDevice/LirenaCaptureDevice.h"

#include <glib.h> //g_assert


LirenaCaptureDevice* LirenaCaptureDevice::createCaptureDevice(
    LirenaConfig * configPtr 
)
{
    switch (configPtr->captureDeviceType)
    {
    case LIRENA_CAPTURE_DEVICE_TYPE_videotestsrc:
        return new LirenaGstVideotestsrcCaptureDevice(configPtr);
        break;
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
    : configPtr(configPtr),
    deviceType(configPtr->captureDeviceType)
{
}

LirenaCaptureDevice::~LirenaCaptureDevice()
{
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// subclass impls.
// TODO outsource


LirenaGstVideotestsrcCaptureDevice::LirenaGstVideotestsrcCaptureDevice(
    LirenaConfig * configPtr)
    : LirenaCaptureDevice(configPtr)
{
}


LirenaGstVideotestsrcCaptureDevice::~LirenaGstVideotestsrcCaptureDevice()
{

}

//setup
bool LirenaGstVideotestsrcCaptureDevice::openDevice() 
{
    g_assert(0 && "TODO IMPLEMENT");
    return false;
}

bool LirenaGstVideotestsrcCaptureDevice::setupParams()  
{
    g_assert(0 && "TODO IMPLEMENT");
    return false;
}

bool LirenaGstVideotestsrcCaptureDevice::startVideoAquisition()  
{
    g_assert(0 && "TODO IMPLEMENT");
    return false;
}

    // acquisition, processing (e.g. debayer), enhancing (w/ metadata),
bool LirenaGstVideotestsrcCaptureDevice::getFrame()  
{
    g_assert(0 && "TODO IMPLEMENT");
    return false;
}

bool LirenaGstVideotestsrcCaptureDevice::postProcessFrame()  
{
    g_assert(0 && "TODO IMPLEMENT");
    return false;
}

bool LirenaGstVideotestsrcCaptureDevice::calcTiming()  
{
    g_assert(0 && "TODO IMPLEMENT");
    return false;
}

bool LirenaGstVideotestsrcCaptureDevice::acquireFrameMetadata()  
{
    g_assert(0 && "TODO IMPLEMENT");
    return false;
}
    
// "publishing"
bool LirenaGstVideotestsrcCaptureDevice::pushFrameMetaDataToGstreamer()  
{
    g_assert(0 && "TODO IMPLEMENT");
    return false;
}

bool LirenaGstVideotestsrcCaptureDevice::pushVideoFrameToGstreamer()  
{
    g_assert(0 && "TODO IMPLEMENT");
    return false;
}




//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

LirenaXimeaCaptureDevice::LirenaXimeaCaptureDevice(LirenaConfig * configPtr)
    : LirenaCaptureDevice(configPtr)
{
    g_assert(0 && "TODO IMPLEMENT");
}


LirenaXimeaCaptureDevice::~LirenaXimeaCaptureDevice()
{
    g_assert(0 && "TODO IMPLEMENT");
}


//setup
bool LirenaXimeaCaptureDevice::openDevice() 
{
    g_assert(0 && "TODO IMPLEMENT");
    return false;
}

bool LirenaXimeaCaptureDevice::setupParams()  
{
    g_assert(0 && "TODO IMPLEMENT");
    return false;
}

bool LirenaXimeaCaptureDevice::startVideoAquisition()  
{
    g_assert(0 && "TODO IMPLEMENT");
    return false;
}

    // acquisition, processing (e.g. debayer), enhancing (w/ metadata),
bool LirenaXimeaCaptureDevice::getFrame()  
{
    g_assert(0 && "TODO IMPLEMENT");
    return false;
}

bool LirenaXimeaCaptureDevice::postProcessFrame()  
{
    g_assert(0 && "TODO IMPLEMENT");
    return false;
}

bool LirenaXimeaCaptureDevice::calcTiming()  
{
    g_assert(0 && "TODO IMPLEMENT");
    return false;
}

bool LirenaXimeaCaptureDevice::acquireFrameMetadata()  
{
    g_assert(0 && "TODO IMPLEMENT");
    return false;
}
    
// "publishing"
bool LirenaXimeaCaptureDevice::pushFrameMetaDataToGstreamer()  
{
    g_assert(0 && "TODO IMPLEMENT");
    return false;
}

bool LirenaXimeaCaptureDevice::pushVideoFrameToGstreamer()  
{
    g_assert(0 && "TODO IMPLEMENT");
    return false;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

LirenaMagewellEcoCaptureDevice::LirenaMagewellEcoCaptureDevice(
    LirenaConfig * configPtr)
    : LirenaCaptureDevice(configPtr)
{
}


LirenaMagewellEcoCaptureDevice::~LirenaMagewellEcoCaptureDevice()
{
    g_assert(0 && "TODO IMPLEMENT");
}

