
#include "LirenaCaptureDisplayController.h"

#include "LirenaCaptureApp.h"


// for later:
//#include "LirenaConfig.h"
//#include "LirenaKLVappsrc.h"

//-----------------------------------------------------------------------------


bool lirenaCaptureDisplayController_setupWidgets(LirenaCaptureDisplayController *dispCtrl)
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
	gtk_window_set_title(GTK_WINDOW(dispCtrl->widgets.controlWindow), "streamViewer control");
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
	g_signal_connect(appPtr->localDisplayCtrl.widgets.run, "toggled",
					 G_CALLBACK(lirenaCaptureDisplayController_initCam_startCaptureThread), (gpointer)appPtr); 



	g_signal_connect(appPtr->localDisplayCtrl.widgets.raw, "toggled",
					 G_CALLBACK(lirenaCaptureDisplayController_setupCamParams), (gpointer)appPtr);





	g_signal_connect(gtk_range_get_adjustment(GTK_RANGE(appPtr->localDisplayCtrl.widgets.gain)),
					 "value_changed", G_CALLBACK(update_gain), &appPtr->streamer.camParams);
	g_signal_connect(gtk_range_get_adjustment(GTK_RANGE(appPtr->localDisplayCtrl.widgets.exp)),
					 "value_changed", G_CALLBACK(update_exposure), &appPtr->streamer.camParams);
	g_signal_connect(gtk_spin_button_get_adjustment(GTK_SPIN_BUTTON(appPtr->localDisplayCtrl.widgets.x0)),
					 "value_changed", G_CALLBACK(update_x0), &appPtr->streamer.camParams);
	g_signal_connect(gtk_spin_button_get_adjustment(GTK_SPIN_BUTTON(appPtr->localDisplayCtrl.widgets.y0)),
					 "value_changed", G_CALLBACK(update_y0), &appPtr->streamer.camParams);
	g_signal_connect(gtk_spin_button_get_adjustment(GTK_SPIN_BUTTON(appPtr->localDisplayCtrl.widgets.cx)),
					 "value_changed", G_CALLBACK(update_cx), &appPtr->streamer.camParams);
	g_signal_connect(gtk_spin_button_get_adjustment(GTK_SPIN_BUTTON(appPtr->localDisplayCtrl.widgets.cy)),
					 "value_changed", G_CALLBACK(update_cy), &appPtr->streamer.camParams);
	g_signal_connect(appPtr->localDisplayCtrl.widgets.show, "toggled",
					 G_CALLBACK(update_show), &appPtr->streamer.camParams);


	// Messy hack to handle "shutdown app if CONTROL window is closed,
	// but to sth ales if RENDER window is closed".
	// The original logic didn't even work in the first place...
	close_cb_params *params = (close_cb_params *)g_malloc0(sizeof(close_cb_params));
	params->doShutDownApp = true; // do shutdown on closing of control window!
	params->captureThreadPtr = &appPtr->streamer.captureThread;
	params->camPtr= &appPtr->streamer.camParams;
	g_signal_connect(appPtr->localDisplayCtrl.widgets.controlWindow,
					 "delete_event", G_CALLBACK(close_cb), params);


    return true;
}






