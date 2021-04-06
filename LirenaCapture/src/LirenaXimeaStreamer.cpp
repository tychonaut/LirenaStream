

#include "LirenaXimeaStreamer.h"

#include "LirenaCaptureApp.h"
#include "LirenaKLVappsrc.h"


#include <gst/app/gstappsrc.h>

#include <sys/time.h>
#include <pthread.h>
#include <string.h>























// TODO remove streaming logic from this gui function!
//void* videoDisplay(void*)
void *lirena_XimeaStreamer_captureThread_run(void *appVoidPtr)
{
	LirenaCaptureApp *app = (LirenaCaptureApp *)appVoidPtr;

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
	GstClockTime gst_defaultClock_prevTime = 0;

	XI_IMG_FORMAT prev_format = XI_RAW8;
	XI_IMG image;
	image.size = sizeof(XI_IMG);
	image.bp = NULL;
	image.bp_size = 0;

	if (xiStartAcquisition(app->camState.cameraHandle) != XI_OK)
	{
		return lirena_XimeaStreamer_captureThread_terminate(app);
	}

	gdk_threads_enter();

	//GdkScreen *screen;

	app->localDisplayCtrl.widgets.videoWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);

	gtk_window_set_title(
		GTK_WINDOW(app->localDisplayCtrl.widgets.videoWindow), "streamViewer");
	gtk_widget_set_double_buffered(
		app->localDisplayCtrl.widgets.videoWindow, FALSE);

	g_signal_connect(app->localDisplayCtrl.widgets.videoWindow,
					 "realize", 
					 G_CALLBACK(video_widget_realize_cb), 
					 &app->localDisplayCtrl);

	// Messy hack to handle "shutdown app if CONTROL window is closed,
	// but to sth ales if RENDER window is closed".
	// The original logic didn't even work in the first place...
	close_cb_params *params = (close_cb_params *)g_malloc0(sizeof(close_cb_params));
	params->doShutDownApp = false; // do NOT shutdown on closing of control window!
	params->captureThreadPtr = &app->captureThread;
	params->camPtr= &app->camState;
	g_signal_connect(app->localDisplayCtrl.widgets.videoWindow,
					 "delete-event", G_CALLBACK(close_cb), params);

	gtk_widget_show_all(app->localDisplayCtrl.widgets.videoWindow);
	gtk_widget_realize(app->localDisplayCtrl.widgets.videoWindow);

	app->localDisplayCtrl.widgets.screen = gdk_screen_get_default();

	max_width = 0.8 * gdk_screen_get_width(app->localDisplayCtrl.widgets.screen);
	max_height = 0.8 * gdk_screen_get_width(app->localDisplayCtrl.widgets.screen);

	gdk_threads_leave();

