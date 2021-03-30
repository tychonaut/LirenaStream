#include <sys/time.h>
#include <pthread.h>
#include <string.h>
#include <gtk/gtk.h>

#ifdef GDK_WINDOWING_X11
#include <gdk/gdkx.h>
#include <X11/Xlib.h>
#endif

#include <gst/app/gstappsrc.h>

#include <gst/video/videooverlay.h>

#include <m3api/xiApi.h> //Ximea API

#include <stdlib.h>
#include <argp.h> // CLI arguments parsing


// ---------------------------------------------------------------------------
//{ KLV experiments
//endianess converting (for KLV metadata)
#include <stdint.h>   // For 'uint64_t'
#include <endian.h>   // htobe64  be64toh (host to big endian 64)

#define KLV_KEY_SIZE_BYTES     8
#define KLV_LENGTH_SIZE_BYTES  8

// don't know if low keys are reserved or forbidden or sth...
#define KLV_KEY_OFFSET 0
enum HOMEBREW_KLV_keys
{
    KLV_KEY_invalid  = 0 + KLV_KEY_OFFSET,
    KLV_KEY_string   = 1 + KLV_KEY_OFFSET,
    KLV_KEY_uint64   = 2 + KLV_KEY_OFFSET,
    KLV_KEY_float64  = 3 + KLV_KEY_OFFSET,
    
    KLV_KEY_camera_ID = 4 + KLV_KEY_OFFSET,
    
    KLV_KEY_image_capture_time_stamp  = 5 + KLV_KEY_OFFSET,
    KLV_KEY_navigation_data_time_stamp  = 6 + KLV_KEY_OFFSET,
    
    KLV_KEY_image_frame_number  = 7 + KLV_KEY_OFFSET,
    KLV_KEY_navigation_data_frame_number = 8 + KLV_KEY_OFFSET,
    
    KLV_KEY_navigation_data_payload = 9 + KLV_KEY_OFFSET,
    KLV_KEY_image_statistics = 10 + KLV_KEY_OFFSET,
    
    KLV_KEY_num_keys = 11
};

// Converts relevant values to big endian and writes a KLV triple
// return pointer points at the byte right after the area 
//  the func has written to: 
//  valuePtr + KLV_KEY_SIZE_BYTES + KLV_LENGTH_SIZE_BYTES + len_host
guint8 * 
write_homebrew_KLV_item(
    guint8 * klv_buffer,
    uint64_t key_host,
    uint64_t len_host, 
    guint8 const * value_ptr)
{   
    uint64_t key_be = htobe64(key_host);
    memcpy(klv_buffer, &key_be, KLV_KEY_SIZE_BYTES);
    klv_buffer += KLV_KEY_SIZE_BYTES;
            
    uint64_t len_be = htobe64(len_host);
    memcpy(klv_buffer, &len_be, KLV_LENGTH_SIZE_BYTES);
    klv_buffer += KLV_LENGTH_SIZE_BYTES; 
            
   memcpy(klv_buffer, value_ptr, len_host);
   klv_buffer += len_host;
   
   return klv_buffer;
}




// ---------------------------------------------------------------------------
//} KLV experiments


//{ parsing CLI options -------------------------------------------------------
const char *argp_program_version =
  "ximeaJetsonStream 0.2.0";
const char *argp_program_bug_address =
  "<mschlueter@geomar.de>";

/* Program documentation. */
static char doc[] =
  "Capture and stream video from Ximea camera on Jetson using GStreamer";

/* A description of the arguments we accept. */
static char args_doc[] = "IP Port";

/* The options we understand. */
static struct argp_option options[] = {
  //{"verbose",  'v', 0,      0,  "Produce verbose output" },
  {"localdisplay",  'l', 0,      0,  "display video locally, (not just stream per UDP)" },
  {"output",   'o', "FILE", 0, "Dump encoded stream to disk" }, //OPTION_ARG_OPTIONAL
  {"exposure", 'e', "time_ms", 0, "Exposure time in milliseconds" },
  { 0 }
};

/* Used by main to communicate with parse_opt. */
struct ApplicationArguments
{
  bool doLocalDisplay;
  //int silent, verbose;
  char const *output_file;

  char const *IP;
  char const *port;
  
  int exposure_ms;
};



/* Parse a single option. */
static error_t
parse_opt (int key, char *arg, struct argp_state *state)
{
  /* Get the input argument from argp_parse, which we
     know is a pointer to our arguments structure. */
  struct ApplicationArguments *myArgs = 
    (struct ApplicationArguments *) state->input;

  switch (key)
    {
    case 'l':
      myArgs->doLocalDisplay = true;
      break;
    //case 'v':
    //  arguments->verbose = 1;
    //  break;
    case 'o':
      myArgs->output_file = arg;
      printf("%s", "Warning: local dump to disk currently not supported.\n");
      break;
    case 'e':
      myArgs->exposure_ms = atoi(arg);
      break;

    case ARGP_KEY_ARG:
      if (state->arg_num >= 2)
      {
        /* Too many arguments. */
        argp_usage (state);
      }
      if(state->arg_num == 0)
      {
        myArgs->IP = arg;
      }
      else
      {
        myArgs->port = arg;
      }
      break;

    case ARGP_KEY_END:
      if (state->arg_num < 2)
        /* Not enough arguments. */
        argp_usage (state);
      break;

    default:
      return ARGP_ERR_UNKNOWN;
    }
  return 0;
}

/* Our argp parser. */
static struct argp argp = { options, parse_opt, args_doc, doc };
//} ---------------------------------------------------------------------------


// ----------------------------------------------------------------------------
//{ Global variables; TODO organize
ApplicationArguments globalAppArgs;