gboolean lirenaCaptureDisplayController_initCam_startCaptureThread(GtkToggleButton *run, LirenaCaptureApp *appPtr)
{
	LirenaCaptureWidgets *widgets = &appPtr->localDisplayCtrl.widgets;

	gtk_toggle_button_set_inconsistent(run, false);
	appPtr->streamer.camParams.acquire = gtk_toggle_button_get_active(run);
	if (appPtr->streamer.camParams.acquire && appPtr->streamer.camParams.cameraHandle == INVALID_HANDLE_VALUE)
	{
		lirena_XimeaStreamer_openCamera(&appPtr->streamer);






		// //{ downsample
		// XI_RETURN xiStatus = XI_OK;


		// // set downsampling "resolution" : XI_DWN_2x2 <-- half resolution
		// xiStatus = xiSetParamInt(
		// 	appPtr->streamer.camParams.cameraHandle, 
		// 	XI_PRM_DOWNSAMPLING, 
		// 	XI_DWN_2x2
		// 	//XI_DWN_4x4
		// );
		// if(xiStatus != XI_OK)
		// {
		// 	GST_WARNING("ximea downsampling XI_DWN_2x2: binning: return value not XI_OK: %d", xiStatus);
		// 	sleep(4);
		// }



		// // downsampling mode to binning:
		// xiStatus = xiSetParamInt(
		// 	appPtr->streamer.camParams.cameraHandle, 
		// 	XI_PRM_DOWNSAMPLING_TYPE, 
		// 	//XI_SKIPPING
		// 	XI_BINNING
		// );
		// if(xiStatus != XI_OK)
		// {
		// 	GST_WARNING("ximea downsampling type XI_PRM_DOWNSAMPLING_TYPE: XI_BINNING: return value not XI_OK: %d", xiStatus);
		// 	sleep(4);
		// }


	    // // binning mode to PRESERVE BAYER PATTERN (!!!111)
		// xiStatus = xiSetParamInt(
		// 	appPtr->streamer.camParams.cameraHandle,
		// 	XI_PRM_BINNING_HORIZONTAL_PATTERN, 
		// 	XI_BIN_BAYER
		// );
		// if(xiStatus != XI_OK)
		// {
		// 	GST_WARNING("XI_PRM_BINNING_HORIZONTAL_PATTERN: XI_BIN_BAYER: return value not XI_OK: %d", xiStatus);
		// 	sleep(4);
		// }
		// xiStatus = xiSetParamInt(
		// 	appPtr->streamer.camParams.cameraHandle,
		// 	XI_PRM_BINNING_VERTICAL_PATTERN, 
		// 	XI_BIN_BAYER
		// );
		// 		if(xiStatus != XI_OK)
		// {
		// 	GST_WARNING("XI_PRM_BINNING_VERTICAL_PATTERN: XI_BIN_BAYER: return value not XI_OK: %d", xiStatus);
		// 	sleep(4);
		// }



		// xiStatus = xiSetParamInt(
		// 	appPtr->streamer.camParams.cameraHandle,
		// 	XI_PRM_BINNING_HORIZONTAL_MODE, 
		// 	XI_BIN_MODE_AVERAGE
		// );
		// if(xiStatus != XI_OK)
		// {
		// 	GST_WARNING("ximea downsampling(binning) mode horiz.: XI_BIN_MODE_AVERAGE: return value not XI_OK: %d", xiStatus);
		// 	sleep(4);
		// }
		// xiStatus = xiSetParamInt(
		// 	appPtr->streamer.camParams.cameraHandle,
		// 	XI_PRM_BINNING_VERTICAL_MODE, 
		// 	XI_BIN_MODE_AVERAGE
		// );
		// if(xiStatus != XI_OK)
		// {
		// 	GST_WARNING("ximea downsampling(binning) mode vert.: XI_BIN_MODE_AVERAGE: return value not XI_OK: %d", xiStatus);
		// 	sleep(4);
		// }


		// //} downsample






		lirenaCaptureDisplayController_setupCamParams(GTK_TOGGLE_BUTTON(widgets->raw), appPtr);
		int isColor = 0;
		xiGetParamInt(appPtr->streamer.camParams.cameraHandle, XI_PRM_IMAGE_IS_COLOR, &isColor);
		if (isColor)
		{
			xiSetParamInt(appPtr->streamer.camParams.cameraHandle, XI_PRM_AUTO_WB, 1);
		}
		
		// set exposure from CLI arg
		xiSetParamInt(appPtr->streamer.camParams.cameraHandle, XI_PRM_EXPOSURE, 1000 * appPtr->config.exposure_ms);

		gtk_adjustment_set_value(
			gtk_range_get_adjustment(GTK_RANGE(widgets->exp)),
			appPtr->config.exposure_ms);

		if (pthread_create(&appPtr->streamer.captureThread,
						   NULL, lirena_XimeaStreamer_captureThread_run, (void *)appPtr))
		{
			exit(1);
		}
	}

	gtk_widget_set_sensitive(widgets->boxx, appPtr->streamer.camParams.acquire);
	gtk_widget_set_sensitive(widgets->boxy, appPtr->streamer.camParams.acquire);
	gtk_widget_set_sensitive(widgets->exp, appPtr->streamer.camParams.acquire);
	gtk_widget_set_sensitive(widgets->gain, appPtr->streamer.camParams.acquire);

	return TRUE;
}


 


