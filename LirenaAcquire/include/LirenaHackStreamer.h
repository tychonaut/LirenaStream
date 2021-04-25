#ifndef __LIRENA_HACK_STREAMER_H__
#define __LIRENA_HACK_STREAMER_H__

#include "LirenaConfig.h"

#include <gst/app/gstappsrc.h>


namespace cv
{
    class Mat;    
}

class LirenaHackStreamer
{
    public:

        explicit LirenaHackStreamer(LirenaConfig const * config);
        ~LirenaHackStreamer();

        //to be called by cuda "completion callback"
        bool pushCvMatToGstreamer(
            cv::Mat& cpuMat, 
            // maybe not nececcary, 
            //but keep track of self-alloced mem this way:
            gchar** ptrToSelfAllocedMemoryPtr);

    private:
 
        // returns PTS (presentation timestamp)
        // according to GstClock
        // called by pushCvMatToGstreamer(..)
        GstClockTime calcTimings();

        // return value must be g_free'ed
        gchar* constructPipelineString();
        // return value must be g_free'ed
        gchar* constructMinimalPipelineString();

        

        //member variables:

        LirenaConfig const * config;

        GstElement *pipeline = nullptr;
        GstElement *appsrc_video = nullptr;

        //{ PTP stuff: TODO setup and use
        GstClock * createGstPTPclock();
        GstClock * syncGstPTPclock();
        GstClock * myGstPTPclock = nullptr;
        //}
};




#endif // __LIRENA_HACK_STREAMER_H__