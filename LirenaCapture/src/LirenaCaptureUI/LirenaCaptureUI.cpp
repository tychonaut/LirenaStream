
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


LirenaCaptureUI* LirenaCaptureUI::createInstance(LirenaCaptureDevice* device)
{
	g_assert(0 && "TODO implement GUI, network UI for ximea and magewell;"
		"Also think about design of this diff: mutiple UI classes or a generic "
		"device interface?");
	return nullptr;
}



//-----------------------------------------------------------------------------


bool lirenaCaptureDisplayController_setupWidgets(LirenaCaptureUI *dispCtrl)
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
	dispCtrl->widgets.gpi1 = gtk_check_button_new_with_label("GPI1");
	dispCtrl->widgets.gpi2 = gtk_check_button_new_with_label("GPI2");
	dispCtrl->widgets.gpi3 = gtk_check_button_new_with_label("GPI3");
	dispCtrl->widgets.gpi4 = gtk_check_button_new_with_label("GPI4");
	dispCtrl->widgets.exp = gtk_hscale_new_with_range(1, 1000, 1);
	dispCtrl->widgets.gain = gtk_hscale_new_with_range(0, 1, 0.1);	 //use dummy limits
	dispCtrl->widgets.x0 = gtk_spin_button_new_with_range(0, 128, 2); //use dummy max limit
	dispCtrl->widgets.y0 = gtk_spin_button_new_with_range(0, 128, 2); //use dummy max limit
	dispCtrl->widgets.cx = gtk_spin_button_new_with_range(4, 128, 4); //use dummy max limit
	dispCtrl->widgets.cy = gtk_spin_button_new_with_range(2, 128, 2); //use dummy max limit
	dispCtrl->widgets.raw = gtk_toggle_button_new_with_label("Display RAW data");
	dispCtrl->widgets.show = gtk_toggle_button_new_with_label("Live view");
	dispCtrl->widgets.run = gtk_toggle_button_new_with_label("Acquisition");
	//tune them
	gtk_window_set_title(GTK_WINDOW(dispCtrl->widgets.controlWindow), "capture control");
	gtk_window_set_keep_above(GTK_WINDOW(dispCtrl->widgets.controlWindow), TRUE);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(dispCtrl->widgets.raw), TRUE);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(dispCtrl->widgets.show), TRUE);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(dispCtrl->widgets.run), TRUE); //actual start is delayed by 100ms, see below
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
	gtk_container_add(GTK_CONTAINER(dispCtrl->widgets.boxgpi), dispCtrl->widgets.gpi1);
	gtk_container_add(GTK_CONTAINER(dispCtrl->widgets.boxgpi), dispCtrl->widgets.gpi2);
	gtk_container_add(GTK_CONTAINER(dispCtrl->widgets.boxgpi), dispCtrl->widgets.gpi3);
	gtk_container_add(GTK_CONTAINER(dispCtrl->widgets.boxgpi), dispCtrl->widgets.gpi4);
	gtk_container_add(GTK_CONTAINER(dispCtrl->widgets.boxmain), dispCtrl->widgets.boxgpi);
	gtk_container_add(GTK_CONTAINER(dispCtrl->widgets.boxx), dispCtrl->widgets.labelx0);
	gtk_container_add(GTK_CONTAINER(dispCtrl->widgets.boxx), dispCtrl->widgets.x0);
	gtk_container_add(GTK_CONTAINER(dispCtrl->widgets.boxy), dispCtrl->widgets.labely0);
	gtk_container_add(GTK_CONTAINER(dispCtrl->widgets.boxy), dispCtrl->widgets.y0);
	gtk_container_add(GTK_CONTAINER(dispCtrl->widgets.boxx), dispCtrl->widgets.labelcx);
	gtk_container_add(GTK_CONTAINER(dispCtrl->widgets.boxx), dispCtrl->widgets.cx);
	gtk_container_add(GTK_CONTAINER(dispCtrl->widgets.boxy), dispCtrl->widgets.labelcy);
	gtk_container_add(GTK_CONTAINER(dispCtrl->widgets.boxy), dispCtrl->widgets.cy);
	gtk_container_add(GTK_CONTAINER(dispCtrl->widgets.boxmain), dispCtrl->widgets.boxx);
	gtk_container_add(GTK_CONTAINER(dispCtrl->widgets.boxmain), dispCtrl->widgets.boxy);
	gtk_container_add(GTK_CONTAINER(dispCtrl->widgets.boxmain), dispCtrl->widgets.labelexp);
	gtk_container_add(GTK_CONTAINER(dispCtrl->widgets.boxmain), dispCtrl->widgets.exp);
	gtk_container_add(GTK_CONTAINER(dispCtrl->widgets.boxmain), dispCtrl->widgets.labelgain);
	gtk_container_add(GTK_CONTAINER(dispCtrl->widgets.boxmain), dispCtrl->widgets.gain);
	gtk_container_add(GTK_CONTAINER(dispCtrl->widgets.boxmain), dispCtrl->widgets.raw);
	gtk_container_add(GTK_CONTAINER(dispCtrl->widgets.boxmain), dispCtrl->widgets.show);
	gtk_container_add(GTK_CONTAINER(dispCtrl->widgets.boxmain), dispCtrl->widgets.run);
	gtk_container_add(GTK_CONTAINER(dispCtrl->widgets.controlWindow), dispCtrl->widgets.boxmain);

	return true;
}


