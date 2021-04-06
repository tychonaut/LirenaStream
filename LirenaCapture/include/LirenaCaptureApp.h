#ifndef __LIRENA_CAPTURE_APP__H__
#define __LIRENA_CAPTURE_APP__H__


#include "LirenaConfig.h"
#include "LirenaXimeaStreamer.h"
// #include "LirenaKLVappsrc.h"

#include "LirenaCaptureDisplayController.h"

//----------------------------------------------------------------------------
// Type forwards:




//----------------------------------------------------------------------------
// Type definitions

struct LirenaCaptureApp
{
	LirenaConfig config;

    LirenaXimeaStreamer streamer;
    
    //TODO migrate to streamer
	LirenaXimeaStreamer_CameraParams camState;
    pthread_t captureThread;
    
    
    // non-GUI loop for bus-listening,
    // in case local displaying is not requested
    GMainLoop *pureLoop;

    //GUI stuff
	LirenaCaptureDisplayController localDisplayCtrl;
};


#endif //__LIRENA_CAPTURE_APP__H__