

#include "LirenaStreamer.h"

#include "LirenaCaptureApp.h"
#include "LirenaKLVappsrc.h"


#include <gst/app/gstappsrc.h>

#include <sys/time.h>
#include <pthread.h>
#include <string.h>




LirenaStreamer::LirenaStreamer(LirenaConfig * configPtr)
	:
	configPtr(configPtr)
{
	gst_init(&configPtr->argc, &configPtr->argv);


	this->captureThread = 0;


	//UI-related stuff to follow:

	this->doAcquireFrames = TRUE;



	captureDevicePtr = LirenaCaptureDevice::createCaptureDevice(
		configPtr);

	//TODO outsource to device
	this->camParams.cameraHandle = INVALID_HANDLE_VALUE;
}

LirenaStreamer::~LirenaStreamer()
{
	if(captureThreadIsRunning)
	{
		terminateCaptureThread();
	}

	if(captureDevicePtr)
	{
		delete captureDevicePtr;
		captureDevicePtr = nullptr;
	}
	else
	{
		GST_WARNING("captureDevicePtr is already null"
					" on LirenaStreamer destructor");
	}
}


//TODO continue here
bool LirenaStreamer::setupCaptureDevice()
{
	g_assert(0&& "TODO implement");
	return false;
}
		
bool LirenaStreamer::setupGStreamerPipeline()
{
	g_assert(0&& "TODO implement");
	return false;
}

bool LirenaStreamer::launchCaptureThread(
	LirenaCaptureApp * appPtr_refactor_compat_TODO_delete
)
{
	//Adapted from C-style function:
	//	gboolean lirenaStreamer_startCaptureThread(
	//		LirenaCaptureApp *appPtr);

	g_assert(this->doAcquireFrames &&
		     "capture thread may only be started"
			 " if capturing is desired");
	
	int pthread_ret = pthread_create(
						& this->captureThread,
						NULL, 
						lirena_XimeaStreamer_captureThread_run, 
						(void *)appPtr_refactor_compat_TODO_delete);
	
	if (pthread_ret != 0)
	{
		//triple fail redundancy ;(
		g_assert(0 &&  "capture thread creation failed!");
		exit(1);
		return false;
	}
	

	return true;
}

bool LirenaStreamer::terminateCaptureThread()
{
	if(captureThreadIsRunning)
	{	
		g_assert(captureThread != 0 &&
        	"enforce logical consistency between "
			"thread running guard and thread ID variable");


		//TODO set acquire to false to make thread loop end
		doAcquireFrames=false;
		//wait for thread ending
		pthread_join(captureThread, NULL);

		//cleanup variables
		captureThread = 0;
		captureThreadIsRunning = false;

		return true;
	} 
	else
	{
		g_assert(captureThread == 0 && 
		         "enforce logical consistency between "
				 "thread running guard and thread ID variable");
				 
		return true;
	}
}



















gboolean lirena_XimeaStreamer_openCamera(
	LirenaStreamer* streamer
)
{

	streamer->doAcquireFrames = TRUE;

	// open cam required?
	if (streamer->doAcquireFrames && 
	    streamer->camParams.cameraHandle == INVALID_HANDLE_VALUE)
	{

		DWORD nIndex = 0;
		char *env = getenv("CAM_INDEX");
		if (env)
		{
			nIndex = atoi(env);
		}
		DWORD tmp;
		xiGetNumberDevices(&tmp); //rescan available devices

		if (xiOpenDevice(nIndex, & streamer->camParams.cameraHandle) != XI_OK)
		{
			printf("Couldn't setup camera!\n");
			streamer->doAcquireFrames = FALSE;
			return FALSE; //TRUE in original ximea code 0o ...
		}

	}

	return TRUE;
}



