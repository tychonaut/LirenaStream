#ifndef __LIRENA_CAPTURE_APP__H__
#define __LIRENA_CAPTURE_APP__H__


#include "LirenaConfig.h"
#include "LirenaStreamer.h"
// #include "LirenaKLVappsrc.h"

#include "LirenaCaptureUI/LirenaCaptureUI.h"

//----------------------------------------------------------------------------
// Type forwards:




//----------------------------------------------------------------------------
// Type definitions

class LirenaCaptureApp
{
    public:
        LirenaCaptureApp(LirenaConfig * configPtr);
        ~LirenaCaptureApp();

        LirenaConfig * configPtr;

        LirenaStreamer* streamerPtr;

        // UI stuff
        // TODO make pointer cause actract and factory...
        LirenaCaptureUI* uiPtr;
        

};


#endif //__LIRENA_CAPTURE_APP__H__