// ten thousand chars are hopefully sufficient...
#define MAX_GSTREAMER_PIPELINE_STRING_LENGTH 10000
#define MAX_GSTREAMER_PIPELINE_SNIPPET_STRING_LENGTH 1000

	gchar gstreamerPipelineString[MAX_GSTREAMER_PIPELINE_STRING_LENGTH];
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

	if (app->config.doLocalDisplay)
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

	if (app->config.outputFile)
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
				   app->config.outputFile);
	}

	if (app->config.useTCP)
	{
		//TCP
		GST_WARNING("%s", "Using TCP instead of UDP: "
						  "This is experimental an my not work!");

		g_snprintf(gStreamerNetworkTransmissionSnippet,
				   (gulong)MAX_GSTREAMER_PIPELINE_SNIPPET_STRING_LENGTH,
				   " rtpstreampay ! " // wrap RTP stuff another time for TCP ... (?)
				   " tcpserversink "
				   "   host=%s "
				   "   port=%s "
				   "   sync-method=burst "
				   //"  sync-method=next-keyframe // works, but only for single-output recedeiver pipelines
				   //"   sync-method=latest " //  works, but also no batter than next-keyframe
				   ,
				   app->config.IP,
				   app->config.port);
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
				   app->config.IP,
				   app->config.port);
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
			   " mpegtsmux name=mp2ts_muxer alignment=0 "
			   //"  mpegtsmux."
			   ""
			   // Video appsrc, encode h264, to muxer:
			   " appsrc format=GST_FORMAT_TIME is-live=TRUE name=streamViewer ! "
			   ""
			   //{ software debayering by GStreamer itself
			   //: TODO outsource to Cuda&OpenCV
			   "   video/x-bayer ! "
			   " bayer2rgb ! "
			   "   video/x-raw, format=(string)BGRx ! "
			   " videoconvert ! "
			   //}

			   " %s " // optional fork to local display
			   " queue ! "
			   " nvvidconv ! "
			   " nvv4l2h264enc maxperf-enable=1 bitrate=8000000 ! "
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

	GST_INFO("HERE MY PIPELINE: %s", gstreamerPipelineString);
	sleep(1);

	pipeline = gst_parse_launch(
		gstreamerPipelineString,
		NULL);

	if (!pipeline)
	{
		return lirena_XimeaStreamer_captureThread_terminate(app);
	}

	bus = gst_pipeline_get_bus(GST_PIPELINE(pipeline));
	gst_bus_set_sync_handler(bus, 
							(GstBusSyncHandler)bus_sync_handler,
							 &app->localDisplayCtrl, //pipeline,
							 NULL);
	gst_object_unref(bus);

	appsrc_video = gst_bin_get_by_name(GST_BIN(pipeline), "streamViewer");
	appsrc_klv = gst_bin_get_by_name(GST_BIN(pipeline), "klvSrc");

	scale_element = gst_bin_get_by_name(GST_BIN(pipeline), "scale_element");

	//{ "main loop"
	firsttime = getcurus();
	prevtime = getcurus();

	gst_defaultClock_prevTime = GST_ELEMENT(pipeline)->base_time;

	while (app->camState.acquire)
	{

		// grab image
		if (xiGetImage(app->camState.cameraHandle, 5000, &image) != XI_OK)
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

		if (app->camState.doRender)
		{
			unsigned long raw_image_buffer_size =
				image.width * image.height * (image.frm == XI_RAW8 ? 1 : 4);

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

			if ((guint)prev_width != image.width ||
				(guint)prev_height != image.height ||
				prev_format != image.frm)
			{
				if (appsrc_video_caps)
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

				gdk_threads_enter();
				gtk_window_resize(GTK_WINDOW(app->localDisplayCtrl.widgets.videoWindow), width, height);
				gdk_threads_leave();
			}

			//Clock stuff:
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

			//just test
			guint64 myDuration = // abs_time - gst_defaultClock_prevTime;
				gst_util_uint64_scale_int(
					1, app->config.exposure_ms * GST_MSECOND, 1);
			//update for next frame
			gst_defaultClock_prevTime = abs_time;

			GST_BUFFER_PTS(video_frame_GstBuffer) = myPTS_timestamp;
			//GST_BUFFER_DURATION (video_frame_GstBuffer) = myDuration;

			// push buffer into gstreamer pipeline
			ret = gst_app_src_push_buffer(GST_APP_SRC(appsrc_video), video_frame_GstBuffer);

			if (ret != GST_FLOW_OK)
			{
				break;
			}

			//-----------------------------------------------------------------
			//{ KLV buffer injection

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

			// make empty buffer
			klv_frame_GstBuffer = gst_buffer_new();

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
				break;
			}

			//} ---------------------------------------------------------------
		}

		// statistics bookkeeping:
		frames++;
		if (image.nframe > lastframe)
		{
			lostframes += image.nframe - (lastframe + 1);
		}
		lastframe = image.nframe;
		curtime = getcurus();

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
			gtk_window_set_title(
				GTK_WINDOW(app->localDisplayCtrl.widgets.videoWindow),
				statistics_string);

			prevframes = frames;
			prevtime = curtime;
		}
	}
	//} end "main loop"

	//{ cleanup
	if (appsrc_video_caps)
		gst_caps_unref(appsrc_video_caps);
	if (size_caps)
		gst_caps_unref(size_caps);
	gst_app_src_end_of_stream(GST_APP_SRC(appsrc_video));
	gst_app_src_end_of_stream(GST_APP_SRC(appsrc_klv));
	gst_element_set_state(pipeline, GST_STATE_NULL);
	gst_object_unref(GST_OBJECT(pipeline));

	//exit:

	return lirena_XimeaStreamer_captureThread_terminate(app);
}





void *lirena_XimeaStreamer_captureThread_terminate(LirenaCaptureApp *appPtr)
{
	gdk_threads_enter();

	gtk_widget_destroy(appPtr->localDisplayCtrl.widgets.videoWindow);
	if (appPtr->camState.quitting)
	{
		gtk_main_quit();
	}
	xiStopAcquisition(appPtr->camState.cameraHandle);
	appPtr->camState.acquire = FALSE;
	xiCloseDevice(appPtr->camState.cameraHandle);
	appPtr->camState.cameraHandle = INVALID_HANDLE_VALUE;
	appPtr->captureThread = 0;

	gdk_threads_leave();

	return NULL;
}