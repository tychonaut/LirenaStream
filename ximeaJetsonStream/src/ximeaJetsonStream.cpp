
#include "LirenaOptions.h"
#include "KLV_appsrc.h"
#include "StreamViewerGUI.h"


#include <sys/time.h>
#include <pthread.h>
#include <string.h>


#include <gst/app/gstappsrc.h>

#include <gst/video/videooverlay.h>

#include <m3api/xiApi.h> //Ximea API


//----------------------------------------------------------------------------
// Macros
#define STATUS_STRING_LENGTH 4096


//----------------------------------------------------------------------------
// Type forwards:
struct LirenaCamStreamApp;
struct LirenaCamera;
//struct LirenaCamStreamLocalDisplay;



//----------------------------------------------------------------------------
// Type definitions



struct LirenaCamera
{
	HANDLE cameraHandle = INVALID_HANDLE_VALUE;

	BOOLEAN acquire = TRUE;
	BOOLEAN quitting = TRUE;
	BOOLEAN doRender = TRUE;

	int maxcx, maxcy, roix0, roiy0, roicx, roicy;
};

struct LirenaFrameMeta
{
    char statusString[STATUS_STRING_LENGTH];

    unsigned long current_time = 0;
    unsigned long prev_time = 0;

	unsigned long current_sensed_frame_count = 0;  
    unsigned long current_captured_frame_count = 0;
    unsigned long current_preprocessed_frame_count = 0;

	//derived values:
	unsigned long current_frame_duration = 0;
    unsigned long lost_frames_count = 0;
};


struct LirenaCamStreamApp
{
	LirenaCamStreamConfig config;

	LirenaCamera camState;

	LirenaCamStreamLocalDisplay localDisplay;
};







//----------------------------------------------------------------------------
int main(int argc, char **argv);




// ----------------------------------------------------------------------------
//{ Global variables; TODO rename, organize, replace by local aggregates


// TODO replace by ApplicationState::appconfig
//LirenaCamStreamConfig globalLirenaCamStreamConfig ;


//HANDLE cameraHandle = INVALID_HANDLE_VALUE;

BOOLEAN acquire, quitting, render = TRUE;

int maxcx, maxcy, roix0, roiy0, roicx, roicy;


pthread_t videoThread;
guintptr window_handle;


//}

// creates zero-inited instance and parses and sets args/options
LirenaCamStreamApp* lirena_camStreamApp_create(int argc, char **argv)
{
    LirenaCamStreamApp* appPtr = 
		//init all to zero/nullptr/false
		(LirenaCamStreamApp*) g_malloc0(sizeof(LirenaCamStreamApp));
	appPtr->camState.cameraHandle = INVALID_HANDLE_VALUE;

	appPtr->camState.acquire = TRUE;
	appPtr->camState.doRender = TRUE;
	appPtr->camState.quitting = TRUE;

	appPtr->config = parseArguments(argc, argv);

	//TODO continue;

	return appPtr;
}

void lirena_camStreamApp_destroy(LirenaCamStreamApp* appPtr)
{
	g_free(appPtr);
}