BOOLEAN acquire, quitting, render = TRUE;
int maxcx, maxcy, roix0, roiy0, roicx, roicy;
pthread_t videoThread;
HANDLE handle = INVALID_HANDLE_VALUE;
guintptr window_handle;

//}







inline unsigned long getcurus() 
{
	struct timeval now;
	gettimeofday(&now, NULL);
	return now.tv_sec * 1000000 + now.tv_usec;
}



GstBusSyncReply bus_sync_handler(GstBus* bus, GstMessage* message, gpointer) 
{
	if(!gst_is_video_overlay_prepare_window_handle_message(message))
		return GST_BUS_PASS;
	gst_video_overlay_set_window_handle(GST_VIDEO_OVERLAY(GST_MESSAGE_SRC(message)), window_handle);
	gst_message_unref(message);
	return GST_BUS_DROP;
}




void video_widget_realize_cb(GtkWidget* widget, gpointer) 
{
	GdkWindow *window = gtk_widget_get_window(widget);
	if (!gdk_window_ensure_native(window))
		g_error ("Couldn't create native window needed for GstXOverlay!");
	window_handle = GDK_WINDOW_XID(window);
}


gboolean close_cb(GtkWidget*, GdkEvent*, gpointer quit) 
{
	quitting = quit ? TRUE : FALSE;
	if(videoThread)
		acquire = FALSE;
	else
		gtk_main_quit();
	return TRUE;
}



