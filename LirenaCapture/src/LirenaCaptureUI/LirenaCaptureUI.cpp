
#include "LirenaCaptureUI/LirenaCaptureUI.h"

#include "LirenaCaptureApp.h"


// for later:
//#include "LirenaConfig.h"
//#include "LirenaKLVappsrc.h"

 bool LirenaCaptureUI::init(LirenaConfig* configPtr)
 {
	static bool isAlreadyCalled = false;
	if(isAlreadyCalled)
	{
		// error: already initialized
		return false;
	}

	if(configPtr->doLocalDisplay || configPtr->haveLocalGUI)
	{
		#ifdef GDK_WINDOWING_X11
		XInitThreads();
		#endif

		gdk_threads_init();

		gdk_threads_enter();

	  	gtk_init( & configPtr->argc, & configPtr->argv);
	}

	 isAlreadyCalled = true;
	 return true;
 }


LirenaCaptureUI* LirenaCaptureUI::createInstance(LirenaStreamer* streamerPtr)
{
	g_assert(0 && "TODO implement GUI, network UI for ximea and magewell;"
		"Also think about design of this diff: mutiple UI classes or a generic "
		"device interface?");
	/*
		pseudocode:

		if(havelocalGUI || showLocalVideo)
		{
			if(devType = Ximea)
			{
				return new XimeaGUI
			}
			else
			{
				error cause magewell/videotestsrc GUI not supported yet
				exit(1);
			}
		}
		else
		{
			// return base UI class, implementing only a gmainloop 
			//  without any bus listeners
			return LirenaCaptureUI 
		}
	*/


	return nullptr;
}


LirenaCaptureUI::LirenaCaptureUI(LirenaStreamer* streamerPtr)
	:
	streamerPtr(streamerPtr),
	pureMainLoop(nullptr)
{

}
        
LirenaCaptureUI::~LirenaCaptureUI()
{
	//TODO think: is this sensible?
	shutdownUI();
}


bool LirenaCaptureUI::setupUI()
{
	g_assert(pureMainLoop == nullptr);
	pureMainLoop = g_main_loop_new (NULL, FALSE);
	return pureMainLoop != nullptr;
}


bool LirenaCaptureUI::setupCallbacks()
{
	// nothing to do in this minimal base class;
	return true;
}


 bool LirenaCaptureUI::enterMainLoop()
 {
	//non-GUI bus listening (no callbacks specced right now, though)
	g_main_loop_run(pureMainLoop);
	
	return true;
 }

 bool LirenaCaptureUI::exitMainLoop()
 {
	if(pureMainLoop != nullptr)
	{
		g_main_loop_quit(pureMainLoop);
		pureMainLoop = nullptr;

		return true;
	}
	else
	{
		GST_WARNING("%s", 
			"LirenaCaptureUI::shutdownUI() called on main loop nullptr;\n"
			"Maybe it is called more than once?");

		return true;
	}
 }


bool LirenaCaptureUI::shutdownUI()
{
	if(pureMainLoop != nullptr)
	{
		exitMainLoop();
		g_assert(pureMainLoop == nullptr);
	}

	return true;
}


bool LirenaCaptureUI::doMaintainOwnVideoWindow()
{
	bool doDisplayVideoLocally = streamerPtr->configPtr->doLocalDisplay;
	bool haveLocalGUI = streamerPtr->configPtr->haveLocalGUI;

	return (doDisplayVideoLocally && haveLocalGUI);
}


//-----------------------------------------------------------------------------
LirenaCaptureXimeaGUI::LirenaCaptureXimeaGUI(LirenaStreamer* streamerPtr)
	:
	LirenaCaptureUI(streamerPtr)
{
}

LirenaCaptureXimeaGUI::~LirenaCaptureXimeaGUI()
{
	//TODO think: is this sensible?
	shutdownUI();
}

bool LirenaCaptureXimeaGUI::setupUI()
{
	gdk_threads_enter();
	
	GST_ERROR("LirenaCaptureXimeaGUI::setupUI(): TODO IMPLEMENT");
	return false;

	gdk_threads_leave();
}