//-----------------------------------------------------------------------------
int main(int argc, char **argv)
{    
	LirenaCamStreamApp*  appPtr = lirena_camStreamApp_create(argc, argv);

	

	//globalLirenaCamStreamConfig = 
    

	//LirenaCamStreamControlWindow controlWindow;
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
	appPtr->localDisplay.controlWindow.window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	appPtr->localDisplay.controlWindow.boxmain = gtk_vbox_new(FALSE, 0);
	appPtr->localDisplay.controlWindow.boxgpi = gtk_hbox_new(TRUE, 0);
	appPtr->localDisplay.controlWindow.boxx = gtk_hbox_new(FALSE, 0);
	appPtr->localDisplay.controlWindow.boxy = gtk_hbox_new(FALSE, 0);
	appPtr->localDisplay.controlWindow.labelexp = gtk_label_new("Exposure (ms)");
	appPtr->localDisplay.controlWindow.labelgain = gtk_label_new("Gain (dB)");
	appPtr->localDisplay.controlWindow.labelx0 = gtk_label_new("x0");
	appPtr->localDisplay.controlWindow.labelcx = gtk_label_new("cx");
	appPtr->localDisplay.controlWindow.labely0 = gtk_label_new("y0");
	appPtr->localDisplay.controlWindow.labelcy = gtk_label_new("cy");
	appPtr->localDisplay.controlWindow.gpi1 = gtk_check_button_new_with_label("GPI1");
	appPtr->localDisplay.controlWindow.gpi2 = gtk_check_button_new_with_label("GPI2");
	appPtr->localDisplay.controlWindow.gpi3 = gtk_check_button_new_with_label("GPI3");
	appPtr->localDisplay.controlWindow.gpi4 = gtk_check_button_new_with_label("GPI4");
	appPtr->localDisplay.controlWindow.exp = gtk_hscale_new_with_range(1, 1000, 1);
	appPtr->localDisplay.controlWindow.gain = gtk_hscale_new_with_range(0, 1, 0.1); //use dummy limits
	appPtr->localDisplay.controlWindow.x0 = gtk_spin_button_new_with_range(0, 128, 2); //use dummy max limit
	appPtr->localDisplay.controlWindow.y0 = gtk_spin_button_new_with_range(0, 128, 2); //use dummy max limit
	appPtr->localDisplay.controlWindow.cx = gtk_spin_button_new_with_range(4, 128, 4); //use dummy max limit
	appPtr->localDisplay.controlWindow.cy = gtk_spin_button_new_with_range(2, 128, 2); //use dummy max limit
	appPtr->localDisplay.controlWindow.raw = gtk_toggle_button_new_with_label("Display RAW data");
	appPtr->localDisplay.controlWindow.show = gtk_toggle_button_new_with_label("Live view");
	appPtr->localDisplay.controlWindow.run = gtk_toggle_button_new_with_label("Acquisition");
	//tune them
	gtk_window_set_title(GTK_WINDOW(appPtr->localDisplay.controlWindow.window), "streamViewer control");
	gtk_window_set_keep_above(GTK_WINDOW(appPtr->localDisplay.controlWindow.window), TRUE);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(appPtr->localDisplay.controlWindow.raw), TRUE);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(appPtr->localDisplay.controlWindow.show), TRUE);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(appPtr->localDisplay.controlWindow.run), TRUE); //actual start is delayed by 100ms, see below
	gtk_widget_set_sensitive(appPtr->localDisplay.controlWindow.boxgpi, FALSE);
	gtk_scale_set_digits(GTK_SCALE(appPtr->localDisplay.controlWindow.exp), 0);
	gtk_range_set_update_policy(GTK_RANGE(appPtr->localDisplay.controlWindow.exp), GTK_UPDATE_DISCONTINUOUS);
	gtk_range_set_update_policy(GTK_RANGE(appPtr->localDisplay.controlWindow.gain), GTK_UPDATE_DISCONTINUOUS);
	gtk_adjustment_set_value(gtk_range_get_adjustment(GTK_RANGE(appPtr->localDisplay.controlWindow.exp)), 10);
	gtk_scale_set_value_pos(GTK_SCALE(appPtr->localDisplay.controlWindow.exp), GTK_POS_RIGHT);
	gtk_scale_set_value_pos(GTK_SCALE(appPtr->localDisplay.controlWindow.gain), GTK_POS_RIGHT);
	gtk_widget_set_sensitive(appPtr->localDisplay.controlWindow.boxx, FALSE);
	gtk_widget_set_sensitive(appPtr->localDisplay.controlWindow.boxy, FALSE);
	gtk_widget_set_sensitive(appPtr->localDisplay.controlWindow.exp, FALSE);
	gtk_widget_set_sensitive(appPtr->localDisplay.controlWindow.gain, FALSE);
	//pack everything into window
	gtk_container_add(GTK_CONTAINER(appPtr->localDisplay.controlWindow.boxgpi), appPtr->localDisplay.controlWindow.gpi1);
	gtk_container_add(GTK_CONTAINER(appPtr->localDisplay.controlWindow.boxgpi), appPtr->localDisplay.controlWindow.gpi2);
	gtk_container_add(GTK_CONTAINER(appPtr->localDisplay.controlWindow.boxgpi), appPtr->localDisplay.controlWindow.gpi3);
	gtk_container_add(GTK_CONTAINER(appPtr->localDisplay.controlWindow.boxgpi), appPtr->localDisplay.controlWindow.gpi4);
	gtk_container_add(GTK_CONTAINER(appPtr->localDisplay.controlWindow.boxmain), appPtr->localDisplay.controlWindow.boxgpi);
	gtk_container_add(GTK_CONTAINER(appPtr->localDisplay.controlWindow.boxx), appPtr->localDisplay.controlWindow.labelx0);
	gtk_container_add(GTK_CONTAINER(appPtr->localDisplay.controlWindow.boxx), appPtr->localDisplay.controlWindow.x0);
	gtk_container_add(GTK_CONTAINER(appPtr->localDisplay.controlWindow.boxy), appPtr->localDisplay.controlWindow.labely0);
	gtk_container_add(GTK_CONTAINER(appPtr->localDisplay.controlWindow.boxy), appPtr->localDisplay.controlWindow.y0);
	gtk_container_add(GTK_CONTAINER(appPtr->localDisplay.controlWindow.boxx), appPtr->localDisplay.controlWindow.labelcx);
	gtk_container_add(GTK_CONTAINER(appPtr->localDisplay.controlWindow.boxx), appPtr->localDisplay.controlWindow.cx);
	gtk_container_add(GTK_CONTAINER(appPtr->localDisplay.controlWindow.boxy), appPtr->localDisplay.controlWindow.labelcy);
	gtk_container_add(GTK_CONTAINER(appPtr->localDisplay.controlWindow.boxy), appPtr->localDisplay.controlWindow.cy);
	gtk_container_add(GTK_CONTAINER(appPtr->localDisplay.controlWindow.boxmain), appPtr->localDisplay.controlWindow.boxx);
	gtk_container_add(GTK_CONTAINER(appPtr->localDisplay.controlWindow.boxmain), appPtr->localDisplay.controlWindow.boxy);	
	gtk_container_add(GTK_CONTAINER(appPtr->localDisplay.controlWindow.boxmain), appPtr->localDisplay.controlWindow.labelexp);
	gtk_container_add(GTK_CONTAINER(appPtr->localDisplay.controlWindow.boxmain), appPtr->localDisplay.controlWindow.exp);
	gtk_container_add(GTK_CONTAINER(appPtr->localDisplay.controlWindow.boxmain), appPtr->localDisplay.controlWindow.labelgain);
	gtk_container_add(GTK_CONTAINER(appPtr->localDisplay.controlWindow.boxmain), appPtr->localDisplay.controlWindow.gain);
	gtk_container_add(GTK_CONTAINER(appPtr->localDisplay.controlWindow.boxmain), appPtr->localDisplay.controlWindow.raw);
	gtk_container_add(GTK_CONTAINER(appPtr->localDisplay.controlWindow.boxmain), appPtr->localDisplay.controlWindow.show);
	gtk_container_add(GTK_CONTAINER(appPtr->localDisplay.controlWindow.boxmain), appPtr->localDisplay.controlWindow.run);
	gtk_container_add(GTK_CONTAINER(appPtr->localDisplay.controlWindow.window), appPtr->localDisplay.controlWindow.boxmain);
	//register handlers
	gdk_threads_add_timeout(1000, (GSourceFunc)time_handler, (gpointer)appPtr);//.localDisplay.controlWindow);
	//only way I (ximea dev) found to make sure window is displayed right away:
	gdk_threads_add_timeout(100, (GSourceFunc)start_cb, (gpointer)appPtr);//appPtr->localDisplay.controlWindow); 

	g_signal_connect(appPtr->localDisplay.controlWindow.window, 
		"delete_event", G_CALLBACK(close_cb), (gpointer)TRUE);
	g_signal_connect(gtk_range_get_adjustment(GTK_RANGE(appPtr->localDisplay.controlWindow.gain)), 
		"value_changed", G_CALLBACK(update_gain), &appPtr->camState);
	g_signal_connect(gtk_range_get_adjustment(GTK_RANGE(appPtr->localDisplay.controlWindow.exp)), 
		"value_changed", G_CALLBACK(update_exposure), &appPtr->camState);
	g_signal_connect(gtk_spin_button_get_adjustment(GTK_SPIN_BUTTON(appPtr->localDisplay.controlWindow.x0)), 
		"value_changed", G_CALLBACK(update_x0), &appPtr->camState);
	g_signal_connect(gtk_spin_button_get_adjustment(GTK_SPIN_BUTTON(appPtr->localDisplay.controlWindow.y0)), 
		"value_changed", G_CALLBACK(update_y0), &appPtr->camState);
	g_signal_connect(gtk_spin_button_get_adjustment(GTK_SPIN_BUTTON(appPtr->localDisplay.controlWindow.cx)), 
		"value_changed", G_CALLBACK(update_cx), &appPtr->camState);
	g_signal_connect(gtk_spin_button_get_adjustment(GTK_SPIN_BUTTON(appPtr->localDisplay.controlWindow.cy)), 
		"value_changed", G_CALLBACK(update_cy), &appPtr->camState);
	g_signal_connect(appPtr->localDisplay.controlWindow.raw, "toggled",
	 	G_CALLBACK(update_raw), (gpointer)appPtr);
	g_signal_connect(appPtr->localDisplay.controlWindow.show, "toggled",
	 	G_CALLBACK(update_show), &appPtr->camState);
	g_signal_connect(appPtr->localDisplay.controlWindow.run, "toggled", 
		G_CALLBACK(update_run), (gpointer)appPtr);//appPtr->localDisplay.controlWindow);

	//show window
	gtk_widget_show_all(appPtr->localDisplay.controlWindow.window);
	//start the main loop
	gtk_main();


	//exit
	gdk_threads_leave();
	acquire = FALSE;
	if(videoThread)
	{
		pthread_join(videoThread, NULL);
	}

	lirena_camStreamApp_destroy(appPtr);
	appPtr = NULL;
	
	return 0;
}