void* videoDisplay(void*) 
{	
	GstElement *pipeline = 0;
	GstElement *appsrc_video = 0;
	GstElement *appsrc_klv = 0;	
	GstElement *scale_element = 0;
	//GstElement *mpegtsmux = 0;

	GstBus *bus = 0;
	
	GstBuffer *video_frame_GstBuffer = 0;
	GstBuffer *klv_frame_GstBuffer = 0;
	
	GstCaps *appsrc_video_caps = 0;
	GstCaps *size_caps = 0;
    //GstCaps *appsrc_klv_caps = 0;
	
	GstFlowReturn ret;


	int max_width, max_height;
	int prev_width = -1;
	int prev_height = -1;
	
	gchar statistics_string[256];
	unsigned long frames = 0;
	unsigned long prevframes = 0;
	unsigned long lostframes = 0;
	long lastframe = -1;
	
	
	unsigned long curtime, prevtime;
    unsigned long firsttime = getcurus();
    unsigned long gstreamerPreviousTime;
    
	
	
	
	XI_IMG_FORMAT prev_format = XI_RAW8;
	XI_IMG image;
	image.size = sizeof(XI_IMG);
	image.bp = NULL;
	image.bp_size = 0;
	
	if(xiStartAcquisition(handle) != XI_OK) 
	{
		goto exit;
	}
	
	
	
	GtkWidget *videoWindow;
	GdkScreen *screen;
	
	gdk_threads_enter();
	videoWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(videoWindow), "streamViewer");
	gtk_widget_set_double_buffered(videoWindow, FALSE);
	g_signal_connect(videoWindow, "realize", G_CALLBACK(video_widget_realize_cb), NULL);
	g_signal_connect(videoWindow, "delete-event", G_CALLBACK(close_cb), NULL);
	gtk_widget_show_all(videoWindow);
	gtk_widget_realize(videoWindow);
	screen = gdk_screen_get_default();
	max_width = 0.8 * gdk_screen_get_width(screen);
	max_height = 0.8 * gdk_screen_get_width(screen);
	gdk_threads_leave();
	

		


   	// ten thousand chars are hopefully sufficient...
   	#define MAX_GSTREAMER_PIPELINE_STRING_LENGTH 10000
   	#define MAX_GSTREAMER_PIPELINE_SNIPPET_STRING_LENGTH 1000
   	
    gchar gstreamerPipelineString [MAX_GSTREAMER_PIPELINE_STRING_LENGTH];
    gchar gstreamerForkString_raw_to_show_and_enc[
      MAX_GSTREAMER_PIPELINE_SNIPPET_STRING_LENGTH]; 
    gchar gstreamerForkString_enc_to_disk_and_UDP[
      MAX_GSTREAMER_PIPELINE_SNIPPET_STRING_LENGTH];
      
    // init snippets to empty string:
    g_snprintf(gstreamerForkString_raw_to_show_and_enc,
               (gulong) MAX_GSTREAMER_PIPELINE_SNIPPET_STRING_LENGTH,
                "%s","");     
    g_snprintf(gstreamerForkString_enc_to_disk_and_UDP,
               (gulong) MAX_GSTREAMER_PIPELINE_SNIPPET_STRING_LENGTH,
                "%s",""); 
      
    if(globalAppArgs.doLocalDisplay)
    {
        g_snprintf(gstreamerForkString_raw_to_show_and_enc,
            (gulong) MAX_GSTREAMER_PIPELINE_SNIPPET_STRING_LENGTH,
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
            " fork_raw_to_show_and_enc. ! "
        );
    }
    
    if(globalAppArgs.output_file)
    {
        /*
          Inject dump-to-disk:
          [encoded stream incoming] -> write to disk
	                                \-> [to stream as RTP over UDP]
        */
        g_snprintf(gstreamerForkString_enc_to_disk_and_UDP,
            (gulong) MAX_GSTREAMER_PIPELINE_SNIPPET_STRING_LENGTH,
            // TODO maybe add " h264parse disable-passthrough=true ! "
            " tee name=fork_enc_to_disk_and_UDP " 
            ""
            " fork_enc_to_disk_and_UDP. ! "
            " queue ! "
            " filesink location=%s "
            ""
            " fork_enc_to_disk_and_UDP. ! "
            " queue ! "
            ,
            globalAppArgs.output_file);
    }
    
	/*
	    Full pipeline (stuff in brackets is optional):
	    
	    cam [-> show on screen]
	        \-> hardware encode h264 
	                 \ 
	                  >   wrap in mpegts [-> write to disk ]
	    klv ---------/                   \-> stream per as RTP over UDP                                                
    */	
   g_snprintf(gstreamerPipelineString,
        (gulong) MAX_GSTREAMER_PIPELINE_STRING_LENGTH,
            " appsrc format=GST_FORMAT_TIME is-live=TRUE name=klvSrc ! "
            "  meta/x-klv, parsed=true ! "
            //" queue ! "
            " mp2ts_muxer. "
            ""
            " appsrc format=GST_FORMAT_TIME is-live=TRUE name=streamViewer ! "                         
	        "   video/x-bayer ! "
            " bayer2rgb ! "
            "   video/x-raw, format=(string)BGRx ! "
            " videoconvert ! "
	        " %s "          // optional fork to local display
            " queue ! "
            " nvvidconv ! "
            " nvv4l2h264enc maxperf-enable=1 bitrate=8000000 ! "                
            " h264parse !" // disable-passthrough=true ! "
            " mp2ts_muxer. "
            ""
            " mpegtsmux name=mp2ts_muxer ! "
            " tsparse ! " //set-timestamps=true ! " // pcr-pid=-1 TODO what does set-timestamps=true do?
            " queue max-size-time=30000000000 max-size-bytes=0 max-size-buffers=0 ! "
            " %s "          // optional fork to dump-to-disk
            " rtpmp2tpay ! " //" rtph264pay ! "
            " udpsink "
            "   host=%s"
            "   port=%s"
            "   sync=false "
        ,
        gstreamerForkString_raw_to_show_and_enc,
        gstreamerForkString_enc_to_disk_and_UDP,
        globalAppArgs.IP,
        globalAppArgs.port 
    );
	     
   	GST_DEBUG("HERE MY PIPELINE: %s",gstreamerPipelineString);
       	
    pipeline = gst_parse_launch(
      	         gstreamerPipelineString,
    	         NULL);   
	     
	if(!pipeline)
		goto exit;
		
		
		
		
		
	bus = gst_pipeline_get_bus(GST_PIPELINE(pipeline));
	gst_bus_set_sync_handler(bus, (GstBusSyncHandler)bus_sync_handler,
			pipeline,
			NULL);
	gst_object_unref(bus);
	
	appsrc_video = gst_bin_get_by_name(GST_BIN(pipeline), "streamViewer");
	appsrc_klv = gst_bin_get_by_name(GST_BIN(pipeline), "klvSrc");
	
	scale_element = gst_bin_get_by_name(GST_BIN(pipeline), "scale_element");
	
	
	
	//{ "main loop"  
	firsttime = getcurus();
	prevtime = getcurus();
	gstreamerPreviousTime = 0; // IIRC, when going to playing state, all times are zero
	
	while(acquire) {
	
	    // grab image
		if(xiGetImage(handle, 5000, &image) != XI_OK)
		{
			break;
		}
		
		//{ record timestamp of frame capture
		// The following works: Presentation time stamp (PTS) 
		//  from SYSTEM time, NOT GstClock! 
		// TODO setup a working GstClock (PTP)
		// usage: 	GST_BUFFER_PTS(myBuffer) = frame_capture_PTS;
		curtime = getcurus();
		guint64 frame_capture_PTS = (guint64) (curtime - firsttime) * GST_USECOND ;
		//}
		
			
		if(render) 
		{
			unsigned long raw_image_buffer_size = 
			    image.width*image.height*(image.frm == XI_RAW8 ? 1 : 4);
			
			if(!image.padding_x) 
			{
				video_frame_GstBuffer = gst_buffer_new();
				
				gst_buffer_insert_memory(
				    video_frame_GstBuffer, 
				    -1, 
				    gst_memory_new_wrapped(
				        GST_MEMORY_FLAG_READONLY, 
				        (guint8*)image.bp, 
				        raw_image_buffer_size, 
				        0, 
				        raw_image_buffer_size, 
				        0, 
				        0)
				);
			} 
			else 
			{
                printf("Warning: image is padded, mem copy is row-wise!\n");
				video_frame_GstBuffer = gst_buffer_new_allocate(0, raw_image_buffer_size, 0);
				if(!video_frame_GstBuffer)
					break;

				for(guint i = 0; i < image.height; i++)
				{
					gst_buffer_fill(
					        video_frame_GstBuffer,
							i * image.width * (image.frm == XI_RAW8 ? 1 : 4),
							(guint8*)image.bp
							+
							i*(image.width*(image.frm == XI_RAW8 ? 1 : 4)
							+
							image.padding_x),
							image.width*(image.frm == XI_RAW8 ? 1 : 4)
							);
				}
			}
			
			
			 
			
			
			if( (guint)prev_width  != image.width || 
		        (guint)prev_height != image.height ||
			    prev_format != image.frm) 
			{
				if(appsrc_video_caps)
					gst_caps_unref(appsrc_video_caps);
				
				/*
				printf("DEBUG: BGRx from debayering;\n");
				int wid_half= image.width/2;
				int height_half= image.height/2;
			    appsrc_video_caps = gst_caps_new_simple(
							"video/x-raw",
							"format", G_TYPE_STRING, "BGRx",
							"bpp", G_TYPE_INT, 32,
							"depth", G_TYPE_INT, 24,
							"endianness", G_TYPE_INT, G_BIG_ENDIAN,
							"red_mask",   G_TYPE_INT, 0x0000ff00,
							"green_mask", G_TYPE_INT, 0x00ff0000,
							"blue_mask",  G_TYPE_INT, 0xff000000,
							"framerate", GST_TYPE_FRACTION, 0, 1,
							"width", G_TYPE_INT, wid_half,
							"height", G_TYPE_INT, height_half,
							NULL);
				
				// debayering: raw image is not relevant anymore.. NOT... mess
				
				*/
				
				if(image.frm == XI_RAW8)
				{   
				    /* old raw stuff w/o debayering
				    printf("DEBUG: GRAY8\n");
					appsrc_video_caps = gst_caps_new_simple(
							"video/x-raw",
							"format", G_TYPE_STRING, "GRAY8",
							"bpp", G_TYPE_INT, 8,
							"depth", G_TYPE_INT, 8,
							"framerate", GST_TYPE_FRACTION, 0, 1,
							"width", G_TYPE_INT, image.width,
							"height", G_TYPE_INT, image.height,
							NULL);
					*/		
					
					printf("DEBUG: Appsrc has bayer caps!;\n");
					appsrc_video_caps = gst_caps_new_simple(
							"video/x-bayer",
							"format", G_TYPE_STRING, "bggr",
							"bpp", G_TYPE_INT, 8,
							"depth", G_TYPE_INT, 8,
							"framerate", GST_TYPE_FRACTION, 0, 1,
							"width", G_TYPE_INT, image.width,
							"height", G_TYPE_INT, image.height,
							NULL);
					
				
				}			
				else if(image.frm == XI_RGB32)
				{
				    printf("DEBUG: RGB32\n");
					appsrc_video_caps = gst_caps_new_simple(
							"video/x-raw",
							"format", G_TYPE_STRING, "BGRx",
							"bpp", G_TYPE_INT, 32,
							"depth", G_TYPE_INT, 24,
							"endianness", G_TYPE_INT, G_BIG_ENDIAN,
							"red_mask",   G_TYPE_INT, 0x0000ff00,
							"green_mask", G_TYPE_INT, 0x00ff0000,
							"blue_mask",  G_TYPE_INT, 0xff000000,
							"framerate", GST_TYPE_FRACTION, 0, 1,
							"width", G_TYPE_INT, image.width,
							"height", G_TYPE_INT, image.height,
							NULL);
				}			
				else
				{
				    printf("DEBUG: unknown pixel format!\n");
					break;
				}	
		
				
				
				gst_element_set_state(pipeline, GST_STATE_PAUSED);
				gst_app_src_set_caps(GST_APP_SRC(appsrc_video), appsrc_video_caps);
				gst_element_set_state(pipeline, GST_STATE_PLAYING);
				prev_width = image.width;
				prev_height = image.height;
				prev_format = image.frm;
				int width = image.width;
				int height = image.height;
				
				while(width > max_width || height > max_height) {
					width /= 2;
					height /=2;
				}
				
				if(size_caps)
				{
					gst_caps_unref(size_caps);
				}
				
				size_caps = 
				    gst_caps_new_simple(
						"video/x-raw",
						"width", G_TYPE_INT, width, 
						"height", G_TYPE_INT, height, 
						NULL);
						
				g_object_set(G_OBJECT(scale_element), "caps", size_caps, NULL);
				
				gdk_threads_enter();
				gtk_window_resize(GTK_WINDOW(videoWindow), width, height);
				gdk_threads_leave();
			}
			
			
			//Clock stuff:
			//https://stackoverflow.com/questions/34294755/mux-klv-data-with-h264-by-mpegtsmux
			GstClock *clock = nullptr;
            GstClockTime abs_time, base_time;

            //TODO try with appscr instead of pipeline
            GST_OBJECT_LOCK (pipeline);
                // TODO try gst_pipeline_get_pipeline_clock
                clock = GST_ELEMENT_CLOCK (pipeline);
                if(!clock)
                {
                    GST_ERROR("%s","NO CLOCK");
                    exit(1);
                }
                base_time = GST_ELEMENT (pipeline)->base_time;
                gst_object_ref (clock);
                    abs_time = gst_clock_get_time (clock);
                gst_object_unref (clock);
            GST_OBJECT_UNLOCK (pipeline);
                
            guint64 myPTS_timestamp = abs_time - base_time;
            GST_DEBUG("myPTS_timestamp: %lu",myPTS_timestamp);
            
            
            guint64 myDuration = gst_util_uint64_scale_int (
                                      1, 30 * GST_MSECOND, 1);

			
			
            GST_BUFFER_PTS (video_frame_GstBuffer) = myPTS_timestamp;
            //GST_BUFFER_DURATION (video_frame_GstBuffer) = myDuration;
			
			
	        //// Timestamp stuff, based on system clock, NOT gst clock! works, but only without KLV:
			//// this does not seem important, but PTS does... 0_o
            //GST_BUFFER_TIMESTAMP(video_frame_GstBuffer) = GST_CLOCK_TIME_NONE;
			//GST_BUFFER_PTS(video_frame_GstBuffer) = frame_capture_PTS;
			
			
			
			// push buffer into gstreamer pipeline
			ret = gst_app_src_push_buffer(GST_APP_SRC(appsrc_video), video_frame_GstBuffer);
			
		    if(ret != GST_FLOW_OK)
			{
				break;
			}
			
			
			
			
			//-----------------------------------------------------------------
        	//{ KLV buffer injection
	        
            // hundred thousand bytes
            #define KLV_BUFFER_MAX_SIZE 100000            
            guint8 klv_data[KLV_BUFFER_MAX_SIZE];
            memset(klv_data, 0, KLV_BUFFER_MAX_SIZE);

            guint8 * klv_data_end_ptr = &klv_data[0];

            // Write string  "0123456789ABCDEF"
            gchar const * test16byteString   = "0123456789ABCDEF";
            klv_data_end_ptr = write_homebrew_KLV_item(
                klv_data_end_ptr, // guint8 * klv_buffer,
                KLV_KEY_string, // uint64_t key_host,
                strlen(test16byteString), // uint64_t len_host, 
                (guint8 const *) test16byteString // guint8 * value_ptr
            );
           
            // Write frame count KLV_KEY_image_frame_number
            uint64_t value_be = htobe64((uint64_t)frames);
            klv_data_end_ptr = write_homebrew_KLV_item(
                klv_data_end_ptr, // guint8 * klv_buffer,
                KLV_KEY_image_frame_number, // uint64_t key_host,
                sizeof(uint64_t), // uint64_t len_host, 
                (guint8 const *) &value_be // guint8 * value_ptr
            );
            
            // Write frame timestamp KLV_KEY_image_capture_time_stamp
            value_be = htobe64((uint64_t)frame_capture_PTS);
            klv_data_end_ptr = write_homebrew_KLV_item(
                klv_data_end_ptr, // guint8 * klv_buffer,
                KLV_KEY_image_capture_time_stamp, // uint64_t key_host,
                sizeof(uint64_t), // uint64_t len_host, 
                (guint8 const *) &value_be // guint8 * value_ptr
            );
            

            // Write string "!frame meta end!"
            gchar const * frameMetaEndString = "!frame meta end!";
            klv_data_end_ptr = write_homebrew_KLV_item(
                klv_data_end_ptr, // guint8 * klv_buffer,
                KLV_KEY_string, // uint64_t key_host,
                strlen(frameMetaEndString), // uint64_t len_host, 
                (guint8 const *) frameMetaEndString // guint8 * value_ptr
            );

                   
            guint klvBufferSize = klv_data_end_ptr - (&klv_data[0]);
            
         
            // make empty buffer 
            klv_frame_GstBuffer = gst_buffer_new();

            // alloc memory for buffer
            GstMemory *gstMem = gst_allocator_alloc (NULL, klvBufferSize, NULL);
            
            // add the buffer
            gst_buffer_append_memory (klv_frame_GstBuffer, gstMem);
            
            // get WRITE access to the memory and fill with our KLV data 
            GstMapInfo gstInfo;
            gst_buffer_map (klv_frame_GstBuffer, &gstInfo, GST_MAP_WRITE);
           
            //do the writing
            memcpy (gstInfo.data, klv_data, gstInfo.size);
            
            gst_buffer_unmap (klv_frame_GstBuffer, &gstInfo);

				
	        // Timestamp stuff onto KLV buffer itself to get associated 
	        // with video buffer:
            //GST_BUFFER_TIMESTAMP(klv_frame_GstBuffer) = GST_CLOCK_TIME_NONE;
			//GST_BUFFER_PTS(klv_frame_GstBuffer) = frame_capture_PTS;
			
			GST_BUFFER_PTS (klv_frame_GstBuffer) = myPTS_timestamp;
            //GST_BUFFER_DURATION (klv_frame_GstBuffer) = myDuration;
			

            // This function takes ownership of the buffer. so no unref!
			ret = gst_app_src_push_buffer(
			  GST_APP_SRC(appsrc_klv), 
			  klv_frame_GstBuffer);

			if(ret != GST_FLOW_OK)
			{
				break;
			}
			
			
        	//} ---------------------------------------------------------------
							
		}
		
		
		
		// statistics bookkeeping:
		frames++;
		if(image.nframe > lastframe)
		{
			lostframes += image.nframe - (lastframe + 1);
		}
		lastframe = image.nframe;
		curtime = getcurus();
		
		// update presentation of statistics each second:
		if(curtime - prevtime > 1000000) 
		{
			snprintf(
			    statistics_string, 
			    256, 
			    "Acquisition [ captured: %lu, skipped: %lu, fps: %.2f ]", 
			    frames, 
			    lostframes, 
			    1000000.0 * (frames - prevframes) / (curtime - prevtime)
			);
			gtk_window_set_title(GTK_WINDOW(videoWindow), statistics_string);
			
			prevframes = frames;
			prevtime = curtime;
		}
		
	}
    //} end "main loop"
	
	
	
    //{ cleanup	
	if(appsrc_video_caps)
		gst_caps_unref(appsrc_video_caps);
	if(size_caps)
		gst_caps_unref(size_caps);
	gst_app_src_end_of_stream(GST_APP_SRC(appsrc_video));
	gst_element_set_state(pipeline, GST_STATE_NULL);
	gst_object_unref(GST_OBJECT(pipeline));	
	