bool LirenaCaptureXimeaGUI::setupCallbacks()
{
	gdk_threads_enter();

	//adapted from C-style function:
	//  gboolean lirenaCaptureXimeaGUI_createWidgets(LirenaCaptureUI *dispCtrl)

	g_signal_connect(
		gtk_range_get_adjustment(
			GTK_RANGE(widgets.gain)),
		"value_changed",
		G_CALLBACK(lirenaCaptureXimeaGUI_cb_updateGain),
		&streamerPtr->camParams);

	g_signal_connect(
		gtk_range_get_adjustment(
			GTK_RANGE(widgets.exp)),
		"value_changed",
		G_CALLBACK(lirenaCaptureXimeaGUI_cb_updateExposure),
		&streamerPtr->camParams);

	g_signal_connect(
		gtk_spin_button_get_adjustment(
			GTK_SPIN_BUTTON(widgets.x0)),
		"value_changed",
		G_CALLBACK(lirenaCaptureXimeaGUI_cb_updateX0),
		&streamerPtr->camParams);

	g_signal_connect(
		gtk_spin_button_get_adjustment(
			GTK_SPIN_BUTTON(widgets.y0)),
		"value_changed",
		G_CALLBACK(lirenaCaptureXimeaGUI_cb_updateY0),
		&streamerPtr->camParams);

	g_signal_connect(
		gtk_spin_button_get_adjustment(
			GTK_SPIN_BUTTON(widgets.cx)),
		"value_changed", 
		G_CALLBACK(lirenaCaptureXimeaGUI_cb_updateCx), 
		&streamerPtr->camParams);

	g_signal_connect(
		gtk_spin_button_get_adjustment(
			GTK_SPIN_BUTTON(widgets.cy)),
		"value_changed", 
		G_CALLBACK(lirenaCaptureXimeaGUI_cb_updateCy), 
		&streamerPtr->camParams);

	g_signal_connect(
		widgets.controlWindow,
		"delete_event", 
		G_CALLBACK(LirenaCaptureXimeaGUI_cb_closeWindow),
		streamerPtr);

	//g_signal_connect(
	//	widgets.show,
	//  "toggled",
	//	G_CALLBACK(update_show), 
	//  &streamerPtr->camParams);

	gdk_threads_leave();

	return TRUE;
}



 bool LirenaCaptureXimeaGUI::enterMainLoop()
 {
	gdk_threads_enter();

	//show GUI windows
	gtk_widget_show_all(widgets.controlWindow);
	//TODO find out if nececcary, and if yes in what order wrt. gtk_widget_show_all
	//gtk_widget_realize(appPtr->uiPtr->widgets.controlWindow);


	//start the GUI main loop
	gtk_main();

	gdk_threads_leave();

	return true;
	//n.b. probably exiting program  after return from main loop
 }

 bool LirenaCaptureXimeaGUI::exitMainLoop()
 {
	gdk_threads_enter();

	// don't bother with window destruction, just shut down GTK
	// the dirty way
	//gtk_widget_destroy(appPtr->uiPtr->widgets.videoWindow);
		
	gtk_main_quit();

	gdk_threads_leave();

	return true;
 }