//----------------------------------------------------------------------------
//TODO outsourc pure GUI code





// TODO remove streaming logic from this gui function!
//void* videoDisplay(void*) 
void* videoDisplay(void* appVoidPtr)
{	
	LirenaCamStreamApp* app = (LirenaCamStreamApp*) appVoidPtr;

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
	
	if(xiStartAcquisition(app->camState.cameraHandle) != XI_OK) 
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
      
    if(app->config.doLocalDisplay)
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
    
    if(app->config.output_file)
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
            app->config.output_file);
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
        app->config.IP,
        app->config.port 
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
		if(xiGetImage(app->camState.cameraHandle, 5000, &image) != XI_OK)
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
            klv_data_end_ptr = write_KLV_item(
                klv_data_end_ptr, // guint8 * klv_buffer,
                KLV_KEY_string, // uint64_t key_host,
                strlen(test16byteString), // uint64_t len_host, 
                (guint8 const *) test16byteString // guint8 * value_ptr
            );
           
            // Write frame count KLV_KEY_image_frame_number
            uint64_t value_be = htobe64((uint64_t)frames);
            klv_data_end_ptr = write_KLV_item(
                klv_data_end_ptr, // guint8 * klv_buffer,
                KLV_KEY_image_captured_frame_number, // uint64_t key_host,
                sizeof(uint64_t), // uint64_t len_host, 
                (guint8 const *) &value_be // guint8 * value_ptr
            );
            
            // Write frame timestamp KLV_KEY_image_capture_time_stamp
            value_be = htobe64((uint64_t)frame_capture_PTS);
            klv_data_end_ptr = write_KLV_item(
                klv_data_end_ptr, // guint8 * klv_buffer,
                KLV_KEY_image_capture_time_stamp, // uint64_t key_host,
                sizeof(uint64_t), // uint64_t len_host, 
                (guint8 const *) &value_be // guint8 * value_ptr
            );
            

            // Write string "!frame meta end!"
            gchar const * frameMetaEndString = "!frame meta end!";
            klv_data_end_ptr = write_KLV_item(
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
	xiStopAcquisition(app->camState.cameraHandle);
	acquire = FALSE;
	xiCloseDevice(app->camState.cameraHandle);
	app->camState.cameraHandle = INVALID_HANDLE_VALUE;
	videoThread = 0;
	gdk_threads_leave();
	//} end cleanup	
	
	return 0;
}