exit:
	gdk_threads_enter();
	gtk_widget_destroy(videoWindow);
	if(quitting)
		gtk_main_quit();
	xiStopAcquisition(handle);
	acquire = FALSE;
	xiCloseDevice(handle);
	handle = INVALID_HANDLE_VALUE;
	videoThread = 0;
	gdk_threads_leave();
	//} end cleanup	
	
	return 0;
}




struct ctrl_window {
    GtkWidget *window, 
        *boxmain, 
        *boxgpi, 
        *boxx, 
        *boxy, 
        *gpi1, 
        *gpi2, 
        *gpi3, 
        *gpi4, 
        *labelexp, 
        *exp, 
        *labelgain, 
        *gain, 
        *labelx0, 
        *x0, 
        *labely0, 
        *y0, 
        *labelcx, 
        *cx, 
        *labelcy, 
        *cy, 
        *raw, 
        *show, 
        *run;
};

gboolean time_handler(ctrl_window *ctrl) 
{
	int level = 0;
	if(acquire && handle != INVALID_HANDLE_VALUE) {
		xiSetParamInt(handle, XI_PRM_GPI_SELECTOR, 1);
		xiGetParamInt(handle, XI_PRM_GPI_LEVEL, &level);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ctrl->gpi1), level);
		xiSetParamInt(handle, XI_PRM_GPI_SELECTOR, 2);
		xiGetParamInt(handle, XI_PRM_GPI_LEVEL, &level);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ctrl->gpi2), level);
		xiSetParamInt(handle, XI_PRM_GPI_SELECTOR, 3);
		xiGetParamInt(handle, XI_PRM_GPI_LEVEL, &level);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ctrl->gpi3), level);
		xiSetParamInt(handle, XI_PRM_GPI_SELECTOR, 4);
		xiGetParamInt(handle, XI_PRM_GPI_LEVEL, &level);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ctrl->gpi4), level);
	}
	gtk_toggle_button_set_inconsistent(
	  GTK_TOGGLE_BUTTON(ctrl->run), 
	  gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ctrl->run)) != acquire);
	gtk_widget_set_sensitive(ctrl->boxx, acquire);
	gtk_widget_set_sensitive(ctrl->boxy, acquire);
	gtk_widget_set_sensitive(ctrl->exp, acquire);
	gtk_widget_set_sensitive(ctrl->gain, acquire);
	return TRUE;
}

