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
    explicit LirenaHackStreamer(LirenaConfig const * config)
        : config(config)
    {
        // n.b. gst_init has been called in main();

        //debug:
        gchar* pipeString = constructMinimalPipelineString();
        //gchar* pipeString = constructPipelineString();;

        pipeline = gst_parse_launch(
		    pipeString,
		    NULL);
        g_assert(pipeline);

        printf("%s\n", "pipeline created");

        //TODO setup appsource etc;

        g_free(pipeString);

        gst_element_set_state (pipeline, GST_STATE_PLAYING);

        //GMainLoop* mainloop = g_main_loop_new(NULL, false);
        //g_main_loop_run(mainloop);

        //g_main_
    }

    ~LirenaHackStreamer()
    {

    }


    // return value must be g_free'ed
    gchar* constructPipelineString();

    // return value must be g_free'ed
    gchar* constructMinimalPipelineString();


    bool pushCvMatToGstreamer(cv::Mat& cpuMat);

    //member variables:

    LirenaConfig const * config;

    GstElement *pipeline = nullptr;
	GstElement *appsrc_video = nullptr;

};




#endif // __LIRENA_HACK_STREAMER_H__