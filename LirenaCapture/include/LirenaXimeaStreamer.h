#ifndef __LIRENA_XIMEA_STREAMER_H__
#define __LIRENA_XIMEA_STREAMER_H__


#include "LirenaConfig.h"


#include <m3api/xiApi.h> //Ximea API

#include <gst/gst.h>


//----------------------------------------------------------------------------
// Macros
#define LIRENA_STATUS_STRING_LENGTH 4096




//struct LirenaCaptureApp;




//LirenaXimeaStreamer_Camera
struct LirenaXimeaStreamer_CameraParams
{
	HANDLE cameraHandle = INVALID_HANDLE_VALUE;

	//TODO put frame data here

	//cropping data, currently unused 
	//(test cam doesn't seem to support)
	int maxcx, maxcy, roix0, roiy0, roicx, roicy;

	// following GUI-related stuff 
	BOOLEAN acquire = TRUE;
	// dunno what ximea devs were thinking about this...
	BOOLEAN quitting = TRUE; 
	BOOLEAN doRender = TRUE;
};





// don't let RAM overflow and let latency explode unrecoverably,
// rather skip frames
// "2" works for 2k@140FPS, but there is some jam in the beginning.
// "6" works without jam for 2k@140FPS,
#define LIRENA_XIMEA_STREAMER_MAX_ENQUEUED_CUDA_DEMOSAIC_IMAGES 6
// for human-readable meta data formatting
#define LIRENA_XIMEA_STREAMER_MAX_STATUS_STRING_LENGTH 4096

struct LirenaXimeaStreamer_CudaFrameData
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






struct LirenaXimeaStreamer_FrameCaptureTimingData
{
    char statusString[LIRENA_STATUS_STRING_LENGTH];

    unsigned long current_time = 0;
    unsigned long prev_time = 0;

	unsigned long current_sensed_frame_count = 0;  
    unsigned long current_captured_frame_count = 0;
    unsigned long current_preprocessed_frame_count = 0;

	//derived values:
	unsigned long current_frame_duration = 0;
    unsigned long lost_frames_count = 0;
};

// dummy for later
struct LirenaXimeaStreamer_NaviMetaData
{
	int dummy_waitingForInputFromLightHouseGuys;
};

struct LirenaXimeaStreamer_FrameMetaData
{
	LirenaXimeaStreamer_FrameCaptureTimingData   timingData;
	LirenaXimeaStreamer_NaviMetaData             naviMeta;
};


struct LirenaXimeaStreamer_Frame
{
	LirenaXimeaStreamer_FrameMetaData metaData;

	LirenaXimeaStreamer_CudaFrameData cudaFrameData;

	//TODO put frame (ximea)data/(gst)buffer(s) in here

};




// Main "class" object:
struct LirenaXimeaStreamer
{
	LirenaConfig const * configPtr;

	LirenaXimeaStreamer_CameraParams camParams;
    pthread_t captureThread;


	// for asynchronous processing of CUDA stuff: has to be initialized
    //cv::cuda::Stream cudaDemoisaicStream;
    
    // minimalistic memory pool for GPU matrix objects, used by cuda stream
	LirenaXimeaStreamer_Frame framebufferPool[
		LIRENA_XIMEA_STREAMER_MAX_ENQUEUED_CUDA_DEMOSAIC_IMAGES
	];

	//TODO put gstreamer Element objects here
};













//-----------------------------------------------------------------------------
// function interface

LirenaXimeaStreamer * 
lirena_XimeaStreamer_create(
	LirenaConfig const * configPtr);

void 
lirena_XimeaStreamer_destroy(
	LirenaXimeaStreamer * streamer);

gboolean lirena_XimeaStreamer_initCam(LirenaXimeaStreamer *streamer);




// TODO rename & restructure function, 
// separate gui from capture&processing&streaming logic!
void* lirena_XimeaStreamer_captureThread_run(void* appVoidPtr);

struct LirenaCaptureApp;
void *lirena_XimeaStreamer_captureThread_terminate(LirenaCaptureApp *appPtr);




#endif //__LIRENA_XIMEA_STREAMER_H__