gboolean lirenaCaptureDisplayController_setupCamParams(GtkToggleButton *raw, LirenaCaptureApp *appPtr)
{
	LirenaCaptureWidgets *widgets = &appPtr->localDisplayCtrl.widgets;
	HANDLE cameraHandle = appPtr->streamer.camParams.cameraHandle;

	if (cameraHandle != INVALID_HANDLE_VALUE)
	{
		float mingain, maxgain;
		xiSetParamInt(cameraHandle, XI_PRM_IMAGE_DATA_FORMAT, gtk_toggle_button_get_active(raw) ? XI_RAW8 : XI_RGB32);

		xiGetParamFloat(cameraHandle, XI_PRM_GAIN XI_PRM_INFO_MIN, &mingain);
		xiGetParamFloat(cameraHandle, XI_PRM_GAIN XI_PRM_INFO_MAX, &maxgain);
		xiGetParamInt(cameraHandle, XI_PRM_WIDTH XI_PRM_INFO_MAX, &appPtr->streamer.camParams.maxcx);
		xiGetParamInt(cameraHandle, XI_PRM_HEIGHT XI_PRM_INFO_MAX, &appPtr->streamer.camParams.maxcy);
		appPtr->streamer.camParams.roicx = appPtr->streamer.camParams.maxcx;
		appPtr->streamer.camParams.roicy = appPtr->streamer.camParams.maxcy;
		appPtr->streamer.camParams.roix0 = 0;
		appPtr->streamer.camParams.roiy0 = 0;

		float midgain = (maxgain + mingain) * 0.5f;

		gtk_adjustment_configure(gtk_range_get_adjustment(GTK_RANGE(widgets->gain)),
			midgain, mingain, maxgain, 0.1, 1, 0);
		gtk_adjustment_configure(gtk_spin_button_get_adjustment(
			GTK_SPIN_BUTTON(widgets->x0)), 
			appPtr->streamer.camParams.roix0, 
			0, appPtr->streamer.camParams.maxcx - 4, 2, 20, 0);
		gtk_adjustment_configure(gtk_spin_button_get_adjustment(
			GTK_SPIN_BUTTON(widgets->y0)), 
			appPtr->streamer.camParams.roiy0, 0, 
			appPtr->streamer.camParams.maxcy - 2, 2, 20, 0);
		gtk_adjustment_configure(gtk_spin_button_get_adjustment(
			GTK_SPIN_BUTTON(widgets->cx)), 
			appPtr->streamer.camParams.roicx, 4,
			appPtr->streamer.camParams.maxcx, 4, 20, 0);
		gtk_adjustment_configure(gtk_spin_button_get_adjustment(
			GTK_SPIN_BUTTON(widgets->cy)), 
			appPtr->streamer.camParams.roicy, 2, 
			appPtr->streamer.camParams.maxcy, 2, 20, 0);

		//xiSetParamFloat(cameraHandle, XI_PRM_GAIN, mingain);
		xiSetParamFloat(cameraHandle, XI_PRM_GAIN, midgain);


		// NO ROI
		// xiSetParamInt(cameraHandle, XI_PRM_OFFSET_X, appPtr->streamer.camParams.roix0);
		// xiSetParamInt(cameraHandle, XI_PRM_OFFSET_Y, appPtr->streamer.camParams.roiy0);
		// xiSetParamInt(cameraHandle, XI_PRM_WIDTH,    appPtr->streamer.camParams.roicx);
		// xiSetParamInt(cameraHandle, XI_PRM_HEIGHT, appPtr->streamer.camParams.roicy);

		//exposure doesn't seem to be affected by format change
	}
	return TRUE;
}




