#ifndef __LIRENA_CAPTURE_APP__H__
#define __LIRENA_CAPTURE_APP__H__


#include "LirenaConfig.h"
#include "LirenaCamera.h"
// #include "LirenaKLVappsrc.h"

#include "LirenaCaptureDisplayController.h"

//----------------------------------------------------------------------------
// Type forwards:




//----------------------------------------------------------------------------
// Type definitions

struct LirenaCaptureApp
{
	LirenaConfig config;

	LirenaCamera camState;

	LirenaCaptureDisplayController localDisplayCtrl;

    // non-GUI loop for bus-listening,
    // in case local displaying is not requested
    GMainLoop *pureLoop;
};


#endif //__LIRENA_CAPTURE_APP__H__