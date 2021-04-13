#ifndef __LIRENA_XIMEA_STREAMER_H__
#define __LIRENA_XIMEA_STREAMER_H__


#include "LirenaConfig.h"
#include "LirenaCaptureDevice/LirenaCaptureDevice.h"


#include <m3api/xiApi.h> //Ximea API

#include <gst/gst.h>


/*
	TODO:
	 	- rename LirenaXimeaStreamer struct and files to LirenaStreamer,
		  Use LirenaCaptureDevice as members for internal differentiation
		- ditch c-style OO-interface; I wanted to practice 
		  "GLib-style pure-C OO programming", to be better prepared for 
		  gst plugin development, but the boilerplate 
		  becomes more and more annoying, and time pressure enforces 
		  quick results over future synergies
*/




//----------------------------------------------------------------------------
// Macros
#define LIRENA_STATUS_STRING_MAX_LENGTH 4096



// during refactoring for compatibility reasons;
// TODO delete
class LirenaCaptureApp;




//LirenaXimeaStreamer_Camera
struct LirenaXimeaStreamer_CameraParams
{
	HANDLE cameraHandle = INVALID_HANDLE_VALUE;

	//TODO put frame data here

	//cropping data, currently unused 
	//(test cam doesn't seem to support)
	int maxcx = 0;
	int maxcy = 0;

	int roix0 = 0;
	int roiy0 = 0;

	int roicx = 0;
	int roicy = 0;

	float mingain = 0.0f;
	float maxgain = 0.0f;
	
	int isColor = 0;

	// milliseconds;
	// read from config
	int exposureToUse_ms = 30;

	// intemediate derived value
	// TODO make CLI-configurable
	float gainToUse = 0.0f;

	// https://www.ximea.com/support/wiki/apis/xiapi_manual#XI_PRM_GPI_SELECTOR-or-gpi_selector
	// Some kind of general purpose input state:
	// don't really get it yet, was 0 for GPI selector 1-4 on test cam, threw error on
	// selector 4, hinting at "0-3". Awkward, useless so far, but keep in mind that
	// with new cam and active synchronized triggering, this may become very important!
	int gpi_levels[LIRENA_XIMEA_MAX_GPI_SELECTORS] = {0, 0, 0, 0};
};





// don't let RAM overflow and let latency explode unrecoverably,
// rather skip frames
// "2" works for 2k@140FPS, but there is some jam in the beginning.
// "6" works without jam for 2k@140FPS,
#define LIRENA_STREAMER_MAX_ENQUEUED_CUDA_DEMOSAIC_IMAGES 6


struct LirenaStreamer_CudaFrameData
{
	int TODO_dummyTillLinkage;


    // CudaFrameData()
    // : bufferIndex(0),
    //   slotIsUsed(false),
    //   cudaHostMemory(nullptr),
    //   hostRawMat()

    // {}

    // int bufferIndex; // backtracking into ApplicationState::cudaFrameDataArray
    // // Status flag: Is data slot used by cuda right now?
    // bool slotIsUsed; 
    
    // //pointer to CUDA host data where the Ximea captured frame is mapped to
    // void* cudaHostMemory; 
    // // wrappers for cudaHostMemory for async GPU upload
    // cv::Mat hostRawMat;
    // //cv::InputArray hostRawMatInputArray;
    
    // cuda::GpuMat gpuRawMatrix;   // OpenCV+Cuda-representation of XIMEA image
    // cuda::GpuMat gpuColorMatrix; // debayered result image
};






struct LirenaFrameCaptureTimingData
{
    char statusString[LIRENA_STATUS_STRING_MAX_LENGTH];

	GstClockTime lastStreamStart_abs_time = 0;
    GstClockTime current_abs_time = 0;
    GstClockTime prev_abs_time = 0;
	// derived value:
	// do not use in gst buffer duration, 
	//makes problems,don't know why
	GstClockTime current_frame_duration = 0;

	unsigned long current_sensed_frame_count = 0;  
    unsigned long current_captured_frame_count = 0;
    //unsigned long current_preprocessed_frame_count = 0;

	// derived value:
    unsigned long lost_frames_count = 0;
};

// dummy for later
struct LirenaNaviMetaData
{
	int dummy_waitingForInputFromLightHouseGuys;
};


struct LirenaFrame
{
	struct MetaData
	{
		LirenaFrameCaptureTimingData   timingData;
		LirenaNaviMetaData             naviMeta;
	};

	MetaData metaData;

	LirenaStreamer_CudaFrameData cudaFrameData;

	//TODO put frame (ximea)data/(gst)buffer(s) in here

};




// Main "class" object:
class LirenaStreamer
{
	public:
		LirenaStreamer(LirenaConfig * configPtr);

		~LirenaStreamer();


		bool setupCaptureDevice();

		bool setupGStreamerPipeline();

		bool launchCaptureThread(
			LirenaCaptureApp * appPtr_refactor_compat_TODO_delete
		);
		bool terminateCaptureThread();


private: //TODO make most of rest private
public:
		LirenaConfig * configPtr = nullptr;

		//LirenaStreamer takes ownership of captureDevice
		LirenaCaptureDevice* captureDevicePtr = nullptr;

		//modified by GUI thread to shut down capture thread
		volatile BOOLEAN  doAcquireFrames = TRUE;

		pthread_t captureThread = 0;
		bool captureThreadIsRunning = false;

		


	//TODO put into appropriate LirenaCaptureDevice
		LirenaXimeaStreamer_CameraParams camParams; //camParams;


		// for asynchronous processing of CUDA stuff: has to be initialized
		//cv::cuda::Stream cudaDemoisaicStream;
		
		// minimalistic memory pool for GPU matrix objects, used by cuda stream
		LirenaFrame framebufferPool[
			LIRENA_STREAMER_MAX_ENQUEUED_CUDA_DEMOSAIC_IMAGES
		];
		int currentFrameBufferPoolIndex = 0;

		//TODO put gstreamer Element objects here
};













//-----------------------------------------------------------------------------
// function interface

// LirenaStreamer * 
// lirena_XimeaStreamer_create(
// 	LirenaConfig const * configPtr);

// void 
// lirena_XimeaStreamer_destroy(
// 	LirenaStreamer * streamer);




gboolean lirena_XimeaStreamer_openCamera(
	LirenaStreamer* streamer
);
gboolean lirena_XimeaStreamer_setupCamParams(
	LirenaXimeaStreamer_CameraParams* camParams,
	LirenaConfig const * config
);




// TODO rename & restructure function, 
// separate ui from capture&processing&streaming logic!
void* lirena_XimeaStreamer_captureThread_run(void* appVoidPtr);

struct LirenaCaptureApp;
void *lirena_XimeaStreamer_captureThread_terminate(LirenaCaptureApp *appPtr);




#endif //__LIRENA_XIMEA_STREAMER_H__