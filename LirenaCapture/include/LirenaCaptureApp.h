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
    
    // non-GUI loop for bus-listening,
    // in case local displaying is not requested;
    // probably not nececcary as long as no listening to 
    // any messages ;()
    GMainLoop *pureLoop;

    //GUI stuff
	LirenaCaptureDisplayController localDisplayCtrl;
};


#endif //__LIRENA_CAPTURE_APP__H__