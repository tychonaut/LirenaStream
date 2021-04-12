


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
//LirenaCaptureApp *lirena_captureApp_create(int argc, char **argv);
//void lirena_captureApp_destroy(LirenaCaptureApp *appPtr);




//----------------------------------------------------------------------------









//-----------------------------------------------------------------------------
int main(int argc, char **argv)
{
	LirenaConfig* configPtr = new LirenaConfig(argc, argv);
	LirenaCaptureUI::init(configPtr);
	LirenaCaptureApp *appPtr = new LirenaCaptureApp(configPtr);



	delete appPtr;
	appPtr = nullptr;

	return 0;
	
	//-----------------------------------------------------------------------
	//old code:

	//TODO find out if this threading- ui mess can be reordered/reduced...
	if(appPtr->configPtr->doLocalDisplay)
	{
		#ifdef GDK_WINDOWING_X11
		XInitThreads();
		#endif

		gdk_threads_init();

		gdk_threads_enter();

	  	gtk_init(&argc, &argv);
	}


	gst_init(&argc, &argv);

	// have transformed until here ----	

	if(appPtr->configPtr->doLocalDisplay)
	{
		bool success = lirenaCaptureDisplayController_setupWidgets(appPtr->uiPtr);
		success &= lirenaCaptureDisplayController_setupCallbacks(appPtr);		
		//show GUI windows
		gtk_widget_show_all(appPtr->uiPtr->widgets.controlWindow);
		//start the GUI main loop
		gtk_main();
		//exit program
		gdk_threads_leave();
	}
	else
	{
		// No-GUI- start-up:

		//non-GUI-glib main loop for GStreamer
		appPtr->uiPtr->setupUI();
		//appPtr->pureMainLoop = g_main_loop_new (NULL, FALSE);

		if (//shall capture ?
			appPtr->streamerPtr->doAcquireFrames 
		)
		{
			// open cam
			gboolean success = 
				lirena_XimeaStreamer_openCamera(
					appPtr->streamerPtr
				);
			g_assert(success);

			// set reasonable params to cam
			if( success && 
				// cam opened?
				(appPtr->streamerPtr->camParams.cameraHandle != INVALID_HANDLE_VALUE) 
			)
			{
				success &= 
					lirena_XimeaStreamer_setupCamParams(
						& appPtr->streamerPtr->camParams,
						appPtr->streamerPtr->configPtr
					);
				g_assert(success);
			}

			// launch capture thread so this main thread 
			// can stay free for bus message listening etc.
			if( success && 
				// capture thread not already existing?
				! appPtr->streamerPtr->captureThreadIsRunning
			)
			{
				if (pthread_create(
						&appPtr->streamerPtr->captureThread,
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
		//g_main_loop_run(appPtr->pureMainLoop);
		appPtr->uiPtr->enterMainLoop();

	}	


	//exit program

	appPtr->streamerPtr->doAcquireFrames = FALSE;
	
	if (appPtr->streamerPtr->captureThread != 0)
	{
		pthread_join(appPtr->streamerPtr->captureThread, NULL);
		appPtr->streamerPtr->captureThread = 0;
		g_assert( ! appPtr->streamerPtr->captureThreadIsRunning);
	}

	//lirena_captureApp_destroy(appPtr);
	delete appPtr;
	appPtr = NULL;

	return 0;
}

//----------------------------------------------------------------------------


// creates zero-inited instance and parses and sets args/options
//LirenaCaptureApp *lirena_captureApp_create(int argc, char **argv)
LirenaCaptureApp::LirenaCaptureApp(LirenaConfig * configPtr)
	:
	configPtr(configPtr),
	streamerPtr( new LirenaStreamer(configPtr) ),
	uiPtr( LirenaCaptureUI::createInstance (streamerPtr) )
{





}


LirenaCaptureApp::~LirenaCaptureApp()
{
	delete uiPtr;
	uiPtr = nullptr;

	delete streamerPtr;
	streamerPtr = nullptr;
	
	delete configPtr;
	configPtr = nullptr;
};