gboolean update_x0(GtkAdjustment *adj, gpointer) {
	roix0 = gtk_adjustment_get_value(adj);
	if(roicx + roix0 > maxcx){
		roix0 = maxcx - roicx;
		gtk_adjustment_set_value(adj, roix0);
	}
	xiSetParamInt(handle, XI_PRM_OFFSET_X, roix0);
	return TRUE;
}

gboolean update_y0(GtkAdjustment *adj, gpointer) {
	roiy0 = gtk_adjustment_get_value(adj);
	if(roicy + roiy0 > maxcy){
		roiy0 = maxcy - roicy;
		gtk_adjustment_set_value(adj, roiy0);
	}
	xiSetParamInt(handle, XI_PRM_OFFSET_Y, roiy0);
	return TRUE;
}

gboolean update_cx(GtkAdjustment *adj, gpointer) {
	roicx = gtk_adjustment_get_value(adj);
	if(roix0 + roicx > maxcx){
		roicx = maxcx - roix0;
		gtk_adjustment_set_value(adj, roicx);
	}
	xiSetParamInt(handle, XI_PRM_WIDTH, roicx);
	return TRUE;
}

gboolean update_cy(GtkAdjustment *adj, gpointer) {
	roicy = gtk_adjustment_get_value(adj);
	if(roiy0 + roicy > maxcy) {
		roicy = maxcy - roiy0;
		gtk_adjustment_set_value(adj, roicy);
	}
	xiSetParamInt(handle, XI_PRM_HEIGHT, roicy);
	return TRUE;
}

