

gst-launch-1.0 -v videotestsrc ! \
     'video/x-raw, format=(string)UYVY, width=(int)320, height=(int)240, framerate=(fraction)30/1' ! \
      nvvidconv ! \
      'video/x-raw(memory:NVMM), width=(int)1920, height=(int)1080, format=(string)NV12' ! \
      nvv4l2h264enc ! \
      h264parse ! \
      qtmux ! \
      filesink location=test_1080p.mp4 -e
      
      
      
      
# below might be helpful when integrating hardware demosaic...
                //" video/x-raw(memory:NVMM), format=(string)NV12 ! "
                //" nvv4l2h264enc maxperf-enable=true ! "
	                /* For 4k encoding performance testing 
                   		(2k@120 Hz make problems, hence trying 4k@30,
                        which is our target resolution)
                    */
                    //BOOLEAN doQuadrupleImage_debug = TRUE;
                // nvcompositor \
                  name=comp sink_0::xpos=0 sink_0::ypos=0 sink_0::width=1920 \
                  sink_0::height=1080 

                //            " video/x-bayer, format=(string)bggr ! "
                 //           " bayer2rgb ! "
                   //         " queue ! "

	                /*
	                    Hardware-accelerated, but hardcoded pipeline:

	                    cam -> hardware encode h264 -> stream per as RTP over UDP

	                pipeline = gst_parse_launch(
	                    "appsrc is-live=TRUE name=streamViewer ! "
                            " queue ! "
                            " nvvidconv ! "
                            " video/x-raw(memory:NVMM), format=(string)NV12 ! "
                            " nvv4l2h264enc ! "
                            " h264parse ! "
                            " queue ! "
                            " rtph264pay ! "
                            " udpsink host=192.168.0.169 port=5001 sync=false ",
	                     NULL);
	                 */
                //" video/xraw(memory:NVMM), width=(int)4096, height=(int)4096, format=(string)GRAY8 ! "
                //" nvv4l2h264enc maxperf-enable=true ! "
                
                
# Failed timestamp shizzle:  
            /*
			//gst_pipeline_set_latency(GST_PIPELINE(pipeline), 600 * GST_MSECOND);
			
			GstClock * pipelineClock =
              gst_pipeline_get_pipeline_clock (GST_PIPELINE(pipeline));
            GstClockTime gstreamerCurrentTime =
                gst_clock_get_time (pipelineClock);
            gst_object_unref(pipelineClock);
            GstClockTime gstreamerStartTime = GST_ELEMENT(pipeline)->start_time;
            
            GstClockTime gstreamerCurrentTimestamp =  gstreamerCurrentTime - gstreamerStartTime;          
             
            GstClockTime gstreamerCurrentFrameDuration = gstreamerCurrentTimestamp - gstreamerPreviousTime;

            //GST_BUFFER_TIMESTAMP(buffer) = gstreamerCurrentTimestamp; //  - 1* GST_MSECOND;
            //GST_BUFFER_DURATION (buffer) = gstreamerCurrentFrameDuration;
            GST_BUFFER_PTS(buffer) = gstreamerCurrentTimestamp; //  + 30 * GST_MSECOND;
            //buffer->duration = gstreamerCurrentFrameDuration;
            //buffer->offset = frames;
            GST_BUFFER_OFFSET(buffer) = frames;
            
            gstreamerPreviousTime = gstreamerCurrentTimestamp;
            
	            
            // works, but unstable due to systematic errors;
			unsigned long target_FPS = (unsigned long) (1000/globalAppArgs.exposure_ms);
			// Set its timestamp and duration
            //GST_BUFFER_TIMESTAMP (buffer) = gst_util_uint64_scale (frames, GST_SECOND, target_FPS);
            //GST_BUFFER_DURATION (buffer) = gst_util_uint64_scale (globalAppArgs.exposure_ms, GST_MSECOND, target_FPS);
            //GST_BUFFER_DURATION (buffer) = GST_CLOCK_TIME_NONE;
            //GST_BUFFER_DURATION (buffer) = globalAppArgs.exposure_ms * GST_MSECOND;
			
			//works, but is not precise
			//GST_BUFFER_PTS(buffer) = (guint64) ( (guint64) frames * (guint64) globalAppArgs.exposure_ms * (guint64) GST_MSECOND);
			// NOT works... maybe the pieline clokc is bad because the source is life and app source...
			//GST_BUFFER_PTS(buffer) = gstreamerCurrentTimestamp;
            */