GstBusSyncReply bus_sync_handler(GstBus* bus, GstMessage* message,  LirenaCamera* cam) //gpointer) 
{
	if(!gst_is_video_overlay_prepare_window_handle_message(message))
		return GST_BUS_PASS;
	gst_video_overlay_set_window_handle(GST_VIDEO_OVERLAY(GST_MESSAGE_SRC(message)), window_handle);
	gst_message_unref(message);
	return GST_BUS_DROP;
}


void video_widget_realize_cb(GtkWidget* widget,  LirenaCamera* cam) //gpointer) 
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





gboolean time_handler(LirenaCamStreamApp* app) 
//gboolean time_handler(LirenaCamStreamControlWindow *controlWindow) 
{
	int level = 0;

	if(acquire && app->camState.cameraHandle != INVALID_HANDLE_VALUE) {
		xiSetParamInt(app->camState.cameraHandle, XI_PRM_GPI_SELECTOR, 1);
		xiGetParamInt(app->camState.cameraHandle, XI_PRM_GPI_LEVEL, &level);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(app->localDisplay.controlWindow.gpi1), level);
		xiSetParamInt(app->camState.cameraHandle, XI_PRM_GPI_SELECTOR, 2);
		xiGetParamInt(app->camState.cameraHandle, XI_PRM_GPI_LEVEL, &level);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(app->localDisplay.controlWindow.gpi2), level);
		xiSetParamInt(app->camState.cameraHandle, XI_PRM_GPI_SELECTOR, 3);
		xiGetParamInt(app->camState.cameraHandle, XI_PRM_GPI_LEVEL, &level);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(app->localDisplay.controlWindow.gpi3), level);
		xiSetParamInt(app->camState.cameraHandle, XI_PRM_GPI_SELECTOR, 4);
		xiGetParamInt(app->camState.cameraHandle, XI_PRM_GPI_LEVEL, &level);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(app->localDisplay.controlWindow.gpi4), level);
	}

	gtk_toggle_button_set_inconsistent(
	  GTK_TOGGLE_BUTTON(app->localDisplay.controlWindow.run), 
	  gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(app->localDisplay.controlWindow.run)) != acquire);

	gtk_widget_set_sensitive(app->localDisplay.controlWindow.boxx, acquire);
	gtk_widget_set_sensitive(app->localDisplay.controlWindow.boxy, acquire);
	gtk_widget_set_sensitive(app->localDisplay.controlWindow.exp, acquire);
	gtk_widget_set_sensitive(app->localDisplay.controlWindow.gain, acquire);

	return TRUE;
}


