#ifndef __LIRENA_XIMEA_STREAMER_H__
#define __LIRENA_XIMEA_STREAMER_H__




#include <m3api/xiApi.h> //Ximea API




//----------------------------------------------------------------------------
// Macros
#define LIRENA_STATUS_STRING_LENGTH 4096




struct LirenaCaptureApp;





struct LirenaCamera
{
	HANDLE cameraHandle = INVALID_HANDLE_VALUE;

	BOOLEAN acquire = TRUE;
	BOOLEAN quitting = TRUE;
	BOOLEAN doRender = TRUE;

	int maxcx, maxcy, roix0, roiy0, roicx, roicy;
};

struct LirenaFrameMetaData
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


// TODO rename & restructure function, 
// separate gui from capture&processing&streaming logic!
void* videoDisplay(void* appVoidPtr);
void *videoThread_shutdown(LirenaCaptureApp *appPtr);




#endif //__LIRENA_XIMEA_STREAMER_H__