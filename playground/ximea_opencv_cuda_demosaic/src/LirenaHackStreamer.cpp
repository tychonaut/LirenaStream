#include "LirenaHackStreamer.h"

#include <unistd.h> //sleep


#include <opencv2/opencv.hpp>







LirenaHackStreamer::LirenaHackStreamer(LirenaConfig const *config)
    : config(config)
{
    // n.b. gst_init has been called in main();

    //debug:
    gchar *pipeString = constructMinimalPipelineString();
    //gchar* pipeString = constructPipelineString();;

    pipeline = gst_parse_launch(
        pipeString,
        NULL);
    g_assert(pipeline);

    printf("%s\n", "pipeline created");

    //TODO setup appsource etc;

    g_free(pipeString);

    gst_element_set_state(pipeline, GST_STATE_PLAYING);

    //GMainLoop* mainloop = g_main_loop_new(NULL, false);
    //g_main_loop_run(mainloop);

    //g_main_
}

LirenaHackStreamer::~LirenaHackStreamer()
{
}

// sixteen thousand chars are hopefully sufficient...
#define MAX_GSTREAMER_PIPELINE_STRING_LENGTH 16384
#define MAX_GSTREAMER_PIPELINE_SNIPPET_STRING_LENGTH 1024

gchar *LirenaHackStreamer::constructMinimalPipelineString()
{
    gchar *gstreamerPipelineString =
        (gchar *)g_malloc0(MAX_GSTREAMER_PIPELINE_STRING_LENGTH);


    g_snprintf(
        gstreamerPipelineString,
        (gulong)MAX_GSTREAMER_PIPELINE_STRING_LENGTH,

         " appsrc format=GST_FORMAT_TIME is-live=TRUE name=videoAppSrc ! "
         //"   video/x-raw,format=(string)RGBA,width=%d,height=%d ! " //format=(string)BGRx,
         //"   video/x-raw,format=(string)RGBA,width=%d,height=%d ! "
	     "  video/x-raw,format=(string)BGRx,"
                "bpp=(int)32,depth=(int)24,"
                "endianness=(int)%d,"
                "red_mask=0xff000000,green_mask=0x0000ff00,blue_mask=0x0000ff00,"
                "width=%d,height=%d,framerate=0/1 ! "
				
        //" videotestsrc do-timestamp=true ! " // num-buffers=%d  
        //"    video/x-raw  format=(string)BGRx !" 
                    //,width=${myStreamingResX},height=${myStreamingResY},framerate=${myTargetFPS}/1 ! "

        //" nvvidconv ! "
        //"   video/x-raw(memory:NVMM) ! "
        //,format=(string)NV12,width=${myStreamingResX},height=${myStreamingResY},framerate=${myTargetFPS}/1" !
        //" tee name=T1 "

        //" T1. ! "
        //"   queue max-size-bytes=1000000000 ! "
        //"   nvivafilter cuda-process=true customer-lib-name=libnvsample_cudaprocess.so ! "
        //"     video/x-raw(memory:NVMM),format=(string)NV12 ! "
        
        //"   nvvidconv ! "
        //"     video/x-raw(memory:NVMM),format=(string)NV12,width=840,height=525 ! " //,framerate=${myTargetFPS}/1 ! "
        //"  nvoverlaysink display-id=0 overlay-x=840 overlay-y=0 overlay-w=840 overlay-h=525  "
        
         " videoscale ! "
         " videoconvert ! "
         " xvimagesink "
        
        //"autovideosink"
        
        ""
        ,
        G_BIG_ENDIAN,
        this->config->getStreamingResolutionX(),
        this->config->getStreamingResolutionY()
        //12 // nun total buffers
    );

    printf("HERE IS THE GSTREAMER PIPELINE\n\n: %s\n\n", gstreamerPipelineString);

    return gstreamerPipelineString;


    // g_snprintf(
    //     gstreamerPipelineString,
    //     (gulong)MAX_GSTREAMER_PIPELINE_STRING_LENGTH,

    //     " videotestsrc do-timestamp=true ! " // num-buffers=%d  
    //     "    video/x-raw !" //,width=${myStreamingResX},height=${myStreamingResY},framerate=${myTargetFPS}/1 ! "
    //     //" nvvidconv ! "
    //     //"   video/x-raw(memory:NVMM) ! "
    //     //,format=(string)NV12,width=${myStreamingResX},height=${myStreamingResY},framerate=${myTargetFPS}/1" !
    //     //" tee name=T1 "

    //     //" T1. ! "
    //     //"   queue max-size-bytes=1000000000 ! "
    //     //"   nvivafilter cuda-process=true customer-lib-name=libnvsample_cudaprocess.so ! "
    //     //"     video/x-raw(memory:NVMM),format=(string)NV12 ! "
    //     "   nvvidconv ! "
    //     "     video/x-raw(memory:NVMM),format=(string)NV12,width=512,height=512 ! " //,framerate=${myTargetFPS}/1 ! "
    //     "  nvoverlaysink display-id=0 overlay-x=840 overlay-y=0 overlay-w=840 overlay-h=525  "
        
    //     // " videoscale ! "
    //     // " videoconvert ! "
    //     // " xvimagesink "
        
    //     //"autovideosink"
        
    //     "%s",
    //     ""
    //     //12 // nun total buffers
    // );
}



