


 #include "Lirena.h"


// #include <gst/app/gstappsrc.h>

// #include <gst/video/videooverlay.h>

// #include <sys/time.h>
// #include <pthread.h>
// #include <string.h>

//----------------------------------------------------------------------------
// Macros

//----------------------------------------------------------------------------
// Type forwards:

//struct LirenaCaptureApp;
//struct LirenaCamera;
//struct LirenaCaptureDisplay;

//----------------------------------------------------------------------------
// Type definitions



//----------------------------------------------------------------------------
int main(int argc, char **argv);

// creates zero-inited instance and parses and sets args/options
LirenaCaptureApp *lirena_camStreamApp_create(int argc, char **argv);
void lirena_camStreamApp_destroy(LirenaCaptureApp *appPtr);




//----------------------------------------------------------------------------









//-----------------------------------------------------------------------------
int main(int argc, char **argv)
{
	LirenaCaptureApp *appPtr = lirena_camStreamApp_create(argc, argv);

	//TODO find out if this threading- gui mess can be reordered/reduced...
	if(appPtr->config.doLocalDisplay)
	{
		#ifdef GDK_WINDOWING_X11
		XInitThreads();
		#endif

		gdk_threads_init();

		gdk_threads_enter();
	}
		

	gst_init(&argc, &argv);


	if(appPtr->config.doLocalDisplay)
	{
	  	gtk_init(&argc, &argv);

		bool success = lirenaCaptureDisplayController_setupWidgets(&appPtr->localDisplayCtrl);

		success &= lirenaCaptureDisplayController_setupCallbacks(appPtr);

			
		//show GUI windows
		gtk_widget_show_all(appPtr->localDisplayCtrl.widgets.controlWindow);
		

		//start the GUI main loop
		gtk_main();


		//exit program
		gdk_threads_leave();
	}
	else
	{
		//No GUI:

		//TODO setup non-GUI stuff that is in the callbacks



		//non-GUI-glib main loop for gstreamer
		appPtr->pureLoop = g_main_loop_new (NULL, FALSE);

		g_main_loop_run (appPtr->pureLoop);
	}	

	

	//exit program
	appPtr->camState.acquire = FALSE;
	if (appPtr->localDisplayCtrl.videoThread)
	{
		pthread_join(appPtr->localDisplayCtrl.videoThread, NULL);
		appPtr->localDisplayCtrl.videoThread = 0;
	}

	lirena_camStreamApp_destroy(appPtr);
	appPtr = NULL;

	return 0;
}

//----------------------------------------------------------------------------
//TODO outsource pure GUI code


// creates zero-inited instance and parses and sets args/options
LirenaCaptureApp *lirena_camStreamApp_create(int argc, char **argv)
{
	LirenaCaptureApp *appPtr =
		//init all to zero/nullptr/false
		(LirenaCaptureApp *) g_malloc0(sizeof(LirenaCaptureApp));

	appPtr->camState.cameraHandle = INVALID_HANDLE_VALUE;

	appPtr->camState.acquire = TRUE;
	appPtr->camState.doRender = TRUE;
	appPtr->camState.quitting = FALSE; //why quitting default true in original code?

	appPtr->config = parseArguments(argc, argv);

	//TODO continue;
	appPtr->localDisplayCtrl.videoThread = 0;
	appPtr->localDisplayCtrl.drawableWindow_handle = 0;

	return appPtr;
}

void lirena_camStreamApp_destroy(LirenaCaptureApp *appPtr)
{
	g_free(appPtr);
}