gboolean update_exposure(GtkAdjustment *adj, gpointer) {
	xiSetParamInt(handle, XI_PRM_EXPOSURE, 1000*gtk_adjustment_get_value(adj));		
	return TRUE;
}

gboolean update_gain(GtkAdjustment *adj, gpointer) {
	xiSetParamFloat(handle, XI_PRM_GAIN, gtk_adjustment_get_value(adj));
	return TRUE;
}

gboolean update_raw(GtkToggleButton *raw, ctrl_window *ctrl) {
	if(handle != INVALID_HANDLE_VALUE) {
		float mingain, maxgain;
		xiSetParamInt(handle, XI_PRM_IMAGE_DATA_FORMAT, gtk_toggle_button_get_active(raw) ? XI_RAW8 : XI_RGB32);
		xiGetParamFloat(handle, XI_PRM_GAIN XI_PRM_INFO_MIN, &mingain);
		xiGetParamFloat(handle, XI_PRM_GAIN XI_PRM_INFO_MAX, &maxgain);
		xiGetParamInt(handle, XI_PRM_WIDTH XI_PRM_INFO_MAX, &maxcx);
		xiGetParamInt(handle, XI_PRM_HEIGHT XI_PRM_INFO_MAX, &maxcy);
		roicx = maxcx;
		roicy = maxcy;
		roix0 = 0;
		roiy0 = 0;
		
		float midgain = (maxgain + mingain) * 0.5f;
		
		//gtk_adjustment_configure(gtk_range_get_adjustment(GTK_RANGE(ctrl->gain)), mingain, mingain, maxgain, 0.1, 1, 0);
		gtk_adjustment_configure(gtk_range_get_adjustment(GTK_RANGE(ctrl->gain)), midgain, mingain, maxgain, 0.1, 1, 0);
		gtk_adjustment_configure(gtk_spin_button_get_adjustment(GTK_SPIN_BUTTON(ctrl->x0)), roix0, 0, maxcx-4, 2, 20, 0);
		gtk_adjustment_configure(gtk_spin_button_get_adjustment(GTK_SPIN_BUTTON(ctrl->y0)), roiy0, 0, maxcy-2, 2, 20, 0);
		gtk_adjustment_configure(gtk_spin_button_get_adjustment(GTK_SPIN_BUTTON(ctrl->cx)), roicx, 4, maxcx, 4, 20, 0);
		gtk_adjustment_configure(gtk_spin_button_get_adjustment(GTK_SPIN_BUTTON(ctrl->cy)), roicy, 2, maxcy, 2, 20, 0);
		
		//xiSetParamFloat(handle, XI_PRM_GAIN, mingain);
		xiSetParamFloat(handle, XI_PRM_GAIN, midgain);
		xiSetParamInt(handle, XI_PRM_OFFSET_X, roix0);
		xiSetParamInt(handle, XI_PRM_OFFSET_Y, roiy0);
		xiSetParamInt(handle, XI_PRM_WIDTH, roicx);
		xiSetParamInt(handle, XI_PRM_HEIGHT, roicy);
		//exposure doesn't seem to be affected by format change
	}
	return TRUE;
}