gboolean lirena_XimeaStreamer_setupCamParams(
	LirenaXimeaStreamer_CameraParams* camParams,
	LirenaConfig const * config
)
{
	HANDLE cameraHandle = camParams->cameraHandle;

	if (cameraHandle != INVALID_HANDLE_VALUE)
	{
		float mingain, maxgain;

		//TODO make sure this also is correct for new cam!
		xiSetParamInt(cameraHandle, XI_PRM_IMAGE_DATA_FORMAT, XI_RAW8);
			//gtk_toggle_button_get_active(raw) ? XI_RAW8 : XI_RGB32);
		
		//get gain stuff
		xiGetParamFloat(cameraHandle, XI_PRM_GAIN XI_PRM_INFO_MIN, &mingain);
		xiGetParamFloat(cameraHandle, XI_PRM_GAIN XI_PRM_INFO_MAX, &maxgain);

		//get cropping range
		xiGetParamInt(cameraHandle, XI_PRM_WIDTH XI_PRM_INFO_MAX, &camParams->maxcx);
		xiGetParamInt(cameraHandle, XI_PRM_HEIGHT XI_PRM_INFO_MAX, &camParams->maxcy);
		
		//init cropping to "no cropping"
		camParams->roicx = camParams->maxcx;
		camParams->roicy = camParams->maxcy;
		camParams->roix0 = 0;
		camParams->roiy0 = 0;

		//hacky heuristic: set "medium gain"
		// TODO make this CLI-configurable
		float midgain = (maxgain + mingain) * 0.5f;
		xiSetParamFloat(cameraHandle, XI_PRM_GAIN, midgain);

		//make sure the cam does no cropping
		xiSetParamInt(cameraHandle, XI_PRM_OFFSET_X, camParams->roix0);
		xiSetParamInt(cameraHandle, XI_PRM_OFFSET_Y, camParams->roiy0);
		xiSetParamInt(cameraHandle, XI_PRM_WIDTH,    camParams->roicx);
		xiSetParamInt(cameraHandle, XI_PRM_HEIGHT, camParams->roicy);
		

		// white balance param..unused by us, as we debayer ourselves
		int isColor = 0;
		xiGetParamInt(cameraHandle, XI_PRM_IMAGE_IS_COLOR, &isColor);
		if (isColor)
		{
			xiSetParamInt(cameraHandle, XI_PRM_AUTO_WB, 1);
		}

		// set exposure in microseconds from CLI arg
		xiSetParamInt(cameraHandle, 
			XI_PRM_EXPOSURE, 
			1000 * config->ximeaparams.exposure_ms);
	}

	return TRUE;
}
 