gchar *LirenaHackStreamer::constructPipelineString()
{

    gchar *gstreamerPipelineString = (gchar *)g_malloc0(MAX_GSTREAMER_PIPELINE_STRING_LENGTH);

    gchar gstreamerForkString_raw_to_show_and_enc[MAX_GSTREAMER_PIPELINE_SNIPPET_STRING_LENGTH];
    gchar gstreamerForkString_enc_to_disk_and_UDP[MAX_GSTREAMER_PIPELINE_SNIPPET_STRING_LENGTH];
    gchar gStreamerNetworkTransmissionSnippet[MAX_GSTREAMER_PIPELINE_SNIPPET_STRING_LENGTH];

    // init snippets to empty string:
    g_snprintf(gstreamerForkString_raw_to_show_and_enc,
               (gulong)MAX_GSTREAMER_PIPELINE_SNIPPET_STRING_LENGTH,
               "%s", "");
    g_snprintf(gstreamerForkString_enc_to_disk_and_UDP,
               (gulong)MAX_GSTREAMER_PIPELINE_SNIPPET_STRING_LENGTH,
               "%s", "");
    g_snprintf(gStreamerNetworkTransmissionSnippet,
               (gulong)MAX_GSTREAMER_PIPELINE_SNIPPET_STRING_LENGTH,
               "%s", "");

    if (config->doLocalDisplay)
    {
        g_snprintf(gstreamerForkString_raw_to_show_and_enc,
                   (gulong)MAX_GSTREAMER_PIPELINE_SNIPPET_STRING_LENGTH,
                   "%s",
                   " tee name=fork_raw_to_show_and_enc "
                   ""
                   " fork_raw_to_show_and_enc. ! "
                   " queue ! "
                   " videoscale add-borders=TRUE ! "
                   " capsfilter name=scale_element ! "
                   " videoconvert ! "
                   " xvimagesink "
                   ""
                   " fork_raw_to_show_and_enc. ! ");
    }

    if (config->outputFile)
    {
        /*
          Inject dump-to-disk:
          [encoded stream incoming] -> write to disk
	                                \-> [to stream as RTP over UDP]
        */
        g_snprintf(gstreamerForkString_enc_to_disk_and_UDP,
                   (gulong)MAX_GSTREAMER_PIPELINE_SNIPPET_STRING_LENGTH,
                   // TODO maybe add " h264parse disable-passthrough=true ! "
                   " tee name=fork_enc_to_disk_and_UDP "
                   ""
                   " fork_enc_to_disk_and_UDP. ! "
                   " queue ! "
                   " filesink location=%s "
                   ""
                   " fork_enc_to_disk_and_UDP. ! "
                   " queue ! ",
                   config->outputFile);
    }

    if (config->useTCP)
    {
        //TCP
        GST_WARNING("%s", "Using TCP instead of UDP: "
                          "This is experimental an my not work!");

        g_snprintf(gStreamerNetworkTransmissionSnippet,
                   (gulong)MAX_GSTREAMER_PIPELINE_SNIPPET_STRING_LENGTH,
                   " rtpstreampay ! " // wrap RTP stuff another time for TCP ... (?)
                   "  application/x-rtp-stream ! "
                   " tcpserversink "
                   "   host=%s "
                   "   port=%s "
                   //"   sync-method=burst "
                   "  sync-method=next-keyframe " // works, but only for single-output recedeiver pipelines
                   //"   sync-method=latest " //  works, but also no batter than next-keyframe
                   ,
                   config->IP,
                   config->port);
    }
    else
    {
        //UDP
        g_snprintf(gStreamerNetworkTransmissionSnippet,
                   (gulong)MAX_GSTREAMER_PIPELINE_SNIPPET_STRING_LENGTH,
                   " udpsink "
                   "   host=%s"
                   "   port=%s"
                   "   sync=true ", // sync=false
                   config->IP,
                   config->port);
    }
    //gStreamerNetworkTransmissionSnippet

    /*
	    Full pipeline (stuff in brackets is optional):
	    
	    cam [-> show on screen]
	        \-> hardware encode h264 
	                 \ 
	                  >   wrap in mpegts [-> write to disk ]
	    klv ---------/                   \-> stream per as RTP over UDP                                                
    */
    g_snprintf(gstreamerPipelineString,
               (gulong)MAX_GSTREAMER_PIPELINE_STRING_LENGTH,
               // Mpeg2TS muxer:
               //" mpegtsmux name=mp2ts_muxer alignment=0 "
               //""
               // KLV appsrc to  muxer:
               " appsrc format=GST_FORMAT_TIME is-live=TRUE name=klvSrc ! "
               "   meta/x-klv, parsed=true ! "
               " mpegtsmux name=mp2ts_muxer alignment=7 "
               //"  mpegtsmux."
               ""
               // Video appsrc, encode h264, to muxer:
               " appsrc format=GST_FORMAT_TIME is-live=TRUE name=streamViewer ! "

               // //nope:
               // //GST_PIPELINE grammar.y:721:gst_parse_perform_link: could not link videoscale0 to bayer2rgb0, videoscale0 can't handle caps video/x-bayer, width=(int)512, height=(int)512
               // " videoscale add-borders=TRUE ! "
               // "   video/x-bayer,width=512,height=512 ! "

               ""
               //{ software debayering by GStreamer itself
               //: TODO outsource to Cuda&OpenCV

               // 16fps fluid show + dump @ half res, BUT pink+moiree!
               //"   video/x-bayer, format=(string)bggr! "
               //blank image
               //"   video/x-bayer, format=(string)grbg! "
               //blank image
               //"   video/x-bayer, format=(string)gbrg! "
               //"   video/x-bayer, format=(string)rggb! "

               " bayer2rgb ! "

               // BGRx is correct when NOT ximea-decimating....
               //"   video/x-raw, format=(string)BGRx ! "
               //test:
               "   video/x-raw, format=(string)BGRx ! "

               " videoconvert ! "
               //}

               " %s " // optional fork to local display
               " queue ! "
               " nvvidconv ! "
               // downsampling works

               //combi works for udp streaming
               //"   video/x-raw(memory:NVMM),width=1920,height=1200 ! "
               //" nvv4l2h264enc maxperf-enable=1 bitrate=8000000 !

               // works also without bitrate param
               //"   video/x-raw(memory:NVMM),width=1920,height=1200 ! "
               //" nvv4l2h264enc maxperf-enable=1 !" // bitrate=16000000 ! "

               "   video/x-raw(memory:NVMM),width=3840,height=2400 ! "
               " nvv4l2h264enc maxperf-enable=1 !" // bitrate=16000000 ! "

               " h264parse  disable-passthrough=true ! " //config-interval=-1  <--may be required in TCP?...
               " mp2ts_muxer. "
               ""
               // prepare muxed stream for network transmission:
               " mp2ts_muxer.! "
               "   tsparse ! "
               "   queue max-size-time=30000000000 max-size-bytes=0 max-size-buffers=0 ! "
               "   %s " // optional fork to dump-to-disk
               "   rtpmp2tpay ! "
               ""
               // send over network via UDP or optionally TCP
               " %s ",
               gstreamerForkString_raw_to_show_and_enc,
               gstreamerForkString_enc_to_disk_and_UDP,
               gStreamerNetworkTransmissionSnippet);

    printf("HERE IS THE GSTREAMER PIPELINE\n\n: %s\n\n", gstreamerPipelineString);
    sleep(1);

    return gstreamerPipelineString;
}