bool LirenaCaptureXimeaGUI::shutdownUI()
{
	// calling gtk_main_quit() more than once seems no problem:
	// https://book.huihoo.com/gtk+-gnome-application-development/sec-mainloop.html
	exitMainLoop();

	return true;
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------


//lirenaCaptureXimeaGUI_createWidgets
gboolean lirenaCaptureXimeaGUI_createWidgets(LirenaCaptureUI *dispCtrl)
{
	//create widgets
	dispCtrl->widgets.controlWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	dispCtrl->widgets.boxmain = gtk_vbox_new(FALSE, 0);
	dispCtrl->widgets.boxgpi = gtk_hbox_new(TRUE, 0);
	dispCtrl->widgets.boxx = gtk_hbox_new(FALSE, 0);
	dispCtrl->widgets.boxy = gtk_hbox_new(FALSE, 0);
	dispCtrl->widgets.labelexp = gtk_label_new("Exposure (ms)");
	dispCtrl->widgets.labelgain = gtk_label_new("Gain (dB)");
	dispCtrl->widgets.labelx0 = gtk_label_new("x0");
	dispCtrl->widgets.labelcx = gtk_label_new("cx");
	dispCtrl->widgets.labely0 = gtk_label_new("y0");
	dispCtrl->widgets.labelcy = gtk_label_new("cy");

	for (int i = 0; i < LIRENA_XIMEA_MAX_GPI_SELECTORS; i++)
	{
		const int maxSize=256;
		gchar label[maxSize];
		//std::string()
		g_snprintf(label,
			   (gulong)maxSize,
			   "%s%i", "GPI", i);

		dispCtrl->widgets.gpi_levels[i] =
			gtk_check_button_new_with_label(label);
	}
	// dispCtrl->widgets.gpi1 = gtk_check_button_new_with_label("GPI1");
	// dispCtrl->widgets.gpi2 = gtk_check_button_new_with_label("GPI2");
	// dispCtrl->widgets.gpi3 = gtk_check_button_new_with_label("GPI3");
	// dispCtrl->widgets.gpi4 = gtk_check_button_new_with_label("GPI4");

	dispCtrl->widgets.exp = gtk_hscale_new_with_range(1, 1000, 1);
	dispCtrl->widgets.gain = gtk_hscale_new_with_range(0, 1, 0.1);	 //use dummy limits
	dispCtrl->widgets.x0 = gtk_spin_button_new_with_range(0, 128, 2); //use dummy max limit
	dispCtrl->widgets.y0 = gtk_spin_button_new_with_range(0, 128, 2); //use dummy max limit
	dispCtrl->widgets.cx = gtk_spin_button_new_with_range(4, 128, 4); //use dummy max limit
	dispCtrl->widgets.cy = gtk_spin_button_new_with_range(2, 128, 2); //use dummy max limit
	//dispCtrl->widgets.raw = gtk_toggle_button_new_with_label("Display RAW data");
	//dispCtrl->widgets.show = gtk_toggle_button_new_with_label("Live view");
	//dispCtrl->widgets.run = gtk_toggle_button_new_with_label("Acquisition");
	//tune them
	gtk_window_set_title(GTK_WINDOW(dispCtrl->widgets.controlWindow), "capture control");
	gtk_window_set_keep_above(GTK_WINDOW(dispCtrl->widgets.controlWindow), TRUE);
	//gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(dispCtrl->widgets.raw), TRUE);
	//gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(dispCtrl->widgets.show), TRUE);
	//gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(dispCtrl->widgets.run), TRUE); //actual start is delayed by 100ms, see below
	gtk_widget_set_sensitive(dispCtrl->widgets.boxgpi, FALSE);
	gtk_scale_set_digits(GTK_SCALE(dispCtrl->widgets.exp), 0);
	gtk_range_set_update_policy(GTK_RANGE(dispCtrl->widgets.exp), GTK_UPDATE_DISCONTINUOUS);
	gtk_range_set_update_policy(GTK_RANGE(dispCtrl->widgets.gain), GTK_UPDATE_DISCONTINUOUS);
	gtk_adjustment_set_value(gtk_range_get_adjustment(GTK_RANGE(dispCtrl->widgets.exp)), 10);
	gtk_scale_set_value_pos(GTK_SCALE(dispCtrl->widgets.exp), GTK_POS_RIGHT);
	gtk_scale_set_value_pos(GTK_SCALE(dispCtrl->widgets.gain), GTK_POS_RIGHT);
	gtk_widget_set_sensitive(dispCtrl->widgets.boxx, FALSE);
	gtk_widget_set_sensitive(dispCtrl->widgets.boxy, FALSE);
	gtk_widget_set_sensitive(dispCtrl->widgets.exp, FALSE);
	gtk_widget_set_sensitive(dispCtrl->widgets.gain, FALSE);
	//pack everything into window

	for (int i = 0; i < LIRENA_XIMEA_MAX_GPI_SELECTORS; i++)
	{
		gtk_container_add(
			GTK_CONTAINER(dispCtrl->widgets.boxgpi), 
			dispCtrl->widgets.gpi_levels[i]);
	}


	gtk_container_add(GTK_CONTAINER(dispCtrl->widgets.boxmain), dispCtrl->widgets.boxgpi);
	gtk_container_add(GTK_CONTAINER(dispCtrl->widgets.boxx), dispCtrl->widgets.labelx0);
	gtk_container_add(GTK_CONTAINER(dispCtrl->widgets.boxx), dispCtrl->widgets.x0);
	gtk_container_add(GTK_CONTAINER(dispCtrl->widgets.boxy), dispCtrl->widgets.labely0);
	gtk_container_add(GTK_CONTAINER(dispCtrl->widgets.boxy), dispCtrl->widgets.y0);
	gtk_container_add(GTK_CONTAINER(dispCtrl->widgets.boxx), dispCtrl->widgets.labelcx);
	gtk_container_add(GTK_CONTAINER(dispCtrl->widgets.boxx), dispCtrl->widgets.cx);
	gtk_container_add(GTK_CONTAINER(dispCtrl->widgets.boxy), dispCtrl->widgets.cy);
	gtk_container_add(GTK_CONTAINER(dispCtrl->widgets.boxmain), dispCtrl->widgets.boxx);
	gtk_container_add(GTK_CONTAINER(dispCtrl->widgets.boxmain), dispCtrl->widgets.boxy);
	gtk_container_add(GTK_CONTAINER(dispCtrl->widgets.boxmain), dispCtrl->widgets.labelexp);
	gtk_container_add(GTK_CONTAINER(dispCtrl->widgets.boxmain), dispCtrl->widgets.exp);
	gtk_container_add(GTK_CONTAINER(dispCtrl->widgets.boxmain), dispCtrl->widgets.labelgain);
	gtk_container_add(GTK_CONTAINER(dispCtrl->widgets.boxmain), dispCtrl->widgets.gain);


	gtk_container_add(GTK_CONTAINER(dispCtrl->widgets.controlWindow), dispCtrl->widgets.boxmain);

	return TRUE;
}

gboolean lirenaCaptureGUI_setupCallbacks(LirenaCaptureApp *appPtr)
{
	g_signal_connect(
		gtk_range_get_adjustment(
			GTK_RANGE(appPtr->uiPtr->widgets.gain)),
		"value_changed",
		G_CALLBACK(lirenaCaptureXimeaGUI_cb_updateGain),
		&appPtr->streamerPtr->camParams);

	g_signal_connect(
		gtk_range_get_adjustment(
			GTK_RANGE(appPtr->uiPtr->widgets.exp)),
		"value_changed",
		G_CALLBACK(lirenaCaptureXimeaGUI_cb_updateExposure),
		&appPtr->streamerPtr->camParams);

	g_signal_connect(
		gtk_spin_button_get_adjustment(
			GTK_SPIN_BUTTON(appPtr->uiPtr->widgets.x0)),
		"value_changed",
		G_CALLBACK(lirenaCaptureXimeaGUI_cb_updateX0),
		&appPtr->streamerPtr->camParams);

	g_signal_connect(
		gtk_spin_button_get_adjustment(
			GTK_SPIN_BUTTON(appPtr->uiPtr->widgets.y0)),
		"value_changed",
		G_CALLBACK(lirenaCaptureXimeaGUI_cb_updateY0),
		&appPtr->streamerPtr->camParams);

	g_signal_connect(
		gtk_spin_button_get_adjustment(
			GTK_SPIN_BUTTON(appPtr->uiPtr->widgets.cx)),
		"value_changed", 
		G_CALLBACK(lirenaCaptureXimeaGUI_cb_updateCx), 
		&appPtr->streamerPtr->camParams);

	g_signal_connect(
		gtk_spin_button_get_adjustment(
			GTK_SPIN_BUTTON(appPtr->uiPtr->widgets.cy)),
		"value_changed", 
		G_CALLBACK(lirenaCaptureXimeaGUI_cb_updateCy), 
		&appPtr->streamerPtr->camParams);

	g_signal_connect(
		appPtr->uiPtr->widgets.controlWindow,
		"delete_event", 
		G_CALLBACK(LirenaCaptureXimeaGUI_cb_closeWindow),
		appPtr->streamerPtr);

	//g_signal_connect(
	//	appPtr->uiPtr->widgets.show,
	//  "toggled",
	//	G_CALLBACK(update_show), 
	//  &appPtr->streamerPtr->camParams);

	return TRUE;
}



// open cam
gboolean lirenaXimeaCaptureDevice_openCam(LirenaCaptureApp *appPtr)
{
	if( appPtr->streamerPtr->camParams.cameraHandle == INVALID_HANDLE_VALUE)
	{
		DWORD nIndex = 0;
		char *env = getenv("CAM_INDEX");
		if (env)
		{
			nIndex = atoi(env);
		
		}
		DWORD tmp;
		xiGetNumberDevices(&tmp); //rescan available devices

		if (xiOpenDevice(nIndex, &appPtr->streamerPtr->camParams.cameraHandle) != XI_OK)
		{
			printf("Couldn't setup camera!\n");
			appPtr->streamerPtr->doAcquireFrames = FALSE;
			return FALSE;
		}

		return TRUE;
	}
	else
	{
		g_assert(0 && "lirenaXimeaCaptureDevice_openCam: cam already opened!");
		return FALSE;
	}
}


gboolean lirenaStreamer_startCaptureThread(
	LirenaCaptureApp *appPtr)
{
	g_assert(appPtr->streamerPtr->doAcquireFrames &&
		"capture thread must only be stardet if capturing is desired");
	
	int pthread_ret = pthread_create(
						&appPtr->streamerPtr->captureThread,
						NULL, 
						lirena_XimeaStreamer_captureThread_run, 
						(void *)appPtr);

	if (pthread_ret != 0)
	{
		//triple fail redundancy ;(
		g_assert(0 &&  "capture thread creation failed!");
		exit(1);
		return FALSE;
	}
	
	return TRUE;
}







gboolean lirenaXimeaCaptureDevice_setupCamParams(
    LirenaCaptureApp* appPtr)
{
	// relict of oold code: we now only take raw stuff,
	// as the software debayering from ximea is awfully slow
	bool raw = true;

	LirenaXimeaStreamer_CameraParams * camParamsPtr = 
			&appPtr->streamerPtr->camParams;

	
	if (camParamsPtr->cameraHandle != INVALID_HANDLE_VALUE)
	{
		// query intrinsic device params

		xiSetParamInt(camParamsPtr->cameraHandle, 
			XI_PRM_IMAGE_DATA_FORMAT, 
			// gtk_toggle_button_get_active(raw) 
			raw ? XI_RAW8 : XI_RGB32);
		xiGetParamFloat(camParamsPtr->cameraHandle, 
			XI_PRM_GAIN XI_PRM_INFO_MIN, &camParamsPtr->mingain);
		xiGetParamFloat(camParamsPtr->cameraHandle, 
			XI_PRM_GAIN XI_PRM_INFO_MAX, &camParamsPtr->maxgain);
		xiGetParamInt(camParamsPtr->cameraHandle, 
			XI_PRM_WIDTH XI_PRM_INFO_MAX, &camParamsPtr->maxcx);
		xiGetParamInt(camParamsPtr->cameraHandle, 
			XI_PRM_HEIGHT XI_PRM_INFO_MAX, &camParamsPtr->maxcy);

		xiGetParamInt(camParamsPtr->cameraHandle, XI_PRM_IMAGE_IS_COLOR, &camParamsPtr->isColor);

		// derive extrinsic device params
		camParamsPtr->roicx = camParamsPtr->maxcx;
		camParamsPtr->roicy = camParamsPtr->maxcy;
		camParamsPtr->roix0 = 0;
		camParamsPtr->roiy0 = 0;

		//intermediate value: take middle until TODO make configurable
		camParamsPtr->gainToUse = (camParamsPtr->maxgain + camParamsPtr->mingain) * 0.5f;


		//{  assign extrinsic device params
		xiSetParamFloat(camParamsPtr->cameraHandle , XI_PRM_GAIN, camParamsPtr->gainToUse);
		xiSetParamInt(camParamsPtr->cameraHandle , XI_PRM_OFFSET_X, camParamsPtr->roix0);
		xiSetParamInt(camParamsPtr->cameraHandle , XI_PRM_OFFSET_Y, camParamsPtr->roiy0);
		xiSetParamInt(camParamsPtr->cameraHandle , XI_PRM_WIDTH,    camParamsPtr->roicx);
		xiSetParamInt(camParamsPtr->cameraHandle , XI_PRM_HEIGHT,   camParamsPtr->roicy);
		
		if (camParamsPtr->isColor)
		{
			xiSetParamInt(camParamsPtr->cameraHandle, XI_PRM_AUTO_WB, 1);
		}

		// set exposure from CLI arg
		xiSetParamInt(camParamsPtr->cameraHandle, XI_PRM_EXPOSURE, 
			1000 * appPtr->configPtr->ximeaparams.exposure_ms);

		//}
	
		//{ Query GPI stuff ...
		// TODO adapt this to the cam mode we actually use, consult
		// https://www.ximea.com/support/wiki/apis/xiapi_manual#XI_PRM_GPI_SELECTOR-or-gpi_selector

		for (int i = 0; i < LIRENA_XIMEA_MAX_GPI_SELECTORS; i++)
		{
			// select current "logical input pin"
			xiSetParamInt(appPtr->streamerPtr->camParams.cameraHandle, 
				XI_PRM_GPI_SELECTOR, 
				//seems to start at 1? TODO investigate!
				i+1);
			
			// get status of that "logical input pin".
			// I don't get the meaning of this yet. 
			// Logical Pin exists? 
			// Pin is configured for controlling programmatically? <-- this is my best guess
			// Kind of Signal that is required to "fire" (0, 1, falling, rising flank)?
			xiGetParamInt(appPtr->streamerPtr->camParams.cameraHandle, 
				XI_PRM_GPI_LEVEL, 
				& appPtr->streamerPtr->camParams.gpi_levels[i]);
		}
		//} end GPI stuff 
	}



	return TRUE;
}


gboolean lirenaCaptureXimeaGUI_updateWidgets(
    LirenaCaptureApp* appPtr)
{
	LirenaXimeaStreamer_CameraParams * camParamsPtr = 
			&appPtr->streamerPtr->camParams;

	//adapt GUI widgets
	if (camParamsPtr->cameraHandle  != INVALID_HANDLE_VALUE)
	{
		LirenaXimeaCaptureWidgets *widgets = &appPtr->uiPtr->widgets;

		gtk_adjustment_configure(
			gtk_range_get_adjustment(GTK_RANGE(widgets->gain)),
			camParamsPtr->gainToUse, 
			camParamsPtr->mingain, 
			camParamsPtr->maxgain, 
			0.1, 1, 0);
		gtk_adjustment_configure(gtk_spin_button_get_adjustment(
			GTK_SPIN_BUTTON(widgets->x0)), 
			camParamsPtr->roix0, 
			0, camParamsPtr->maxcx - 4, 2, 20, 0);
		gtk_adjustment_configure(gtk_spin_button_get_adjustment(
			GTK_SPIN_BUTTON(widgets->y0)), 
			camParamsPtr->roiy0, 0, 
			camParamsPtr->maxcy - 2, 2, 20, 0);
		gtk_adjustment_configure(gtk_spin_button_get_adjustment(
			GTK_SPIN_BUTTON(widgets->cx)), 
			camParamsPtr->roicx, 4,
			camParamsPtr->maxcx, 4, 20, 0);
		gtk_adjustment_configure(gtk_spin_button_get_adjustment(
			GTK_SPIN_BUTTON(widgets->cy)), 
			camParamsPtr->roicy, 2, 
			camParamsPtr->maxcy, 2, 20, 0);

		gtk_adjustment_set_value(
			gtk_range_get_adjustment(GTK_RANGE(widgets->exp)),
			appPtr->configPtr->ximeaparams.exposure_ms);


		g_assert(appPtr->streamerPtr->doAcquireFrames && 
			"should be true all the time in this minimal setup");
		appPtr->streamerPtr->doAcquireFrames = true; 

		gtk_widget_set_sensitive(widgets->boxx, appPtr->streamerPtr->doAcquireFrames);
		gtk_widget_set_sensitive(widgets->boxy, appPtr->streamerPtr->doAcquireFrames);
		gtk_widget_set_sensitive(widgets->exp, appPtr->streamerPtr->doAcquireFrames);
		gtk_widget_set_sensitive(widgets->gain, appPtr->streamerPtr->doAcquireFrames);
	
		//GPI stuff; TODO understand this ...
		for (int i = 0; i < LIRENA_XIMEA_MAX_GPI_SELECTORS; i++)
		{
			gtk_toggle_button_set_active(
				GTK_TOGGLE_BUTTON(appPtr->uiPtr->widgets.gpi_levels[i]), 
				appPtr->streamerPtr->camParams.gpi_levels[i]);
		}
		//} end GPI stuff
	}

	return TRUE;
}

 




//-----------------------------------------------------------------------------
// Everything below seems irrelevant for non-GUI mode







GstBusSyncReply lirenaCaptureXimeaGUI_cb_handleBusSyncEvent(GstBus *bus, GstMessage *message, 
 	LirenaCaptureUI * displayCtrl) 
{
	if (!gst_is_video_overlay_prepare_window_handle_message(message))
	{
		return GST_BUS_PASS;
	}

	gst_video_overlay_set_window_handle(
		GST_VIDEO_OVERLAY(GST_MESSAGE_SRC(message)), 
		displayCtrl->drawableWindow_handle);

	gst_message_unref(message);
	return GST_BUS_DROP;
}



void lirenaCaptureXimeaGUI_cb_grabXhandleForVidWindow(GtkWidget *widget, 
	LirenaCaptureUI* displayCtrl) 
{
	GdkWindow *window = gtk_widget_get_window(widget);
	if (!gdk_window_ensure_native(window))
	{
		g_error("Couldn't create native window needed for GstXOverlay!");
	}

	displayCtrl->drawableWindow_handle = GDK_WINDOW_XID(window);
}





gboolean LirenaCaptureXimeaGUI_cb_closeWindow(GtkWidget *, GdkEvent *,
				  LirenaStreamer* streamerPtr)
{
	// pointer non null and pointee not 0 --> video thread is active
	if (streamerPtr->captureThreadIsRunning)
	{
		//video thead is active, shall be shutdown first
		streamerPtr->doAcquireFrames = FALSE;

		pthread_join(streamerPtr->captureThread, NULL);
		streamerPtr->captureThread = 0;
		streamerPtr->captureThreadIsRunning = false;
	}
	
	//video thead is joined, we can leave GUI stuff altogether
	gtk_main_quit();
	
	return TRUE;
}









//following only widget-to-cam-param stuff ------------------------------------

// gboolean update_show(GtkToggleButton *show,
//	LirenaXimeaStreamer_CameraParams *cam)
// {
// 	cam->doRender = gtk_toggle_button_get_active(show);
// 	return TRUE;
// }


gboolean lirenaCaptureXimeaGUI_cb_updateX0(GtkAdjustment *adj,
	LirenaXimeaStreamer_CameraParams *cam)
{
	
	cam->roix0 = gtk_adjustment_get_value(adj);
	if (cam->roicx + cam->roix0 > cam->maxcx)
	{
		cam->roix0 = cam->maxcx - cam->roicx;
		gtk_adjustment_set_value(adj, cam->roix0);
	}
	xiSetParamInt(cam->cameraHandle, XI_PRM_OFFSET_X, cam->roix0);
	return TRUE;
}

gboolean lirenaCaptureXimeaGUI_cb_updateY0(GtkAdjustment *adj,
	LirenaXimeaStreamer_CameraParams *cam)
{
	cam->roiy0 = gtk_adjustment_get_value(adj);
	if (cam->roicy + cam->roiy0 > cam->maxcy)
	{
		cam->roiy0 = cam->maxcy - cam->roicy;
		gtk_adjustment_set_value(adj, cam->roiy0);
	}
	xiSetParamInt(cam->cameraHandle, XI_PRM_OFFSET_Y, cam->roiy0);
	return TRUE;
}

gboolean lirenaCaptureXimeaGUI_cb_updateCx(GtkAdjustment *adj,
	LirenaXimeaStreamer_CameraParams *cam)
{
	cam->roicx = gtk_adjustment_get_value(adj);
	if (cam->roix0 + cam->roicx > cam->maxcx)
	{
		cam->roicx = cam->maxcx - cam->roix0;
		gtk_adjustment_set_value(adj, cam->roicx);
	}
	xiSetParamInt(cam->cameraHandle, XI_PRM_WIDTH, cam->roicx);
	return TRUE;
}

gboolean lirenaCaptureXimeaGUI_cb_updateCy(GtkAdjustment *adj,
	LirenaXimeaStreamer_CameraParams *cam)
{
	cam->roicy = gtk_adjustment_get_value(adj);
	if (cam->roiy0 + cam->roicy > cam->maxcy)
	{
		cam->roicy = cam->maxcy - cam->roiy0;
		gtk_adjustment_set_value(adj, cam->roicy);
	}
	xiSetParamInt(cam->cameraHandle, XI_PRM_HEIGHT, cam->roicy);
	return TRUE;
}

gboolean lirenaCaptureXimeaGUI_cb_updateExposure(GtkAdjustment *adj,
	LirenaXimeaStreamer_CameraParams *cam)
{
	xiSetParamInt(cam->cameraHandle, XI_PRM_EXPOSURE, 
		1000 * gtk_adjustment_get_value(adj));

	return TRUE;
}

gboolean lirenaCaptureXimeaGUI_cb_updateGain(GtkAdjustment *adj,
	LirenaXimeaStreamer_CameraParams *cam)
{
	xiSetParamFloat(cam->cameraHandle, XI_PRM_GAIN, gtk_adjustment_get_value(adj));
	return TRUE;
}