//return string must be g_freed'd
gchar* constructGstreamerPipelineString(LirenaConfig const * config)
{
	// ten thousand chars are hopefully sufficient...
	#define LIRENA_MAX_GSTREAMER_PIPELINE_STRING_LENGTH 10000
	#define LIRENA_MAX_GSTREAMER_PIPELINE_SNIPPET_STRING_LENGTH 1000

	gchar*  gstreamerPipelineString = (gchar*) g_malloc0(
		LIRENA_MAX_GSTREAMER_PIPELINE_STRING_LENGTH);
	
	gchar gstreamerVideoSourceString[
		LIRENA_MAX_GSTREAMER_PIPELINE_SNIPPET_STRING_LENGTH];
	gchar gstreamerKLVsourceString[
		LIRENA_MAX_GSTREAMER_PIPELINE_SNIPPET_STRING_LENGTH];
	gchar gstreamerPreProcessingString[
		LIRENA_MAX_GSTREAMER_PIPELINE_SNIPPET_STRING_LENGTH];
	gchar gstreamerForkString_raw_to_show_and_enc[
		LIRENA_MAX_GSTREAMER_PIPELINE_SNIPPET_STRING_LENGTH];
	gchar gstreamerForkString_enc_to_disk_and_UDP[
		LIRENA_MAX_GSTREAMER_PIPELINE_SNIPPET_STRING_LENGTH];
	gchar gStreamerNetworkTransmissionSnippet    [
		LIRENA_MAX_GSTREAMER_PIPELINE_SNIPPET_STRING_LENGTH];

	// init snippets to empty string:
	g_snprintf(gstreamerVideoSourceString,
			   (gulong)LIRENA_MAX_GSTREAMER_PIPELINE_SNIPPET_STRING_LENGTH,
			   "%s", "");
	g_snprintf(gstreamerKLVsourceString,
			   (gulong)LIRENA_MAX_GSTREAMER_PIPELINE_SNIPPET_STRING_LENGTH,
			   "%s", "");
	g_snprintf(gstreamerPreProcessingString,
			   (gulong)LIRENA_MAX_GSTREAMER_PIPELINE_SNIPPET_STRING_LENGTH,
			   "%s", "");
	g_snprintf(gstreamerForkString_raw_to_show_and_enc,
			   (gulong)LIRENA_MAX_GSTREAMER_PIPELINE_SNIPPET_STRING_LENGTH,
			   "%s", "");
	g_snprintf(gstreamerForkString_enc_to_disk_and_UDP,
			   (gulong)LIRENA_MAX_GSTREAMER_PIPELINE_SNIPPET_STRING_LENGTH,
			   "%s", "");
	g_snprintf(gStreamerNetworkTransmissionSnippet,
			   (gulong)LIRENA_MAX_GSTREAMER_PIPELINE_SNIPPET_STRING_LENGTH,
			   "%s", "");


	// create video source snippet (appsrc vs videotestsrc):
	if(	
		(config->captureDeviceType == LIRENA_CAPTURE_DEVICE_TYPE_XimeaCamera)
		|| //TODO change conditions like this to a function deviceTypeImpliesAppsrc(...)
		(config->captureDeviceType == LIRENA_CAPTURE_DEVICE_TYPE_MagewellEcoCapture)
	)
	{
			g_snprintf(gstreamerVideoSourceString,
				(gulong)LIRENA_MAX_GSTREAMER_PIPELINE_SNIPPET_STRING_LENGTH,
			   // Video appsrc, encode h264, to muxer:
			   // TODO outsource to snippet to make replaceable by videotestsrc
			   " appsrc "
			   "     format=GST_FORMAT_TIME "
			   "     is-live=TRUE "
			   "     name=captureAppSrc ! "
			);
	}
	else if (config->captureDeviceType == LIRENA_CAPTURE_DEVICE_TYPE_videotestsrc)
	{
			g_snprintf(gstreamerVideoSourceString,
				(gulong)LIRENA_MAX_GSTREAMER_PIPELINE_SNIPPET_STRING_LENGTH,
			    // videotestsrc, hopefully helpful for debugging...
				// TODO keep in mind potential problems due to
				// 		differing resolution and framerate
				//   --> add caps filter matching desired framerate
				// TODO keep in mind potential problems due to
				//      do-timestamp property for videotestsrc
				// TODO catch all "captureAppSrc" occurences and subsequent actions
			    " videotestsrc "
			    "     pattern=ball "
			    // "     format=GST_FORMAT_TIME " <-- videotestsrc has no format property
				"     do-timestamp=TRUE "      // <-- hopefully this will do instead
			    "     is-live=TRUE "
				"     name=myvideotestsrc ! "
			);
	}
	else
	{
		g_assert(0 && "capture device type not supported yet");
	}


	// Create KLV appsrc snippet:
	if(config->injectKLVmeta)
	{
		g_snprintf(gstreamerKLVsourceString,
			(gulong)LIRENA_MAX_GSTREAMER_PIPELINE_SNIPPET_STRING_LENGTH,
			// KLV appsrc:
			// TODO catch all "klvSrc" occurences and subsequent actions
			" appsrc "
			"     format=GST_FORMAT_TIME "
			"     is-live=TRUE "
			"     name=klvSrc ! "
			"   meta/x-klv, parsed=true ! "
			" mp2ts_muxer. "
		);
	}
	else
	{
		//keep empty sting as initialized
	}


	// create preprocessing snippet:
	if (config->captureDeviceType == LIRENA_CAPTURE_DEVICE_TYPE_XimeaCamera)
	{
		if(config->ximeaparams.useCudaDemosaic)
		{
			g_assert(0 && "cuda demosaicing not implemented yet");
			g_snprintf(gstreamerPreProcessingString,
				(gulong)LIRENA_MAX_GSTREAMER_PIPELINE_SNIPPET_STRING_LENGTH,
				"TODO"
			);
		}
		else
		{
			//software demosaic in gstreamer:
			g_snprintf(gstreamerPreProcessingString,
				(gulong)LIRENA_MAX_GSTREAMER_PIPELINE_SNIPPET_STRING_LENGTH,
			    "   video/x-bayer ! "
			    " bayer2rgb ! "
			    "   video/x-raw, format=(string)BGRx ! "
			    " videoconvert ! "
			);

		}
	} else
	{
		// MagewellEco or videotestsrc:
		//just a plain old videoconvert for now
		//software demosaic in gstreamer:
		g_snprintf(gstreamerPreProcessingString,
			(gulong)LIRENA_MAX_GSTREAMER_PIPELINE_SNIPPET_STRING_LENGTH,
		    "   video/x-raw! "
		    " videoconvert ! "
		);
	}



	if (config->doLocalDisplay)
	{
		g_snprintf(gstreamerForkString_raw_to_show_and_enc,
				   (gulong)LIRENA_MAX_GSTREAMER_PIPELINE_SNIPPET_STRING_LENGTH,
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
				   (gulong)LIRENA_MAX_GSTREAMER_PIPELINE_SNIPPET_STRING_LENGTH,
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
						  "THIS IS EXPERIMENTAL AND MAY NOT WORK!");

		g_snprintf(gStreamerNetworkTransmissionSnippet,
				   (gulong)LIRENA_MAX_GSTREAMER_PIPELINE_SNIPPET_STRING_LENGTH,
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
				   (gulong)LIRENA_MAX_GSTREAMER_PIPELINE_SNIPPET_STRING_LENGTH,
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
	g_snprintf(
		gstreamerPipelineString,
		(gulong) LIRENA_MAX_GSTREAMER_PIPELINE_STRING_LENGTH,
		  	   // Video appsrc or videotestsrc:
			   " %s " // gstreamerVideoSourceString
			   ""
			   //{ 
			   // Preprocessing: either of:
			   // 1.  gstreamer software-debayering (ximea stuff that worked before returning the test cam), 
			   // 2.  special filter caps for non-gstreamer cuda-debayered cuda-memory
			   //       and then xvvidconv or so instead of videoconvert ... TODO as soon as cams are here!
			   // 3.  just videoconvert (magewell stuff) TODO try ASAP
			   " %s " // gstreamerPreProcessingString
			   //}
               ""
			   // optional fork to local display:
			   " %s " //gstreamerForkString_raw_to_show_and_enc
			   " queue ! "
			   " nvvidconv ! "
			   " nvv4l2h264enc maxperf-enable=1 bitrate=8000000 ! "
			   " h264parse  disable-passthrough=true ! " //config-interval=-1  <--may be required in TCP?...
			   ""
			   // Mpeg2TS muxer:
			   // TODO check: maybe alignment 7 for UDP!? as gstinspect says!
			   " mpegtsmux name=mp2ts_muxer alignment=0 " 
			   ""
			   // KLV appsrc:
			   " %s " // gstreamerKLVsourceString
			   ""
			   " mp2ts_muxer. "

			   // prepare muxed stream for network transmission:
			   " mp2ts_muxer. ! "
			   "   tsparse ! "
			   "   queue max-size-time=30000000000 max-size-bytes=0 max-size-buffers=0 ! "
			   // optional fork to dump-to-disk;
			   "   %s " // gstreamerForkString_enc_to_disk_and_UDP 
			   "   rtpmp2tpay ! "
			   ""
			   // send over network via UDP or optionally TCP
			   " %s " // gStreamerNetworkTransmissionSnippet
			   ,
			   gstreamerVideoSourceString,
			   gstreamerPreProcessingString,
			   gstreamerForkString_raw_to_show_and_enc,
			   gstreamerKLVsourceString,
			   gstreamerForkString_enc_to_disk_and_UDP,
			   gStreamerNetworkTransmissionSnippet
			);

	GST_INFO("HERE MY PIPELINE: %s", gstreamerPipelineString);
	sleep(1);

	return gstreamerPipelineString;
}









// TODO remove streaming logic from this ui function!
//void* videoDisplay(void*)
void * lirena_XimeaStreamer_captureThread_run(void *appVoidPtr)
{
	LirenaCaptureApp *appPtr = (LirenaCaptureApp *)appVoidPtr;
	appPtr->streamerPtr->captureThreadIsRunning = true;

	//TODO rename
	bool haveGUI = appPtr->configPtr->doLocalDisplay;

	
	GstElement *pipeline = 0;

	
	GstFlowReturn ret;
	
	// for control of local image display:
	GstBus *bus = 0;
	

	int max_width = 1000;
	int max_height = 1000;
	int prev_width = -1;
	int prev_height = -1;


	gchar statistics_string[256];
	unsigned long frames = 0;
	unsigned long prevframes = 0;
	unsigned long lostframes = 0;
	long lastframe = -1;

	unsigned long curtime, prevtime;
	unsigned long firsttime = getcurus();
	//GstClockTime gst_defaultClock_prevTime = 0;




	XI_IMG_FORMAT prev_format = XI_RAW8;
	XI_IMG image;
	image.size = sizeof(XI_IMG);
	image.bp = NULL;
	image.bp_size = 0;




	if (xiStartAcquisition(appPtr->streamerPtr->camParams.cameraHandle) != XI_OK)
	{
		return lirena_XimeaStreamer_captureThread_terminate(appPtr);
	}



	if(haveGUI)
	{
		gdk_threads_enter();

		appPtr->uiPtr->widgets.videoWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);

		gtk_window_set_title(
			GTK_WINDOW(appPtr->uiPtr->widgets.videoWindow), "LirenaCapture local display");
		gtk_widget_set_double_buffered(
			appPtr->uiPtr->widgets.videoWindow, FALSE);

		g_signal_connect(appPtr->uiPtr->widgets.videoWindow,
						"realize", 
						G_CALLBACK(lirenaCaptureXimeaGUI_cb_grabXhandleForVidWindow), 
						appPtr->uiPtr);

		g_signal_connect(appPtr->uiPtr->widgets.videoWindow,
						"delete-event", G_CALLBACK(LirenaCaptureXimeaGUI_cb_closeWindow),
						appPtr->streamerPtr);

		gtk_widget_show_all(appPtr->uiPtr->widgets.videoWindow);
		//TODO find out if nececcary, and if yes in what order wrt. gtk_widget_show_all
		gtk_widget_realize(appPtr->uiPtr->widgets.videoWindow);

		appPtr->uiPtr->widgets.screen = gdk_screen_get_default();

		max_width = 0.8 * gdk_screen_get_width(appPtr->uiPtr->widgets.screen);
		max_height = 0.8 * gdk_screen_get_width(appPtr->uiPtr->widgets.screen);

		gdk_threads_leave();
	}



	gchar * gstreamerPipelineString = 
		constructGstreamerPipelineString(appPtr->configPtr);

	pipeline = gst_parse_launch(
		gstreamerPipelineString,
		NULL);

	g_free(gstreamerPipelineString);

	if (!pipeline)
	{
		return lirena_XimeaStreamer_captureThread_terminate(appPtr);
	}



	if(haveGUI)
	{
		bus = gst_pipeline_get_bus(GST_PIPELINE(pipeline));
		gst_bus_set_sync_handler(bus, 
								(GstBusSyncHandler)lirenaCaptureXimeaGUI_cb_handleBusSyncEvent,
								appPtr->uiPtr,
								NULL);
		gst_object_unref(bus);
	}



	//{ "main loop"
	firsttime = getcurus();
	prevtime = getcurus();

	//gst_defaultClock_prevTime = GST_ELEMENT(pipeline)->base_time;

	while (appPtr->streamerPtr->doAcquireFrames)
	{

		// grab image
		if (xiGetImage(appPtr->streamerPtr->camParams.cameraHandle, 5000, &image) != XI_OK)
		{
			break;
		}

		//{ record timestamp of frame capture
		// The following works: Presentation time stamp (PTS)
		//  from SYSTEM time, NOT GstClock!
		// TODO setup a working GstClock (PTP)
		// usage: 	GST_BUFFER_PTS(myBuffer) = frame_capture_PTS;
		curtime = getcurus();
		guint64 frame_capture_PTS = (guint64)(curtime - firsttime) * GST_USECOND;
		//}

		//have appsrc?
		if(appPtr->streamerPtr->captureDevicePtr->typeImpliesGstAppSrc())
		{
			GstElement *appsrc_video = 0;

			//n.b. pushed buffers don't need to be unrefed
			GstBuffer *video_frame_GstBuffer = 0;

			
			appsrc_video = gst_bin_get_by_name(GST_BIN(pipeline), "captureAppSrc");



			unsigned long raw_image_buffer_size =
				image.width * image.height * (image.frm == XI_RAW8 ? 1 : 4);

			//{ alloc mem for buffer and fill buffer, depending on padding
			if (!image.padding_x)
			{
				video_frame_GstBuffer = gst_buffer_new();

				gst_buffer_insert_memory(
					video_frame_GstBuffer,
					-1,
					gst_memory_new_wrapped(
						GST_MEMORY_FLAG_READONLY,
						(guint8 *)image.bp,
						raw_image_buffer_size,
						0,
						raw_image_buffer_size,
						0,
						0));
			}
			else
			{
				printf("Warning: image is padded, mem copy is row-wise!\n");
				video_frame_GstBuffer = gst_buffer_new_allocate(0, raw_image_buffer_size, 0);
				if (!video_frame_GstBuffer)
					break;

				for (guint i = 0; i < image.height; i++)
				{
					gst_buffer_fill(
						video_frame_GstBuffer,
						i * image.width * (image.frm == XI_RAW8 ? 1 : 4),
						(guint8 *)image.bp +
							i * (image.width * (image.frm == XI_RAW8 ? 1 : 4) +
								 image.padding_x),
						image.width * (image.frm == XI_RAW8 ? 1 : 4));
				}
			}
			//} end alloc mem and fill video gst buffer

			
			// init/update appsrc caps and scale-element caps ?
			if (
				(guint)prev_width != image.width ||
				(guint)prev_height != image.height ||
				prev_format != image.frm
			)
			{
				
				//{ init/update appsrc caps depending on raw vs color
				// to adapt for debayer stuff:

				GstCaps *appsrc_video_caps = 0;
				
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

				if (image.frm == XI_RAW8)
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
				else if (image.frm == XI_RGB32)
				{
					printf("DEBUG: RGB32\n");
					appsrc_video_caps = gst_caps_new_simple(
						"video/x-raw",
						"format", G_TYPE_STRING, "BGRx",
						"bpp", G_TYPE_INT, 32,
						"depth", G_TYPE_INT, 24,
						"endianness", G_TYPE_INT, G_BIG_ENDIAN,
						"red_mask", G_TYPE_INT, 0x0000ff00,
						"green_mask", G_TYPE_INT, 0x00ff0000,
						"blue_mask", G_TYPE_INT, 0xff000000,
						"framerate", GST_TYPE_FRACTION, 0, 1,
						"width", G_TYPE_INT, image.width,
						"height", G_TYPE_INT, image.height,
						NULL);
				}
				else
				{
					printf("DEBUG: unknown pixel format!\n");
					g_assert(0 &&  "unknown pixel format!");
					break;
				}


				gst_element_set_state(pipeline, GST_STATE_PAUSED);
				g_assert(appsrc_video_caps); // better safe than sorry
				gst_app_src_set_caps(GST_APP_SRC(appsrc_video), appsrc_video_caps);
				gst_element_set_state(pipeline, GST_STATE_PLAYING);

				if (appsrc_video_caps)
				{
					gst_caps_unref(appsrc_video_caps);
				}

				// update so that no refresh of caps needs to happen again
				// unless one messes with the cam res or format
				prev_width = image.width;
				prev_height = image.height;
				prev_format = image.frm;

				//} end init/update appsrc caps depending on raw vs color



				//{ init/update scale-element caps for local displaying?
				if(haveGUI)
				{
					GstElement *scale_element = 0;
					GstCaps *size_caps = 0;
					scale_element = gst_bin_get_by_name(GST_BIN(pipeline), "scale_element");

					int width = image.width;
					int height = image.height;

					while (width > max_width || height > max_height)
					{
						width /= 2;
						height /= 2;
					}

					if (size_caps)
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

					gst_object_unref(scale_element);
					
					//free ref from local scope:
					if (size_caps)
					{
						gst_caps_unref(size_caps);
					}


					gdk_threads_enter();
					gtk_window_resize(
						GTK_WINDOW(appPtr->uiPtr->widgets.videoWindow), 
						width, height);
					gdk_threads_leave();
				}
				//} end  init/update scale-element caps for local displaying

			}


			//{ Clock / timestamp-calc stuff:
			//  https://stackoverflow.com/questions/34294755/mux-klv-data-with-h264-by-mpegtsmux
			//  TODO try with appsrc + PTP clock instead of pipeline

			GstClock *clock = nullptr;
			GstClockTime abs_time, base_time;

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

			GST_BUFFER_PTS(video_frame_GstBuffer) = myPTS_timestamp;
			// setting duration always causes severe problems 0_o; TODO investigate reason
			//GST_BUFFER_DURATION (video_frame_GstBuffer) = myDuration;

			//} end clock stuff


			// push buffer into gstreamer pipeline
			ret = gst_app_src_push_buffer(
				GST_APP_SRC(appsrc_video), 
				video_frame_GstBuffer);

			gst_object_unref(appsrc_video);

			if (ret != GST_FLOW_OK)
			{	
				g_assert(0 && "pushing video buffer to appsrc failed!");
				break;
			}



			//-----------------------------------------------------------------
			//{ KLV buffer injection
			// TODO outsource after timing encapsulation
			if(appPtr->configPtr->injectKLVmeta)
			{
				GstElement * appsrc_klv = gst_bin_get_by_name(GST_BIN(pipeline), "klvSrc");

				// hundred thousand bytes
				#define KLV_BUFFER_MAX_SIZE 100000
				guint8 klv_data[KLV_BUFFER_MAX_SIZE];
				memset(klv_data, 0, KLV_BUFFER_MAX_SIZE);

				guint8 *klv_data_end_ptr = &klv_data[0];

				// Write string  "0123456789ABCDEF"
				gchar const *test16byteString = "0123456789ABCDEF";
				klv_data_end_ptr = write_KLV_item(
					klv_data_end_ptr,				 // guint8 * klv_buffer,
					KLV_KEY_string,					 // uint64_t key_host,
					strlen(test16byteString),		 // uint64_t len_host,
					(guint8 const *)test16byteString // guint8 * value_ptr
				);

				// Write frame count KLV_KEY_image_frame_number
				uint64_t value_be = htobe64((uint64_t)frames);
				klv_data_end_ptr = write_KLV_item(
					klv_data_end_ptr,					 // guint8 * klv_buffer,
					KLV_KEY_image_captured_frame_number, // uint64_t key_host,
					sizeof(uint64_t),					 // uint64_t len_host,
					(guint8 const *)&value_be			 // guint8 * value_ptr
				);

				// Write frame timestamp KLV_KEY_image_capture_time_stamp
				value_be = htobe64((uint64_t)frame_capture_PTS);
				klv_data_end_ptr = write_KLV_item(
					klv_data_end_ptr,				  // guint8 * klv_buffer,
					KLV_KEY_image_capture_time_stamp, // uint64_t key_host,
					sizeof(uint64_t),				  // uint64_t len_host,
					(guint8 const *)&value_be		  // guint8 * value_ptr
				);

				// Write string "!frame meta end!"
				gchar const *frameMetaEndString = "!frame meta end!";
				klv_data_end_ptr = write_KLV_item(
					klv_data_end_ptr,				   // guint8 * klv_buffer,
					KLV_KEY_string,					   // uint64_t key_host,
					strlen(frameMetaEndString),		   // uint64_t len_host,
					(guint8 const *)frameMetaEndString // guint8 * value_ptr
				);

				guint klvBufferSize = klv_data_end_ptr - (&klv_data[0]);

				// create empty GstBuffer
				GstBuffer *klv_frame_GstBuffer = gst_buffer_new();

				// alloc memory for buffer
				GstMemory *gstMem = gst_allocator_alloc(NULL, klvBufferSize, NULL);

				// add the buffer
				gst_buffer_append_memory(klv_frame_GstBuffer, gstMem);

				// get WRITE access to the memory and fill with our KLV data
				GstMapInfo gstInfo;
				gst_buffer_map(klv_frame_GstBuffer, &gstInfo, GST_MAP_WRITE);

				//do the writing
				memcpy(gstInfo.data, klv_data, gstInfo.size);

				gst_buffer_unmap(klv_frame_GstBuffer, &gstInfo);

				// Timestamp stuff onto KLV buffer itself to get associated
				// with video buffer:
				//GST_BUFFER_TIMESTAMP(klv_frame_GstBuffer) = GST_CLOCK_TIME_NONE;
				//GST_BUFFER_PTS(klv_frame_GstBuffer) = frame_capture_PTS;

				GST_BUFFER_PTS(klv_frame_GstBuffer) = myPTS_timestamp;
				//GST_BUFFER_DURATION (klv_frame_GstBuffer) = myDuration;

				// This function takes ownership of the buffer. so no unref!
				ret = gst_app_src_push_buffer(
					GST_APP_SRC(appsrc_klv),
					klv_frame_GstBuffer);

				if (ret != GST_FLOW_OK)
				{	
					g_assert(0 && "pushing KLV buffer to appsrc failed!");
					break;
				}

				// free scope-local reference to KLV appsrc
				gst_object_unref(appsrc_klv);
			} 
			//} end KLV buffer injection
			// ----------------------------------------------------------------
		}

		
		
		
		//{  statistics bookkeeping:
		frames++;
		if (image.nframe > lastframe)
		{
			lostframes += image.nframe - (lastframe + 1);
		}
		lastframe = image.nframe;
		// n.b. curtime is updated above already, after frame aquisition
		//curtime = getcurus();


		// update presentation of statistics each second:
		if (curtime - prevtime > 1000000)
		{
			snprintf(
				statistics_string,
				256,
				"Acquisition [ captured: %lu, skipped: %lu, fps: %.2f ]",
				frames,
				lostframes,
				1000000.0 * (frames - prevframes) / (curtime - prevtime));

			if(haveGUI)
			{
				gtk_window_set_title(
					GTK_WINDOW(appPtr->uiPtr->widgets.videoWindow),
					statistics_string);
			}
			else
			{
				GST_INFO("%s",statistics_string);
			}

			prevframes = frames;
			prevtime = curtime;
		}
		//} end statistics bookkeeping
	}
	//} end "main loop"



	//{ cleanup

	if(appPtr->streamerPtr->captureDevicePtr->typeImpliesGstAppSrc())
	{
		GstElement * appsrc_video = gst_bin_get_by_name(GST_BIN(pipeline), "captureAppSrc");
		gst_app_src_end_of_stream(GST_APP_SRC(appsrc_video));
		gst_object_unref(appsrc_video);
	}


	if(appPtr->configPtr->injectKLVmeta)
	{
		GstElement * appsrc_klv = gst_bin_get_by_name(GST_BIN(pipeline), "klvSrc");
		gst_app_src_end_of_stream(GST_APP_SRC(appsrc_klv));
		gst_object_unref(appsrc_klv);
	}
	

	gst_element_set_state(pipeline, GST_STATE_NULL);
	gst_object_unref(GST_OBJECT(pipeline));

	//exit thred:
	return lirena_XimeaStreamer_captureThread_terminate(appPtr);

	//} end cleanup
}





void * lirena_XimeaStreamer_captureThread_terminate(LirenaCaptureApp *appPtr)
{
	bool haveGUI = appPtr->configPtr->doLocalDisplay;

	if(haveGUI)
	{
		gdk_threads_enter();

		// don't bother with window destruction, just shut down GTK
		// the dirty way
		//gtk_widget_destroy(appPtr->uiPtr->widgets.videoWindow);
		
		gtk_main_quit();
		gdk_threads_leave();
	}
	else
	{
		//g_main_loop_quit(appPtr->pureMainLoop);
		appPtr->uiPtr->shutdownUI();
	}

	xiStopAcquisition(appPtr->streamerPtr->camParams.cameraHandle);
	appPtr->streamerPtr->doAcquireFrames = FALSE;
	xiCloseDevice(appPtr->streamerPtr->camParams.cameraHandle);
	appPtr->streamerPtr->camParams.cameraHandle = INVALID_HANDLE_VALUE;


    appPtr->streamerPtr->captureThreadIsRunning = false;

	//DON't mess with the thread ID from within the thread!
	//appPtr->streamerPtr->captureThread = 0;

	return NULL;
}