bool LirenaHackStreamer::pushCvMatToGstreamer(
    cv::Mat &cpuMat,
    gchar** ptrToSelfAllocedMemoryPtr)
{
    appsrc_video = gst_bin_get_by_name(GST_BIN(pipeline), "videoAppSrc");
    //g_assert(appsrc_video);

    //{ Clock/timestamp  stuff:
    //https://stackoverflow.com/questions/34294755/mux-klv-data-with-h264-by-mpegtsmux
    GstClock *clock = nullptr;
    GstClockTime abs_time, base_time;
    //TODO try with appscr instead of pipeline
    GST_OBJECT_LOCK(pipeline);
    // TODO try gst_pipeline_get_pipeline_clock
    clock = GST_ELEMENT_CLOCK(pipeline);
    if (!clock)
    {
        GST_ERROR("%s", "NO CLOCK");
        exit(1);
    }
    base_time = GST_ELEMENT(pipeline)->base_time;
    gst_object_ref(clock);
    abs_time = gst_clock_get_time(clock);
    gst_object_unref(clock);
    GST_OBJECT_UNLOCK(pipeline);
    guint64 myPTS_timestamp = abs_time - base_time;
    GST_DEBUG("myPTS_timestamp: %lu", myPTS_timestamp);
    //}


    cv::Size siz = cpuMat.size();
    gsize buffSize = siz.height * siz.width * 4; //RGBA or whatever
    GstBuffer* video_frame_GstBuffer = gst_buffer_new_allocate(
        0,
        buffSize, 
        0
    );
    gst_buffer_fill(
        video_frame_GstBuffer,
        0, //offset
        cpuMat.data, // gconstpointer src,
        buffSize
    );
    
    GST_BUFFER_PTS(video_frame_GstBuffer) = myPTS_timestamp;
	//GST_BUFFER_DURATION (video_frame_GstBuffer) = myDuration;

	// push buffer into gstreamer pipeline
	GstFlowReturn ret = gst_app_src_push_buffer(GST_APP_SRC(appsrc_video), video_frame_GstBuffer);
    g_assert(ret == GST_FLOW_OK);



    // // kind of workaround for not understanding GstBufferPool in the short time:
    // // the image memory of cpuMat was passed to gstreamer and will be deleted
    // // when gstreamer is done with it;
    // // (idea for later: maybe we can trick gstreamer into not deleting by 
    // // gst_ref()'ing  the buffer; but then, it will not be destroayed and there 
    // // will be no callback to inform us that it can bbe reused... damn)


    // static gchar* pointerGraveYard[1000000];
    // static int currentFrame = 0;

 


    // gchar* newMemory = (gchar *)malloc(
    //        siz.height
    //        *
    //        siz.width
    //        *
    //        4 //RGBA or whatever
    //     );
    // g_assert(newMemory);

    // memset(newMemory, 255,  siz.height
    //        *
    //        siz.width
    //        *
    //        4 );


    // pointerGraveYard[currentFrame] = *ptrToSelfAllocedMemoryPtr;
    // printf("currentFrame: %d, old pointer adress: %lu, value in memory: %lu \n",
    //     currentFrame,
    //     (guint64) pointerGraveYard[currentFrame],
    //     (guint64) pointerGraveYard[currentFrame][1000000]
    // );
    // currentFrame++;

    // g_free(*ptrToSelfAllocedMemoryPtr);


    // *ptrToSelfAllocedMemoryPtr =
    //     newMemory;
    // g_assert(*ptrToSelfAllocedMemoryPtr);
    
    // // cpuMat.release();

    // // //destroy old Mat instance (image mem will not be deleted,
    // // // as manually alloc'ed), re-init with new memory
    // // cpuMat =
    // //     cv::Mat(
    // //         siz,
    // //         //CV_8UC3
    // //         // need to be 32bit RGBx/GBRx/xRGB for nvvidconv
    // //         CV_8UC4
    // //         ,
    // //         //provide self-managed memory
    // //         *ptrToSelfAllocedMemoryPtr
    // // );


    return true;
}


GstClockTime LirenaHackStreamer::calcTimings()
{
    g_assert("TODO implement");

    return 0;
}