gboolean update_show(GtkToggleButton *show, gpointer) {
	render = gtk_toggle_button_get_active(show);
	return TRUE;
}

gboolean update_run(GtkToggleButton *run, ctrl_window *ctrl) {
	gtk_toggle_button_set_inconsistent(run, false);
	acquire = gtk_toggle_button_get_active(run);
	if(acquire && handle == INVALID_HANDLE_VALUE) {
		DWORD nIndex = 0;
		char* env = getenv("CAM_INDEX");
		if(env) {
			nIndex = atoi(env);
		}
		DWORD tmp;
		xiGetNumberDevices(&tmp); //rescan available devices
		if(xiOpenDevice(nIndex, &handle) != XI_OK) {
			printf("Couldn't setup camera!\n");
			acquire = FALSE;
			return TRUE;
		}
		update_raw(GTK_TOGGLE_BUTTON(ctrl->raw), ctrl);
		int isColor = 0;
		xiGetParamInt(handle, XI_PRM_IMAGE_IS_COLOR, &isColor);
		if(isColor)
			xiSetParamInt(handle, XI_PRM_AUTO_WB, 1);

        // set exposure from CLI arg
		xiSetParamInt(handle, XI_PRM_EXPOSURE, 1000 * globalAppArgs.exposure_ms);
		gtk_adjustment_set_value(
		    gtk_range_get_adjustment(GTK_RANGE(ctrl->exp)), 
		    globalAppArgs.exposure_ms);
		
		if(pthread_create(&videoThread, NULL, videoDisplay, NULL))
			exit(1);
	}
	gtk_widget_set_sensitive(ctrl->boxx, acquire);
	gtk_widget_set_sensitive(ctrl->boxy, acquire);
	gtk_widget_set_sensitive(ctrl->exp, acquire);
	gtk_widget_set_sensitive(ctrl->gain, acquire);
	return TRUE;
}



gboolean start_cb(ctrl_window* ctrl) {
	//start acquisition
	update_run(GTK_TOGGLE_BUTTON(ctrl->run), ctrl);
	return FALSE;
}


