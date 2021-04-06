


#include "LirenaCaptureApp.h"


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
//struct LirenaXimeaStreamer_CameraParams;
//struct LirenaCaptureDisplay;

//----------------------------------------------------------------------------
// Type definitions



//----------------------------------------------------------------------------
int main(int argc, char **argv);

// creates zero-inited instance and parses and sets args/options
LirenaCaptureApp *lirena_captureApp_create(int argc, char **argv);
void lirena_captureApp_destroy(LirenaCaptureApp *appPtr);




//----------------------------------------------------------------------------









//-----------------------------------------------------------------------------
int main(int argc, char **argv)
{
	LirenaCaptureApp *appPtr = lirena_captureApp_create(argc, argv);

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
		// No-GUI- start-up:

		//non-GUI-glib main loop for GStreamer
		appPtr->pureLoop = g_main_loop_new (NULL, FALSE);

		if (//shall capture ?
			appPtr->streamer.camParams.acquire 
		)
		{
			// open cam
			gboolean success = 
				lirena_XimeaStreamer_openCamera(
					& appPtr->streamer
				);
			g_assert(success);

			// set reasonable params to cam
			if( success && 
				// cam opened?
				(appPtr->streamer.camParams.cameraHandle != INVALID_HANDLE_VALUE) 
			)
			{
				success &= 
					lirena_XimeaStreamer_setupCamParams(
						& appPtr->streamer.camParams,
						appPtr->streamer.configPtr
					);
				g_assert(success);
			}

			// launch capture thread so this main thread 
			// can stay free for bus message listening etc.
			if( success && 
				// capture thread not already existing?
				! appPtr->streamer.captureThreadIsRunning
			)
			{
				if (pthread_create(
						&appPtr->streamer.captureThread,
						NULL, 
						lirena_XimeaStreamer_captureThread_run, 
						(void *) appPtr
					)
					!= 0 // return 0 seems to mean succes for pthread...
				)
				{
					GST_ERROR("Capture thread (Pthread) "
					          "creation failed! exiting program ...");
					exit(1);
				}
			}

		}

		//non-GUI bus listening (no callbacks specced right now, though)
		g_main_loop_run(appPtr->pureLoop);

	}	


	//exit program

	appPtr->streamer.camParams.acquire = FALSE;
	
	if (appPtr->streamer.captureThread != 0)
	{
		pthread_join(appPtr->streamer.captureThread, NULL);
		appPtr->streamer.captureThread = 0;
		g_assert( ! appPtr->streamer.captureThreadIsRunning);
	}

	lirena_captureApp_destroy(appPtr);
	appPtr = NULL;

	return 0;
}

//----------------------------------------------------------------------------


// creates zero-inited instance and parses and sets args/options
LirenaCaptureApp *lirena_captureApp_create(int argc, char **argv)
{
	LirenaCaptureApp *appPtr =
		//init all to zero/nullptr/false
		(LirenaCaptureApp *) g_malloc0(sizeof(LirenaCaptureApp));


	appPtr->config = parseArguments(argc, argv);

	//for backtracking
	appPtr->streamer.configPtr = &appPtr->config;

	appPtr->streamer.captureThread = 0;

	appPtr->streamer.camParams.cameraHandle = INVALID_HANDLE_VALUE;

	//GUI-related stuff to follow:

	appPtr->streamer.camParams.acquire = TRUE;
	appPtr->streamer.camParams.doRender = TRUE;
	//why quitting default true in original code?
	appPtr->streamer.camParams.quitting = FALSE; 

	appPtr->localDisplayCtrl.drawableWindow_handle = 0;


	return appPtr;
}


void lirena_captureApp_destroy(LirenaCaptureApp *appPtr)
{
	g_free(appPtr);
}







