


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



bool LirenaCaptureApp::run()
{
	bool success = true;

	// first, make device ready: open, init...
	success &= streamerPtr->setupCaptureDevice();
	g_assert(success);

	// maybe create GUI and callbacks to streamer and device
	success &= uiPtr->setupUI();
	g_assert(success);
	success &= uiPtr->setupCallbacks();
	g_assert(success);
	
	//configure gstreamer
	success &= streamerPtr->setupGStreamerPipeline();
	g_assert(success);

	// start acquiring and "publishing" via gstreamer
	// in separate thread
	success &= streamerPtr->launchCaptureThread(this);
	g_assert(success);

	// make this (main) thread the (G)UI thread:
	// no return until UI says so
	success &= uiPtr->enterMainLoop();
	g_assert(success);

	//{ cleanup: ---
	success &= uiPtr->shutdownUI();
	g_assert(success);

	// shutdown capture thread  (if not already terminated):
	success &= streamerPtr->terminateCaptureThread();
	g_assert(success);

	//} ------------

	return success;
}




//-----------------------------------------------------------------------------
int main(int argc, char **argv)
{
	//{ Rewritten code start --------------------------------------------------
	//parse CLI
	LirenaConfig* configPtr = new LirenaConfig(argc, argv);
	
	// init GUI and threading stuff ASAP, depending on config
	LirenaCaptureUI::init(configPtr);

	// In  LirenaCaptureApp constructor:
	// - create LirenaStreamer instance (calls gst_init())
	// 		--> create capture device instance (e.g. open ximea camera),
    //			(but do not configure device yet)
	// - create LirenaCaptureUI instance
	//	 (but do not setup UI yet, i.e. do NOT create widgets, callbacks etc..)
	LirenaCaptureApp *appPtr = new LirenaCaptureApp(configPtr);


	bool success = appPtr->run();

	delete appPtr;
	appPtr = nullptr;

	// exit 0 on success
	// return success ? 0 : 1;
	//} Rewritten code end ----------------------------------------------------



	//-------------------------------------------------------------------------
	//old code:


	// have transformed until here ----	

	if(appPtr->configPtr->doLocalDisplay)
	{
		// TODO distribute this functionality into appropriate: classes
		// LirenaCaptureXimeaGUI, 
		// LirenaCaptureStreamer,
		// LirenaCaptureXimeaDevice 

		gboolean success = TRUE; 


		// open cam
		success &= lirenaXimeaCaptureDevice_openCam(appPtr);
		//init cam
		success &= lirenaXimeaCaptureDevice_setupCamParams(appPtr);

		// Create GUI
		success &= appPtr->uiPtr->setupUI();

		

		//start acquisition
		success &= appPtr->streamerPtr->launchCaptureThread(appPtr);


		appPtr->uiPtr->enterMainLoop();
		//lirenaCaptureXimeaGUI_enterMainLoop(appPtr->uiPtr);


	}
	else
	{
		// No-GUI- start-up:

		// TODO distribute this functionality into appropriate: classes
		// LirenaCaptureUI, 
		// LirenaCaptureStreamer,
		// LirenaCaptureXimeaDevice 


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


	//exit program:

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

	return success ? 0 : 1;
}

//----------------------------------------------------------------------------