bool lirenaCaptureDisplayController_setupCallbacks(LirenaCaptureApp * appPtr)
{
    //register handlers


	// button sensitivity callback: irrelevant for non-GUI mode
	gdk_threads_add_timeout(1000, 
        (GSourceFunc)lirenaCaptureDisplayController_initCamButtonSensitivity,
        (gpointer)appPtr);


	// some kind of hack to force ANOTHER callback to be called early...
	// ... don't understand the details (MS)
	// is irrelevant for non-GUI mode
	//only way I (ximea dev) found to make sure window is displayed right away:
	gdk_threads_add_timeout(100, 
		(GSourceFunc)lirenaCaptureDisplayController_startHack_cb, 
		(gpointer)appPtr); 





	//this callback has important functionality that is also required without GUI
	g_signal_connect(appPtr->uiPtr->widgets.run, "toggled",
					 G_CALLBACK(lirenaCaptureDisplayController_initCam_startCaptureThread), (gpointer)appPtr); 



	g_signal_connect(appPtr->uiPtr->widgets.raw, "toggled",
					 G_CALLBACK(lirenaCaptureDisplayController_setupCamParams), (gpointer)appPtr);





	g_signal_connect(gtk_range_get_adjustment(GTK_RANGE(appPtr->uiPtr->widgets.gain)),
					 "value_changed", G_CALLBACK(update_gain), &appPtr->streamerPtr->camParams);
	g_signal_connect(gtk_range_get_adjustment(GTK_RANGE(appPtr->uiPtr->widgets.exp)),
					 "value_changed", G_CALLBACK(update_exposure), &appPtr->streamerPtr->camParams);
	g_signal_connect(gtk_spin_button_get_adjustment(GTK_SPIN_BUTTON(appPtr->uiPtr->widgets.x0)),
					 "value_changed", G_CALLBACK(update_x0), &appPtr->streamerPtr->camParams);
	g_signal_connect(gtk_spin_button_get_adjustment(GTK_SPIN_BUTTON(appPtr->uiPtr->widgets.y0)),
					 "value_changed", G_CALLBACK(update_y0), &appPtr->streamerPtr->camParams);
	g_signal_connect(gtk_spin_button_get_adjustment(GTK_SPIN_BUTTON(appPtr->uiPtr->widgets.cx)),
					 "value_changed", G_CALLBACK(update_cx), &appPtr->streamerPtr->camParams);
	g_signal_connect(gtk_spin_button_get_adjustment(GTK_SPIN_BUTTON(appPtr->uiPtr->widgets.cy)),
					 "value_changed", G_CALLBACK(update_cy), &appPtr->streamerPtr->camParams);
	g_signal_connect(appPtr->uiPtr->widgets.show, "toggled",
					 G_CALLBACK(update_show), &appPtr->streamerPtr->camParams);


	// Messy hack to handle "shutdown app if CONTROL window is closed,
	// but to sth ales if RENDER window is closed".
	// The original logic didn't even work in the first place...
	close_cb_params *params = (close_cb_params *)g_malloc0(sizeof(close_cb_params));
	params->doShutDownApp = true; // do shutdown on closing of control window!
	params->captureThreadPtr = &appPtr->streamerPtr->captureThread;
	params->camPtr= &appPtr->streamerPtr->camParams;
	g_signal_connect(appPtr->uiPtr->widgets.controlWindow,
					 "delete_event", G_CALLBACK(close_cb), params);


    return true;
}