//-----------------------------------------------------------------------------
// Everything below seems irrelevant for non-GUI mode



gboolean lirenaCaptureDisplayController_initCamButtonSensitivity(LirenaCaptureApp *appPtr)
{
	int level = 0;

	if (appPtr->streamer.camParams.acquire &&
	    appPtr->streamer.camParams.cameraHandle != INVALID_HANDLE_VALUE)
	{
		//xiSetParamInt(appPtr->streamer.camParams.cameraHandle, XI_PRM_GPI_SELECTOR, 1);
		xiGetParamInt(appPtr->streamer.camParams.cameraHandle, XI_PRM_GPI_LEVEL, &level);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(appPtr->localDisplayCtrl.widgets.gpi1), level);
		//xiSetParamInt(appPtr->streamer.camParams.cameraHandle, XI_PRM_GPI_SELECTOR, 2);
		xiGetParamInt(appPtr->streamer.camParams.cameraHandle, XI_PRM_GPI_LEVEL, &level);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(appPtr->localDisplayCtrl.widgets.gpi2), level);
		//xiSetParamInt(appPtr->streamer.camParams.cameraHandle, XI_PRM_GPI_SELECTOR, 3);
		xiGetParamInt(appPtr->streamer.camParams.cameraHandle, XI_PRM_GPI_LEVEL, &level);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(appPtr->localDisplayCtrl.widgets.gpi3), level);
		//xiSetParamInt(appPtr->streamer.camParams.cameraHandle, XI_PRM_GPI_SELECTOR, 4);
		xiGetParamInt(appPtr->streamer.camParams.cameraHandle, XI_PRM_GPI_LEVEL, &level);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(appPtr->localDisplayCtrl.widgets.gpi4), level);
	}

	gtk_toggle_button_set_inconsistent(
		GTK_TOGGLE_BUTTON(appPtr->localDisplayCtrl.widgets.run),
		gtk_toggle_button_get_active(
			GTK_TOGGLE_BUTTON(appPtr->localDisplayCtrl.widgets.run)) != appPtr->streamer.camParams.acquire);

	gtk_widget_set_sensitive(appPtr->localDisplayCtrl.widgets.boxx, appPtr->streamer.camParams.acquire);
	gtk_widget_set_sensitive(appPtr->localDisplayCtrl.widgets.boxy, appPtr->streamer.camParams.acquire);
	gtk_widget_set_sensitive(appPtr->localDisplayCtrl.widgets.exp,  appPtr->streamer.camParams.acquire);
	gtk_widget_set_sensitive(appPtr->localDisplayCtrl.widgets.gain, appPtr->streamer.camParams.acquire);

	return TRUE;
}














gboolean lirenaCaptureDisplayController_startHack_cb(LirenaCaptureApp *appPtr)
{
	//start acquisition
	lirenaCaptureDisplayController_initCam_startCaptureThread(
		GTK_TOGGLE_BUTTON(appPtr->localDisplayCtrl.widgets.run),
		appPtr); 

	return FALSE;
}



GstBusSyncReply bus_sync_handler(GstBus *bus, GstMessage *message, 
 	LirenaCaptureDisplayController * displayCtrl) 
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
	LirenaCaptureDisplayController* displayCtrl) 
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
	//xiSetParamInt(cam->cameraHandle, XI_PRM_OFFSET_X, cam->roix0);
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
	//xiSetParamInt(cam->cameraHandle, XI_PRM_OFFSET_Y, cam->roiy0);
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
	//xiSetParamInt(cam->cameraHandle, XI_PRM_WIDTH, cam->roicx);
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
	//xiSetParamInt(cam->cameraHandle, XI_PRM_HEIGHT, cam->roicy);
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