gboolean update_x0(GtkAdjustment *adj, LirenaCamera* cam) //gpointer)
{
	roix0 = gtk_adjustment_get_value(adj);
	if(roicx + roix0 > maxcx){
		roix0 = maxcx - roicx;
		gtk_adjustment_set_value(adj, roix0);
	}
	xiSetParamInt(cam->cameraHandle, XI_PRM_OFFSET_X, roix0);
	return TRUE;
}

gboolean update_y0(GtkAdjustment *adj,  LirenaCamera* cam) //gpointer)
{
	roiy0 = gtk_adjustment_get_value(adj);
	if(roicy + roiy0 > maxcy){
		roiy0 = maxcy - roicy;
		gtk_adjustment_set_value(adj, roiy0);
	}
	xiSetParamInt(cam->cameraHandle, XI_PRM_OFFSET_Y, roiy0);
	return TRUE;
}

gboolean update_cx(GtkAdjustment *adj,  LirenaCamera* cam) //gpointer)
{
	roicx = gtk_adjustment_get_value(adj);
	if(roix0 + roicx > maxcx){
		roicx = maxcx - roix0;
		gtk_adjustment_set_value(adj, roicx);
	}
	xiSetParamInt(cam->cameraHandle, XI_PRM_WIDTH, roicx);
	return TRUE;
}