gboolean lirenaCaptureDisplayController_initCam_startCaptureThread(GtkToggleButton *run, LirenaCaptureApp *appPtr)
{
	LirenaXimeaCaptureWidgets *widgets = &appPtr->uiPtr->widgets;

	gtk_toggle_button_set_inconsistent(run, false);
	appPtr->streamerPtr->camParams.acquire = gtk_toggle_button_get_active(run);
	if (appPtr->streamerPtr->camParams.acquire && appPtr->streamerPtr->camParams.cameraHandle == INVALID_HANDLE_VALUE)
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
			appPtr->streamerPtr->camParams.acquire = FALSE;
			return TRUE;
		}


		lirenaCaptureDisplayController_setupCamParams(GTK_TOGGLE_BUTTON(widgets->raw), appPtr);




		if (pthread_create(&appPtr->streamerPtr->captureThread,
						   NULL, lirena_XimeaStreamer_captureThread_run, (void *)appPtr))
		{
			exit(1);
		}
	}

	gtk_widget_set_sensitive(widgets->boxx, appPtr->streamerPtr->camParams.acquire);
	gtk_widget_set_sensitive(widgets->boxy, appPtr->streamerPtr->camParams.acquire);
	gtk_widget_set_sensitive(widgets->exp, appPtr->streamerPtr->camParams.acquire);
	gtk_widget_set_sensitive(widgets->gain, appPtr->streamerPtr->camParams.acquire);

	return TRUE;
}


 