//-----------------------------------------------------------------------------
int main(int argc, char **argv)
{    
    //{ parse args ------------------------------------------------------------
    
    /* Default values. */
    globalAppArgs.doLocalDisplay = false;
    globalAppArgs.output_file = nullptr;
    globalAppArgs.exposure_ms = 30;
    globalAppArgs.IP = "192.168.0.169";
    globalAppArgs.port = "5001";

   
    /* Parse our arguments; every option seen by parse_opt will
       be reflected in arguments. */
    argp_parse (&argp, argc, argv, 0, 0, &globalAppArgs);

    printf ("IP = %s\nPort = %s\nExposure = %i\n"
            "OUTPUT_FILE = %s\ndisplay locally = %s\n",
            globalAppArgs.IP, 
            globalAppArgs.port,
            globalAppArgs.exposure_ms,
            globalAppArgs.output_file ? globalAppArgs.output_file : "-none-",
            globalAppArgs.doLocalDisplay ? "yes" : "no");

    //exit(0);
    //} end parse args --------------------------------------------------------
    
    

	ctrl_window ctrl;
#ifdef GDK_WINDOWING_X11
	XInitThreads();
#endif
#if !GLIB_CHECK_VERSION(2, 31, 0)
	g_thread_init(NULL);
#endif
	gdk_threads_init();
	gdk_threads_enter();
	gst_init(&argc, &argv);
	gtk_init(&argc, &argv);
	//create widgets
	ctrl.window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	ctrl.boxmain = gtk_vbox_new(FALSE, 0);
	ctrl.boxgpi = gtk_hbox_new(TRUE, 0);
	ctrl.boxx = gtk_hbox_new(FALSE, 0);
	ctrl.boxy = gtk_hbox_new(FALSE, 0);
	ctrl.labelexp = gtk_label_new("Exposure (ms)");
	ctrl.labelgain = gtk_label_new("Gain (dB)");
	ctrl.labelx0 = gtk_label_new("x0");
	ctrl.labelcx = gtk_label_new("cx");
	ctrl.labely0 = gtk_label_new("y0");
	ctrl.labelcy = gtk_label_new("cy");
	ctrl.gpi1 = gtk_check_button_new_with_label("GPI1");
	ctrl.gpi2 = gtk_check_button_new_with_label("GPI2");
	ctrl.gpi3 = gtk_check_button_new_with_label("GPI3");
	ctrl.gpi4 = gtk_check_button_new_with_label("GPI4");
	ctrl.exp = gtk_hscale_new_with_range(1, 1000, 1);
	ctrl.gain = gtk_hscale_new_with_range(0, 1, 0.1); //use dummy limits
	ctrl.x0 = gtk_spin_button_new_with_range(0, 128, 2); //use dummy max limit
	ctrl.y0 = gtk_spin_button_new_with_range(0, 128, 2); //use dummy max limit
	ctrl.cx = gtk_spin_button_new_with_range(4, 128, 4); //use dummy max limit
	ctrl.cy = gtk_spin_button_new_with_range(2, 128, 2); //use dummy max limit
	ctrl.raw = gtk_toggle_button_new_with_label("Display RAW data");
	ctrl.show = gtk_toggle_button_new_with_label("Live view");
	ctrl.run = gtk_toggle_button_new_with_label("Acquisition");
	//tune them
	gtk_window_set_title(GTK_WINDOW(ctrl.window), "streamViewer control");
	gtk_window_set_keep_above(GTK_WINDOW(ctrl.window), TRUE);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ctrl.raw), TRUE);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ctrl.show), TRUE);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ctrl.run), TRUE); //actual start is delayed by 100ms, see below
	gtk_widget_set_sensitive(ctrl.boxgpi, FALSE);
	gtk_scale_set_digits(GTK_SCALE(ctrl.exp), 0);
	gtk_range_set_update_policy(GTK_RANGE(ctrl.exp), GTK_UPDATE_DISCONTINUOUS);
	gtk_range_set_update_policy(GTK_RANGE(ctrl.gain), GTK_UPDATE_DISCONTINUOUS);
	gtk_adjustment_set_value(gtk_range_get_adjustment(GTK_RANGE(ctrl.exp)), 10);
	gtk_scale_set_value_pos(GTK_SCALE(ctrl.exp), GTK_POS_RIGHT);
	gtk_scale_set_value_pos(GTK_SCALE(ctrl.gain), GTK_POS_RIGHT);
	gtk_widget_set_sensitive(ctrl.boxx, FALSE);
	gtk_widget_set_sensitive(ctrl.boxy, FALSE);
	gtk_widget_set_sensitive(ctrl.exp, FALSE);
	gtk_widget_set_sensitive(ctrl.gain, FALSE);
	//pack everything into window
	gtk_container_add(GTK_CONTAINER(ctrl.boxgpi), ctrl.gpi1);
	gtk_container_add(GTK_CONTAINER(ctrl.boxgpi), ctrl.gpi2);
	gtk_container_add(GTK_CONTAINER(ctrl.boxgpi), ctrl.gpi3);
	gtk_container_add(GTK_CONTAINER(ctrl.boxgpi), ctrl.gpi4);
	gtk_container_add(GTK_CONTAINER(ctrl.boxmain), ctrl.boxgpi);
	gtk_container_add(GTK_CONTAINER(ctrl.boxx), ctrl.labelx0);
	gtk_container_add(GTK_CONTAINER(ctrl.boxx), ctrl.x0);
	gtk_container_add(GTK_CONTAINER(ctrl.boxy), ctrl.labely0);
	gtk_container_add(GTK_CONTAINER(ctrl.boxy), ctrl.y0);
	gtk_container_add(GTK_CONTAINER(ctrl.boxx), ctrl.labelcx);
	gtk_container_add(GTK_CONTAINER(ctrl.boxx), ctrl.cx);
	gtk_container_add(GTK_CONTAINER(ctrl.boxy), ctrl.labelcy);
	gtk_container_add(GTK_CONTAINER(ctrl.boxy), ctrl.cy);
	gtk_container_add(GTK_CONTAINER(ctrl.boxmain), ctrl.boxx);
	gtk_container_add(GTK_CONTAINER(ctrl.boxmain), ctrl.boxy);	
	gtk_container_add(GTK_CONTAINER(ctrl.boxmain), ctrl.labelexp);
	gtk_container_add(GTK_CONTAINER(ctrl.boxmain), ctrl.exp);
	gtk_container_add(GTK_CONTAINER(ctrl.boxmain), ctrl.labelgain);
	gtk_container_add(GTK_CONTAINER(ctrl.boxmain), ctrl.gain);
	gtk_container_add(GTK_CONTAINER(ctrl.boxmain), ctrl.raw);
	gtk_container_add(GTK_CONTAINER(ctrl.boxmain), ctrl.show);
	gtk_container_add(GTK_CONTAINER(ctrl.boxmain), ctrl.run);
	gtk_container_add(GTK_CONTAINER(ctrl.window), ctrl.boxmain);
	//register handlers
	gdk_threads_add_timeout(1000, (GSourceFunc)time_handler, (gpointer)&ctrl);
	gdk_threads_add_timeout(100, (GSourceFunc)start_cb, (gpointer)&ctrl); //only way I found to make sure window is displayed right away
	g_signal_connect(ctrl.window, "delete_event", G_CALLBACK(close_cb), (gpointer)TRUE);
	g_signal_connect(gtk_range_get_adjustment(GTK_RANGE(ctrl.gain)), "value_changed", G_CALLBACK(update_gain), NULL);
	g_signal_connect(gtk_range_get_adjustment(GTK_RANGE(ctrl.exp)), "value_changed", G_CALLBACK(update_exposure), NULL);
	g_signal_connect(gtk_spin_button_get_adjustment(GTK_SPIN_BUTTON(ctrl.x0)), "value_changed", G_CALLBACK(update_x0), NULL);
	g_signal_connect(gtk_spin_button_get_adjustment(GTK_SPIN_BUTTON(ctrl.y0)), "value_changed", G_CALLBACK(update_y0), NULL);
	g_signal_connect(gtk_spin_button_get_adjustment(GTK_SPIN_BUTTON(ctrl.cx)), "value_changed", G_CALLBACK(update_cx), NULL);
	g_signal_connect(gtk_spin_button_get_adjustment(GTK_SPIN_BUTTON(ctrl.cy)), "value_changed", G_CALLBACK(update_cy), NULL);
	g_signal_connect(ctrl.raw, "toggled", G_CALLBACK(update_raw), (gpointer)&ctrl);
	g_signal_connect(ctrl.show, "toggled", G_CALLBACK(update_show), NULL);
	g_signal_connect(ctrl.run, "toggled", G_CALLBACK(update_run), (gpointer)&ctrl);
	//show window
	gtk_widget_show_all(ctrl.window);
	//start the main loop
	gtk_main();
	//exit
	gdk_threads_leave();
	acquire = FALSE;
	if(videoThread)
		pthread_join(videoThread, NULL);
	return 0;
}