gboolean update_cy(GtkAdjustment *adj,  LirenaCamera* cam) //gpointer)
{
	roicy = gtk_adjustment_get_value(adj);
	if(roiy0 + roicy > maxcy) {
		roicy = maxcy - roiy0;
		gtk_adjustment_set_value(adj, roicy);
	}
	xiSetParamInt(cam->cameraHandle, XI_PRM_HEIGHT, roicy);
	return TRUE;
}


gboolean update_exposure(GtkAdjustment *adj,  LirenaCamera* cam) //gpointer)
{
	xiSetParamInt(cam->cameraHandle, XI_PRM_EXPOSURE, 1000*gtk_adjustment_get_value(adj));		
	return TRUE;
}


gboolean update_gain(GtkAdjustment *adj,  LirenaCamera* cam) //gpointer)
{
	xiSetParamFloat(cam->cameraHandle, XI_PRM_GAIN, gtk_adjustment_get_value(adj));
	return TRUE;
}


gboolean update_raw(GtkToggleButton *raw, LirenaCamStreamApp* appPtr)
{
	LirenaCamStreamControlWindow *controlWindow = & appPtr->localDisplay.controlWindow;
	HANDLE cameraHandle = appPtr->camState.cameraHandle;

	if(cameraHandle != INVALID_HANDLE_VALUE) {
		float mingain, maxgain;
		xiSetParamInt(cameraHandle, XI_PRM_IMAGE_DATA_FORMAT, gtk_toggle_button_get_active(raw) ? XI_RAW8 : XI_RGB32);
		xiGetParamFloat(cameraHandle, XI_PRM_GAIN XI_PRM_INFO_MIN, &mingain);
		xiGetParamFloat(cameraHandle, XI_PRM_GAIN XI_PRM_INFO_MAX, &maxgain);
		xiGetParamInt(cameraHandle, XI_PRM_WIDTH XI_PRM_INFO_MAX, &maxcx);
		xiGetParamInt(cameraHandle, XI_PRM_HEIGHT XI_PRM_INFO_MAX, &maxcy);
		roicx = maxcx;
		roicy = maxcy;
		roix0 = 0;
		roiy0 = 0;
		
		float midgain = (maxgain + mingain) * 0.5f;
		
		//gtk_adjustment_configure(gtk_range_get_adjustment(GTK_RANGE(controlWindow->gain)), mingain, mingain, maxgain, 0.1, 1, 0);
		gtk_adjustment_configure(gtk_range_get_adjustment(GTK_RANGE(controlWindow->gain)), midgain, mingain, maxgain, 0.1, 1, 0);
		gtk_adjustment_configure(gtk_spin_button_get_adjustment(GTK_SPIN_BUTTON(controlWindow->x0)), roix0, 0, maxcx-4, 2, 20, 0);
		gtk_adjustment_configure(gtk_spin_button_get_adjustment(GTK_SPIN_BUTTON(controlWindow->y0)), roiy0, 0, maxcy-2, 2, 20, 0);
		gtk_adjustment_configure(gtk_spin_button_get_adjustment(GTK_SPIN_BUTTON(controlWindow->cx)), roicx, 4, maxcx, 4, 20, 0);
		gtk_adjustment_configure(gtk_spin_button_get_adjustment(GTK_SPIN_BUTTON(controlWindow->cy)), roicy, 2, maxcy, 2, 20, 0);
		
		//xiSetParamFloat(cameraHandle, XI_PRM_GAIN, mingain);
		xiSetParamFloat(cameraHandle, XI_PRM_GAIN, midgain);
		xiSetParamInt(cameraHandle, XI_PRM_OFFSET_X, roix0);
		xiSetParamInt(cameraHandle, XI_PRM_OFFSET_Y, roiy0);
		xiSetParamInt(cameraHandle, XI_PRM_WIDTH, roicx);
		xiSetParamInt(cameraHandle, XI_PRM_HEIGHT, roicy);
		//exposure doesn't seem to be affected by format change
	}
	return TRUE;
}