gboolean lirenaCaptureDisplayController_setupCamParams(GtkToggleButton *raw, LirenaCaptureApp *appPtr)
{
	LirenaXimeaCaptureWidgets *widgets = &appPtr->uiPtr->widgets;
	HANDLE cameraHandle = appPtr->streamerPtr->camParams.cameraHandle;

	if (cameraHandle != INVALID_HANDLE_VALUE)
	{
		// query intrinsic device params
		float mingain, maxgain;
		xiSetParamInt(cameraHandle, XI_PRM_IMAGE_DATA_FORMAT, 
			gtk_toggle_button_get_active(raw) ? XI_RAW8 : XI_RGB32);
		xiGetParamFloat(cameraHandle, XI_PRM_GAIN XI_PRM_INFO_MIN, &mingain);
		xiGetParamFloat(cameraHandle, XI_PRM_GAIN XI_PRM_INFO_MAX, &maxgain);
		xiGetParamInt(cameraHandle, XI_PRM_WIDTH XI_PRM_INFO_MAX, &appPtr->streamerPtr->camParams.maxcx);
		xiGetParamInt(cameraHandle, XI_PRM_HEIGHT XI_PRM_INFO_MAX, &appPtr->streamerPtr->camParams.maxcy);
		int isColor = 0;
		xiGetParamInt(appPtr->streamerPtr->camParams.cameraHandle, XI_PRM_IMAGE_IS_COLOR, &isColor);

		// derive extrinsic device params
		appPtr->streamerPtr->camParams.roicx = appPtr->streamerPtr->camParams.maxcx;
		appPtr->streamerPtr->camParams.roicy = appPtr->streamerPtr->camParams.maxcy;
		appPtr->streamerPtr->camParams.roix0 = 0;
		appPtr->streamerPtr->camParams.roiy0 = 0;

		float midgain = (maxgain + mingain) * 0.5f;


		// assign extrinsic device params
		xiSetParamFloat(cameraHandle, XI_PRM_GAIN, midgain);
		xiSetParamInt(cameraHandle, XI_PRM_OFFSET_X, appPtr->streamerPtr->camParams.roix0);
		xiSetParamInt(cameraHandle, XI_PRM_OFFSET_Y, appPtr->streamerPtr->camParams.roiy0);
		xiSetParamInt(cameraHandle, XI_PRM_WIDTH,    appPtr->streamerPtr->camParams.roicx);
		xiSetParamInt(cameraHandle, XI_PRM_HEIGHT, appPtr->streamerPtr->camParams.roicy);
		
		if (isColor)
		{
			xiSetParamInt(appPtr->streamerPtr->camParams.cameraHandle, XI_PRM_AUTO_WB, 1);
		}

		// set exposure from CLI arg
		xiSetParamInt(appPtr->streamerPtr->camParams.cameraHandle, XI_PRM_EXPOSURE, 
			1000 * appPtr->configPtr->ximeaparams.exposure_ms);




		//adapt GUI widgets
		gtk_adjustment_configure(gtk_range_get_adjustment(GTK_RANGE(widgets->gain)),
			midgain, mingain, maxgain, 0.1, 1, 0);
		gtk_adjustment_configure(gtk_spin_button_get_adjustment(
			GTK_SPIN_BUTTON(widgets->x0)), 
			appPtr->streamerPtr->camParams.roix0, 
			0, appPtr->streamerPtr->camParams.maxcx - 4, 2, 20, 0);
		gtk_adjustment_configure(gtk_spin_button_get_adjustment(
			GTK_SPIN_BUTTON(widgets->y0)), 
			appPtr->streamerPtr->camParams.roiy0, 0, 
			appPtr->streamerPtr->camParams.maxcy - 2, 2, 20, 0);
		gtk_adjustment_configure(gtk_spin_button_get_adjustment(
			GTK_SPIN_BUTTON(widgets->cx)), 
			appPtr->streamerPtr->camParams.roicx, 4,
			appPtr->streamerPtr->camParams.maxcx, 4, 20, 0);
		gtk_adjustment_configure(gtk_spin_button_get_adjustment(
			GTK_SPIN_BUTTON(widgets->cy)), 
			appPtr->streamerPtr->camParams.roicy, 2, 
			appPtr->streamerPtr->camParams.maxcy, 2, 20, 0);

		gtk_adjustment_set_value(
			gtk_range_get_adjustment(GTK_RANGE(widgets->exp)),
			appPtr->configPtr->ximeaparams.exposure_ms);

	}
	return TRUE;
}




//-----------------------------------------------------------------------------
// Everything below seems irrelevant for non-GUI mode



gboolean lirenaCaptureDisplayController_initCamButtonSensitivity(LirenaCaptureApp *appPtr)
{
	int level = 0;

	if (appPtr->streamerPtr->camParams.acquire &&
	    appPtr->streamerPtr->camParams.cameraHandle != INVALID_HANDLE_VALUE)
	{
		xiSetParamInt(appPtr->streamerPtr->camParams.cameraHandle, XI_PRM_GPI_SELECTOR, 1);
		xiGetParamInt(appPtr->streamerPtr->camParams.cameraHandle, XI_PRM_GPI_LEVEL, &level);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(appPtr->uiPtr->widgets.gpi1), level);
		xiSetParamInt(appPtr->streamerPtr->camParams.cameraHandle, XI_PRM_GPI_SELECTOR, 2);
		xiGetParamInt(appPtr->streamerPtr->camParams.cameraHandle, XI_PRM_GPI_LEVEL, &level);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(appPtr->uiPtr->widgets.gpi2), level);
		xiSetParamInt(appPtr->streamerPtr->camParams.cameraHandle, XI_PRM_GPI_SELECTOR, 3);
		xiGetParamInt(appPtr->streamerPtr->camParams.cameraHandle, XI_PRM_GPI_LEVEL, &level);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(appPtr->uiPtr->widgets.gpi3), level);
		xiSetParamInt(appPtr->streamerPtr->camParams.cameraHandle, XI_PRM_GPI_SELECTOR, 4);
		xiGetParamInt(appPtr->streamerPtr->camParams.cameraHandle, XI_PRM_GPI_LEVEL, &level);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(appPtr->uiPtr->widgets.gpi4), level);
	}

	gtk_toggle_button_set_inconsistent(
		GTK_TOGGLE_BUTTON(appPtr->uiPtr->widgets.run),
		gtk_toggle_button_get_active(
			GTK_TOGGLE_BUTTON(appPtr->uiPtr->widgets.run)) != appPtr->streamerPtr->camParams.acquire);

	gtk_widget_set_sensitive(appPtr->uiPtr->widgets.boxx, appPtr->streamerPtr->camParams.acquire);
	gtk_widget_set_sensitive(appPtr->uiPtr->widgets.boxy, appPtr->streamerPtr->camParams.acquire);
	gtk_widget_set_sensitive(appPtr->uiPtr->widgets.exp,  appPtr->streamerPtr->camParams.acquire);
	gtk_widget_set_sensitive(appPtr->uiPtr->widgets.gain, appPtr->streamerPtr->camParams.acquire);

	return TRUE;
}














