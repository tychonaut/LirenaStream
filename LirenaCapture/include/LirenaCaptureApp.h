#ifndef __LIRENA_CAPTURE_APP__H__
#define __LIRENA_CAPTURE_APP__H__


#include "LirenaConfig.h"
#include "LirenaStreamer.h"
// #include "LirenaKLVappsrc.h"

#include "LirenaCaptureDisplayController.h"

//----------------------------------------------------------------------------
// Type forwards:




//----------------------------------------------------------------------------
// Type definitions

class LirenaCaptureApp
{
    public:
        LirenaConfig config;

        LirenaStreamer streamer;

        // UI stuff
        // TODO make pointer cause actract and factory...
        LirenaCaptureUI ui;
        
        //TODO outsource to LirenaNetworkUI
        // non-GUI loop for bus-listening,
        // in case local displaying is not requested;
        // probably not nececcary as long as no listening to 
        // any messages ;()
        GMainLoop *pureMainLoop;

};


#endif //__LIRENA_CAPTURE_APP__H__