gboolean update_show(GtkToggleButton *show,  LirenaCamera* cam) //gpointer) 
{
	render = gtk_toggle_button_get_active(show);
	return TRUE;
}


gboolean update_run(GtkToggleButton *run, LirenaCamStreamApp* appPtr)
{
	LirenaCamStreamControlWindow *controlWindow = & appPtr->localDisplay.controlWindow;
	//HANDLE cameraHandle = appPtr->camState.cameraHandle;

	gtk_toggle_button_set_inconsistent(run, false);
	acquire = gtk_toggle_button_get_active(run);
	if(acquire && appPtr->camState.cameraHandle == INVALID_HANDLE_VALUE) {
		DWORD nIndex = 0;
		char* env = getenv("CAM_INDEX");
		if(env) {
			nIndex = atoi(env);
		}
		DWORD tmp;
		xiGetNumberDevices(&tmp); //rescan available devices
		if(xiOpenDevice(nIndex, &appPtr->camState.cameraHandle) != XI_OK) {
			printf("Couldn't setup camera!\n");
			acquire = FALSE;
			return TRUE;
		}
		update_raw(GTK_TOGGLE_BUTTON(controlWindow->raw), appPtr);
		int isColor = 0;
		xiGetParamInt(appPtr->camState.cameraHandle, XI_PRM_IMAGE_IS_COLOR, &isColor);
		if(isColor)
			xiSetParamInt(appPtr->camState.cameraHandle, XI_PRM_AUTO_WB, 1);

        // set exposure from CLI arg
		xiSetParamInt(appPtr->camState.cameraHandle, XI_PRM_EXPOSURE, 1000 * appPtr->config.exposure_ms);
		gtk_adjustment_set_value(
		    gtk_range_get_adjustment(GTK_RANGE(controlWindow->exp)), 
		    appPtr->config.exposure_ms);
		
		if(pthread_create(&videoThread, NULL, videoDisplay,  (void*)appPtr))// NULL))
			exit(1);
	}
	gtk_widget_set_sensitive(controlWindow->boxx, acquire);
	gtk_widget_set_sensitive(controlWindow->boxy, acquire);
	gtk_widget_set_sensitive(controlWindow->exp, acquire);
	gtk_widget_set_sensitive(controlWindow->gain, acquire);
	return TRUE;
}


gboolean start_cb(LirenaCamStreamApp* appPtr)
//gboolean start_cb(LirenaCamStreamControlWindow* controlWindow) 
{
	//start acquisition
	update_run(GTK_TOGGLE_BUTTON(appPtr->localDisplay.controlWindow.run), appPtr); //controlWindow);
	return FALSE;
}