gboolean lirenaCaptureDisplayController_startHack_cb(LirenaCaptureApp *appPtr)
{
	//start acquisition
	lirenaCaptureDisplayController_initCam_startCaptureThread(
		GTK_TOGGLE_BUTTON(appPtr->uiPtr->widgets.run),
		appPtr); 

	return FALSE;
}



GstBusSyncReply bus_sync_handler(GstBus *bus, GstMessage *message, 
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



void video_widget_realize_cb(GtkWidget *widget, 
	LirenaCaptureUI* displayCtrl) 
{
	GdkWindow *window = gtk_widget_get_window(widget);
	if (!gdk_window_ensure_native(window))
	{
		g_error("Couldn't create native window needed for GstXOverlay!");
	}

	displayCtrl->drawableWindow_handle = GDK_WINDOW_XID(window);
}





gboolean close_cb(GtkWidget *, GdkEvent *,
				  //hack
				  close_cb_params *params)
{
	params->camPtr->quitting = (params->doShutDownApp) ? TRUE : FALSE;

	// pointer non null and pointee not 0 --> video thread is active
	if ((*(params->captureThreadPtr)) != 0)
	{
		//video thead is active, shall be shutdown first
		params->camPtr->acquire = FALSE;
	}
	else
	{
		//video thead is already joined, we can leave GUI stuff altogether
		gtk_main_quit();
	}

	g_free(params);

	return TRUE;
}









//following only widget-to-cam-param stuff ------------------------------------

gboolean update_show(GtkToggleButton *show, LirenaXimeaStreamer_CameraParams *cam) //gpointer)
{
	cam->doRender = gtk_toggle_button_get_active(show);
	return TRUE;
}


gboolean update_x0(GtkAdjustment *adj, LirenaXimeaStreamer_CameraParams *cam) //gpointer)
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

gboolean update_y0(GtkAdjustment *adj, LirenaXimeaStreamer_CameraParams *cam) //gpointer)
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

gboolean update_cx(GtkAdjustment *adj, LirenaXimeaStreamer_CameraParams *cam) //gpointer)
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

gboolean update_cy(GtkAdjustment *adj, LirenaXimeaStreamer_CameraParams *cam) //gpointer)
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

gboolean update_exposure(GtkAdjustment *adj, LirenaXimeaStreamer_CameraParams *cam) //gpointer)
{
	xiSetParamInt(cam->cameraHandle, XI_PRM_EXPOSURE, 
		1000 * gtk_adjustment_get_value(adj));

	return TRUE;
}

gboolean update_gain(GtkAdjustment *adj, LirenaXimeaStreamer_CameraParams *cam) //gpointer)
{
	xiSetParamFloat(cam->cameraHandle, XI_PRM_GAIN, gtk_adjustment_get_value(adj));
	return TRUE;
}







