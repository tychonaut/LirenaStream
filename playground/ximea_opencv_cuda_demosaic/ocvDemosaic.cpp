
#include "LirenaConfig.h"

#include "LirenaHackStreamer.h"


#include <m3api/xiApi.h>

#include <iostream>

#include <opencv2/highgui.hpp>
#include <opencv2/cudaimgproc.hpp>
#include <opencv2/cudaarithm.hpp>
#include "opencv2/cudawarping.hpp" //cuda::resize
#include <cuda_runtime.h>

#include <gst/gst.h>

#include <sys/time.h>

 #include <unistd.h> // usleep

//#include <set>


//{Hack: too little time to use CLI args ...

//set to 0 to get rid of overhead of setting up the GUI/showing the image
//#define DO_SHOW_IMAGE 0
#define DO_USE_SENSOR_DECIMATION 0
#define DO_USE_CROP_INSTEAD_RESIZE 1
// add a GPU->CPU download to simulate non-nvivafilter-gstreamer interface
#define SHOW_CPUMAT_INSTEAD_GPUMAT 1

//#define DO_WHITE_BALANCE 0

// Define parameters for a static white balance
#define WB_BLUE 2
#define WB_GREEN 1
#define WB_RED 1.3

#if DO_USE_SENSOR_DECIMATION
# define decimationMultiplierToSet 2
#else
# define decimationMultiplierToSet 1
#endif

#define targetScaledResX 3840
#define targetScaledResY 2160




//}





// Define the number of images acquired, processed and shown in this example
//#define NUMBER_OF_IMAGES 10000

// don't let RAM overflow and let latency explode unrecoverably,
// rather skip frames
// "2" works for 2k@140FPS, but there is some jam in the beginning.
// "6" works without jam for 2k@140FPS,
#define MAX_ENQUEUED_CUDA_DEMOSAIC_IMAGES 12
#define STATUS_STRING_LENGTH 4096



using namespace cv;
using namespace std;

struct CudaFrameData;
struct HackApplicationState;



//-----------------------------------------------------------------------------
//typedef void(* 	StreamCallback) (int status, void *userData)

int main(int argc, char **argv);
HackApplicationState* initApp(int argc, char **argv);
bool lirena_setCamParams(HackApplicationState * appState);
bool lirena_setCamDownsamplingParams(HANDLE xiH);
bool lirena_setupCudaState(HackApplicationState *appState);
bool runAcquisitionLoop(HackApplicationState * appState);
void cudaDemosaicStreamCallback(int status, void *userData);

//-----------------------------------------------------------------------------



//-----------------------------------------------------------------------------
// init to zeros/false via memset
struct HackApplicationState
{
    HackApplicationState(int argc, char **argv)
      : config(argc, argv),
        streamer(&config)
    {}

    LirenaConfig config;

    LirenaHackStreamer streamer;

    XI_IMG xiImage;
    HANDLE xiH = NULL;
    int OCVbayerPattern = 0;

    int ximeaImageWidth = -1;
    int ximeaImageHeight = -1;

    bool doDisplayProcessedImage = false; //yes, not showing anything is the default
        
    // for asynchronous processing of CUDA stuff: has to be initialized
    cv::cuda::Stream cudaDemoisaicStream;
    
    // minimalistic memory pool for GPU matrix objects, used by cuda stream
    CudaFrameData* cudaFrameDataArray; //[MAX_ENQUEUED_CUDA_DEMOSAIC_IMAGES];
    
    

    //{ timing stuff
    unsigned long current_time = 0;
    unsigned long prev_time = 0;
    
    char status_string[STATUS_STRING_LENGTH];
      
    unsigned long current_captured_frame_count = 0;
    unsigned long prev_captured_frame_count = 0;
    unsigned long last_camera_frame_count = 0;
    unsigned long lost_frames_count = 0;
    
    unsigned long cuda_processed_frame_count = 0;
    //}
};

struct CudaFrameData
{
    CudaFrameData(int camImgHeight = -1, int camImgWidth = -1, 
      LirenaConfig* configPtr = nullptr)
    : bufferIndex(0),
      slotIsUsed(false),
      cudaHostMemoryForXiImage(nullptr),
      hostRawMat(),
      //hostRawMatInputArray()
      cropRectangle()
    {
      setupCropRect(camImgHeight, camImgWidth, configPtr);
    }


    bool setupCropRect(int camImgHeight = -1, int camImgWidth = -1, 
      LirenaConfig* configPtr = nullptr)
    {
      if(configPtr != nullptr && camImgHeight >0 &&  camImgWidth > 0)
      {
        int cropRectH = configPtr->targetResolutionY > 0 
                ? 
                configPtr->targetResolutionY
                : camImgHeight;

        int cropRectW = configPtr->targetResolutionX > 0 
                ? 
                configPtr->targetResolutionX
                : camImgWidth;

        int rectY = ( camImgHeight - cropRectH ) / 2;
        int rectX = ( camImgWidth - cropRectW ) / 2;

        cropRectangle = cv::Rect(rectX, rectY , cropRectW, cropRectH);
      }
      return true;
    }


    HackApplicationState* appStatePtr;

    int bufferIndex; // backtracking into HackApplicationState::cudaFrameDataArray
    // Status flag: Is data slot used by cuda right now?
    bool slotIsUsed; 
    
    //pointer to CUDA host data where the Ximea captured frame is mapped to
    void* cudaHostMemoryForXiImage; 
    // wrappers for cudaHostMemoryForXiImage for async GPU upload
    cv::Mat hostRawMat;
    //cv::InputArray hostRawMatInputArray;
    
    cuda::GpuMat gpuRawMatrix;   // OpenCV+Cuda-representation of XIMEA image

    //{ cropping experiments:
    cv::Rect cropRectangle;
    # if DO_USE_CROP_INSTEAD_RESIZE 
    cuda::GpuMat gpuCroppedRawMatrixRef;  // OpenCV+Cuda-representation of XIMEA image
    cuda::GpuMat gpuCroppedColorMatrix; // debayered result image

    // hack: read back from gpu to pass to gstreamer without hassle
    // (cause despite intense search I find no workaround-free way
    // to pass a GPU mat to a GstMemory;
    // workarounds are using stuff like EGL frames and interopping is with cv::GPUMat;
    //  no time for shis right now, unfortunately)
    
    // also self-alloc result matrix' CPU memory, so that it doesn't get destroyed
    // when its containing cpuCroppedColorMatrix gets out of scope!
    // we need this, because the memory will be wrpped by gstreamer memory and buffer,
    // and has to stay alive until gstreamer dosn't need it anymore.
    // TODO check how to create and manage GstBufferPool to omit permanent new allocations!
    gchar * cpuCroppedColorMatrix_selfAllocedHeapMem = nullptr;
    cv::Mat cpuCroppedColorMatrix;

    //} 
    # else
    //{ resize (resample) workflow
    cuda::GpuMat gpuColorMatrix; // debayered result image
    cuda::GpuMat gpuResizedColorMatrix; // debayered and resized result image

    cv::Mat cpuResizedColorMatrix;
    //}
    # endif

};


//-----------------------------------------------------------------------------



//microseconds
inline unsigned long getcurus() {
	struct timeval now;
	gettimeofday(&now, NULL);
	return now.tv_sec * 1000000 + now.tv_usec;
}





//-----------------------------------------------------------------------------
//typedef void(* 	StreamCallback) (int status, void *userData)
void cudaDemosaicStreamCallback(int status, void *userData)
{
    if(status != cudaSuccess)
    {
        printf("%s","Error in Cuda stream! Aborting");
        exit(1);
    }
    
    CudaFrameData * currentCudaFrameData = 
      static_cast<CudaFrameData *>(userData);
     
    
    currentCudaFrameData->appStatePtr->cuda_processed_frame_count ++;
    

    //free slot for reusal:
    currentCudaFrameData->slotIsUsed=false;



    //TODO push to gst via appsrc
}











//-----------------------------------------------------------------------------
int main(int argc, char **argv)
{
  // Simplyfied error handling (just for demonstration)
  try
  {
    gst_init(&argc, &argv);

    // init app state:
    HackApplicationState*  appState =  initApp(argc, argv);

    XI_RETURN xiStatus = XI_OK;
    
    lirena_setupCudaState(appState);



    // Start the image acquisition
    xiStatus = xiStartAcquisition(appState->xiH);
    if (xiStatus != XI_OK)
    {
      throw "Starting image acquisition failed";
    }
    

    if(appState->config.doLocalDisplay)
    {
        // Create a GUI window with OpenGL support
        namedWindow("XIMEA camera", WINDOW_OPENGL);
        // OpenGL window makes problems!
        //namedWindow("XIMEA camera");
        resizeWindow("XIMEA camera", 
          appState->ximeaImageWidth/4, 
          appState->ximeaImageHeight/4);
        //resizeWindow("XIMEA camera", 1600 , 1000);
    }
 
    // Define pointer used for (CUDA) data on GPU
    //void *imageGpu;




    runAcquisitionLoop(appState);
  

    
    // Stop image acquitsition and close device
    xiStopAcquisition(appState->xiH);
    xiCloseDevice(appState->xiH); 

    delete appState;
     
    // Print errors
   }catch(const char* message)
   {
    std::cerr << message << std::endl;
   }

}




//return value had to be delete'd
HackApplicationState* initApp(int argc, char **argv)
{
  
    HackApplicationState*  appState = new HackApplicationState(argc, argv);
    
    XI_RETURN xiStatus = XI_OK;


    // Initialize XI_IMG structure

    memset(&appState->xiImage, 0, sizeof(XI_IMG));
    appState->xiImage.size = sizeof(XI_IMG);

    // Get device handle for the camera
    xiStatus =  xiOpenDevice(0, &appState->xiH);
    if (xiStatus != XI_OK)
      throw "Opening device failed";

    bool success = lirena_setCamParams(appState);
    g_assert(success);

    return appState;
}








bool runAcquisitionLoop(HackApplicationState * appState)
{
    appState->prev_time = getcurus();

    XI_RETURN xiStatus = XI_OK;
  
    // Acquire a number of images, process and render them
    //for (int i = 0; i < NUMBER_OF_IMAGES; i++)
    for(;;) // run indefinitely
    {
    
      // Get host-pointer to image data
      xiStatus = xiGetImage(appState->xiH, 5000, &appState->xiImage);
      if (xiStatus != XI_OK)
      {
        throw "Getting image from camera failed";
      }
 
      //{ trying to parallelize cam image acquisition and cuda processing:
      int currentGpuMatIndex = 0;
      while(currentGpuMatIndex < MAX_ENQUEUED_CUDA_DEMOSAIC_IMAGES)
      {
        // is slot already used?
        if(appState->cudaFrameDataArray[currentGpuMatIndex].slotIsUsed)
        {
            currentGpuMatIndex++;
        }
        else
        {
          // set slot to "used" and stop checking the rest
          appState->cudaFrameDataArray[currentGpuMatIndex].slotIsUsed = 
            true;
          break;
        }
      }
      // all slots used?
      if(currentGpuMatIndex >= MAX_ENQUEUED_CUDA_DEMOSAIC_IMAGES)
      {
        printf("Cuda queue is full! "
                 "Skipping processing of acquired frame "
                 "and sleeping for 1ms!\n");
        // sleep for 1ms=1000microseconds; TODO handle this via mutexes etc.
        usleep(1000);
        // skip this execution
        continue;
      }




      CudaFrameData * currentCudaFrameData = 
        & (appState->cudaFrameDataArray[currentGpuMatIndex]);

      currentCudaFrameData->bufferIndex = currentGpuMatIndex;
      currentCudaFrameData->slotIsUsed = true;
        

      //{ Blocking impl:
      /*
      // Convert Ximea image to cuda device pointer
      cudaHostGetDevicePointer(
        &currentCudaFrameData->cudaHostMemoryForXiImage, appState->xiImage.bp, 0);
      // Create GpuMat from the cuda device pointer
      currentCudaFrameData->gpuRawMatrix =
        cuda::GpuMat(height, width, CV_8UC1, &currentCudaFrameData->cudaHostMemoryForXiImage); 
      // Create GpuMat for the result image
      currentCudaFrameData->gpuColorMatrix =
        cuda::GpuMat(height, width, CV_8UC3); 
      //}
      */
      
      //{ Non-Blocking impl:
      cudaHostGetDevicePointer(
        &currentCudaFrameData->cudaHostMemoryForXiImage, appState->xiImage.bp, 0);
      //CudaMem hostSrc(height, width, CV_8UC1, CudaMem::ALLOC_PAGE_LOCKED);

      // upload data from host: 
      // wrapper arond host mem:   
      const int	sizes[] = {appState->ximeaImageHeight, appState->ximeaImageWidth};
      currentCudaFrameData->hostRawMat =
        cv::Mat(
          2, //int 	ndims,
          sizes, //const int * 	sizes,
          CV_8UC1, //int 	type,
          currentCudaFrameData->cudaHostMemoryForXiImage, // void * 	data,
          0 //const size_t * 	steps = 0 
        ); 
      currentCudaFrameData->gpuRawMatrix.upload(
        cv::InputArray(currentCudaFrameData->hostRawMat),
        appState->cudaDemoisaicStream);
      //}



      #if DO_USE_CROP_INSTEAD_RESIZE

      //crop
      currentCudaFrameData->gpuCroppedRawMatrixRef =
          cuda::GpuMat( currentCudaFrameData->gpuRawMatrix,
              currentCudaFrameData->cropRectangle);

      cuda::demosaicing(
        currentCudaFrameData->gpuCroppedRawMatrixRef, 
        currentCudaFrameData->gpuCroppedColorMatrix, 
        appState->OCVbayerPattern, 
        0, // derive channel layout from other params
        appState->cudaDemoisaicStream 
      );

      #if SHOW_CPUMAT_INSTEAD_GPUMAT 

        //unfortunate hack to get easy-gst-compatible memory: download gpu to cpu:
        currentCudaFrameData->gpuCroppedColorMatrix.download(
          //cv::OutputArray(currentCudaFrameData->cpuResizedColorMatrix),
          currentCudaFrameData->cpuCroppedColorMatrix,
          appState->cudaDemoisaicStream);

      #endif // SHOW_CPUMAT_INSTEAD_GPUMAT 

    #else

      //currentCudaFrameData->gpuRawMatrix.adjustROI


   
      // Demosaic raw bayer image to color image
      cuda::demosaicing(
        currentCudaFrameData->gpuRawMatrix, 
        currentCudaFrameData->gpuColorMatrix, 
        appState->OCVbayerPattern, 
        0, // derive channel layout from other params
        appState->cudaDemoisaicStream 
      );
     
      //  all allways 1 even with auto wb enabled 0o
      // printf("white balance params from ximea: %f, %f, %f\n",
      //   image.wb_red, image.wb_green, image.wb_blue);
        
      //Apply static white balance by multiplying the channels

      //no WB (after coordination with Marco)
      // if(DO_WHITE_BALANCE)
      // {
      //   cuda::multiply(
      //     currentCudaFrameData->gpuColorMatrix, 
      //     cv::Scalar(WB_BLUE, WB_GREEN, WB_RED), 
      //     //cv::Scalar(image.wb_red, image.wb_green, image.wb_blue), 
      //     currentCudaFrameData->gpuColorMatrix,
      //     1.0, 
      //     -1,
      //     appState->cudaDemoisaicStream
      //   );
      // }


      
      //resize(InputArray src, OutputArray dst, Size dsize, double fx=0, double fy=0, int interpolation = INTER_LINEAR, Stream& stream = Stream::Null());
      cuda::resize(
        currentCudaFrameData->gpuColorMatrix,
        currentCudaFrameData->gpuResizedColorMatrix,
        Size( 
          appState->config.targetResolutionY > 0 
            ? appState->config.targetResolutionY
            : height,
          appState->config.targetResolutionX > 0 
            ? appState->config.targetResolutionX
            : width
          ),
          0, 
          0, 
          INTER_LINEAR,
          appState->cudaDemoisaicStream
      );
      
      #if SHOW_CPUMAT_INSTEAD_GPUMAT 

        //unfortunate hack to get easy-gst-compatible memory: download gpu to cpu:
        currentCudaFrameData->gpuResizedColorMatrix.download(
          //cv::OutputArray(currentCudaFrameData->cpuResizedColorMatrix),
          currentCudaFrameData->cpuResizedColorMatrix,
          appState->cudaDemoisaicStream);

      #endif // SHOW_CPUMAT_INSTEAD_GPUMAT 

      #endif

        appState->cudaDemoisaicStream.enqueueHostCallback(
            cudaDemosaicStreamCallback, // StreamCallback 	callback,
            currentCudaFrameData        // void * 	userData
        );

        //{ FPS calcs: ----------------------------------------------------------
        appState->current_captured_frame_count++;
        if (appState->xiImage.nframe > appState->last_camera_frame_count)
        {
          appState->lost_frames_count += appState->xiImage.nframe - (appState->last_camera_frame_count + 1);
	  } 
	  appState->last_camera_frame_count = appState->xiImage.nframe;
      appState->current_time = getcurus();
      //update around each second:
	  if(appState->current_time - appState->prev_time > 1000000) 
	  {
			snprintf(
			    appState->status_string, 
			    STATUS_STRING_LENGTH, 
			    "Acquisition [ Acquired: %lu, processed: %lu, skipped: %lu, fps: %.2f ]\n", 
			    appState->current_captured_frame_count, 
			    appState->cuda_processed_frame_count,
			    appState->lost_frames_count, 
			    1000000.0 * 
			      (appState->current_captured_frame_count - appState->prev_captured_frame_count) 
			     / (appState->current_time - appState->prev_time)
			);
			printf("%s",appState->status_string);
			
			appState->prev_captured_frame_count = appState->current_captured_frame_count;
			appState->prev_time = appState->current_time;  
	  }
      //}  --------------------------------------------------------------------



     if(appState->config.doLocalDisplay)
     {   
        // Render image to the screen (using OpenGL) 
        //imshow("XIMEA camera", currentCudaFrameData->gpuColorMatrix);
        
        //only show every nth frame:
        //if(appState->cuda_processed_frame_count % 3 == 0)
        {
            //block for show:
            appState->cudaDemoisaicStream.waitForCompletion();
            
            #if SHOW_CPUMAT_INSTEAD_GPUMAT 
      
              imshow("XIMEA camera", 
                //currentCudaFrameData->gpuColorMatrix
                #if DO_USE_CROP_INSTEAD_RESIZE
                  currentCudaFrameData->cpuCroppedColorMatrix
                #else // DO_USE_CROP_INSTEAD_RESIZE
                  currentCudaFrameData->cpuResizedColorMatrix
                #endif // DO_USE_CROP_INSTEAD_RESIZE
              );
            
            #else // SHOW_CPUMAT_INSTEAD_GPUMAT

              imshow("XIMEA camera", 
                //currentCudaFrameData->gpuColorMatrix
                #if DO_USE_CROP_INSTEAD_RESIZE
                currentCudaFrameData->gpuCroppedColorMatrix
                #else // DO_USE_CROP_INSTEAD_RESIZE
                currentCudaFrameData->gpuResizedColorMatrix
                #endif // DO_USE_CROP_INSTEAD_RESIZE
              );

            #endif // SHOW_CPUMAT_INSTEAD_GPUMAT 
       }
     }


      
      // prevent window gray-out
      waitKey(1);
    }  


    return xiStatus == XI_OK;
}


bool lirena_setCamDownsamplingParams(HANDLE xiH)
{

  XI_RETURN xiStatus = XI_OK;

  //"decimation"
  int decimation_selector = 0;
  xiGetParamInt(xiH, XI_PRM_DECIMATION_SELECTOR, &decimation_selector);
  // is 0 (XI_DEC_SELECT_SENSOR), despite api saying 1
  printf("previous XI_PRM_DECIMATION_SELECTOR: %d\n", decimation_selector);
  xiStatus = xiSetParamInt(xiH, XI_PRM_DECIMATION_SELECTOR,
                           XI_DEC_SELECT_SENSOR // no error, but not works
                           //XI_BIN_SELECT_DEVICE_FPGA  //<-- returns XI_PARAM_NOT_SETTABLE             =114
                           //XI_BIN_SELECT_HOST_CPU
  );
  if (xiStatus != XI_OK)
  {
    printf(" XI_PRM_DECIMATION_SELECTOR, XI_DEC_SELECT_SENSOR: return value not XI_OK: %d\n", xiStatus);
    sleep(4);
  }

  int decimation_pattern = 0;
  int decimation_multiplier = 0;

  xiGetParamInt(xiH, XI_PRM_DECIMATION_VERTICAL_PATTERN, &decimation_pattern);
  printf("previous XI_PRM_DECIMATION_VERTICAL_PATTERN: %d\n", decimation_pattern);
  xiStatus = xiSetParamInt(xiH, XI_PRM_DECIMATION_VERTICAL_PATTERN, XI_DEC_BAYER);
  if (xiStatus != XI_OK)
  {
    printf(" XI_PRM_DECIMATION_VERTICAL_PATTERN, XI_DEC_BAYER: return value not XI_OK: %d", xiStatus);
    sleep(4);
  }
  xiGetParamInt(xiH, XI_PRM_DECIMATION_VERTICAL, &decimation_multiplier);
  printf("previous XI_PRM_DECIMATION_VERTICAL multiplier: %d\n", decimation_multiplier);
  xiSetParamInt(xiH, XI_PRM_DECIMATION_VERTICAL, decimationMultiplierToSet);
  if (xiStatus != XI_OK)
  {
    printf(" XI_PRM_DECIMATION_VERTICAL := %d: return value not XI_OK: %d", decimationMultiplierToSet, xiStatus);
    sleep(4);
  }

  xiGetParamInt(xiH, XI_PRM_DECIMATION_HORIZONTAL_PATTERN, &decimation_pattern);
  printf("previous XI_PRM_DECIMATION_HORIZONTAL_PATTERN: %d\n", decimation_pattern);
  xiStatus = xiSetParamInt(xiH, XI_PRM_DECIMATION_HORIZONTAL_PATTERN, XI_DEC_BAYER);
  if (xiStatus != XI_OK)
  {
    printf(" XI_PRM_DECIMATION_HORIZONTAL_PATTERN, XI_DEC_BAYER: return value not XI_OK: %d", xiStatus);
    sleep(4);
  }
  xiGetParamInt(xiH, XI_PRM_DECIMATION_HORIZONTAL, &decimation_multiplier);
  printf("previous XI_PRM_DECIMATION_HORIZONTAL multiplier: %d\n", decimation_multiplier);
  xiSetParamInt(xiH, XI_PRM_DECIMATION_HORIZONTAL, decimationMultiplierToSet);
  if (xiStatus != XI_OK)
  {
    printf(" XI_PRM_DECIMATION_HORIZONTAL := %d: return value not XI_OK: %d", decimationMultiplierToSet, xiStatus);
    sleep(4);
  }

  return xiStatus == XI_OK;
}

bool lirena_setCamParams(HackApplicationState *appState)
{
  XI_RETURN xiStatus = XI_OK;

  bool downsamplingOK = lirena_setCamDownsamplingParams(appState->xiH);
  g_assert(downsamplingOK);

  // Get type of camera color filter
  int cfa = 0;
  xiStatus = xiGetParamInt(appState->xiH, XI_PRM_COLOR_FILTER_ARRAY, &cfa);
  if (xiStatus != XI_OK)
    throw "Could not get color filter array from camera";

  // Set correct demosaicing type according to camera color filter
  switch (cfa)
  {
  case XI_CFA_BAYER_RGGB:
  {
    cout << "BAYER_RGGB color filter." << endl;
    appState->OCVbayerPattern = COLOR_BayerRG2BGR;
    break;
  }
  case XI_CFA_BAYER_BGGR:
  {
    cout << "BAYER_BGGR color filter." << endl;
    appState->OCVbayerPattern = COLOR_BayerBG2BGR;
    break;
  }
  case XI_CFA_BAYER_GRBG:
  {
    cout << "BAYER_GRBG color filter." << endl;
    appState->OCVbayerPattern = COLOR_BayerGR2BGR;
    break;
  }
  case XI_CFA_BAYER_GBRG:
  {
    cout << "BAYER_GBRG color filter." << endl;
    appState->OCVbayerPattern = COLOR_BayerGB2BGR;
    break;
  }
  default:
  {
    throw "Not supported color filter for demosaicing.";
  }
  }

  // Use transport data format (no processing done by the API)
  xiStatus = xiSetParamInt(appState->xiH, XI_PRM_IMAGE_DATA_FORMAT, XI_FRM_TRANSPORT_DATA);
  if (xiStatus != XI_OK)
    throw "Setting image data format failed";

  // Make data from the camera stream to zerocopy memory
  xiStatus = xiSetParamInt(appState->xiH, XI_PRM_TRANSPORT_DATA_TARGET, XI_TRANSPORT_DATA_TARGET_ZEROCOPY);
  if (xiStatus != XI_OK)
    throw "Setting transport data target failed";

  // Using 8-bit images here
  xiStatus = xiSetParamInt(appState->xiH, XI_PRM_OUTPUT_DATA_BIT_DEPTH, 8);
  if (xiStatus != XI_OK)
    throw "Setting bit depth failed";

  // Exposure 7 ms --> more than 120 fps (if no other bottleneck is there)
  xiStatus = xiSetParamInt(appState->xiH, XI_PRM_EXPOSURE, 7000);
  if (xiStatus != XI_OK)
    throw "Setting exposure failed";

  // gain
  float mingain, maxgain;
  xiGetParamFloat(appState->xiH, XI_PRM_GAIN XI_PRM_INFO_MIN, &mingain);
  xiGetParamFloat(appState->xiH, XI_PRM_GAIN XI_PRM_INFO_MAX, &maxgain);

  float mygain = mingain + (maxgain - mingain) * 1.0f;
  xiSetParamFloat(appState->xiH, XI_PRM_GAIN, mygain);

  // do auto whitebalance -> sems not applied to image,
  // but the three multipliers are in aquried image struct (?)
  xiStatus = xiSetParamInt(appState->xiH, XI_PRM_AUTO_WB,
                       //0
                       XI_ON);
  if (xiStatus != XI_OK)
    throw "Setting auto white balance failed";

  // xiSetParamFloat(appState->xiH, XI_PRM_WB_KR, WB_RED);
  // xiSetParamFloat(appState->xiH, XI_PRM_WB_KG, WB_GREEN);
  // xiSetParamFloat(appState->xiH, XI_PRM_WB_KB, WB_BLUE);

  // Get width of image

  xiStatus = xiGetParamInt(appState->xiH, XI_PRM_WIDTH, &appState->ximeaImageWidth);
  if (xiStatus != XI_OK)
    throw "Could not get image width from camera";

  // Get height of image
  xiStatus = xiGetParamInt(appState->xiH, XI_PRM_HEIGHT, &appState->ximeaImageHeight);
  if (xiStatus != XI_OK)
    throw "Could not get image height from camera";

  return xiStatus == XI_OK;
}



bool lirena_setupCudaState(HackApplicationState *appState)
{
  appState->cudaDemoisaicStream = cv::cuda::Stream();

  appState->cudaFrameDataArray =
      new CudaFrameData[MAX_ENQUEUED_CUDA_DEMOSAIC_IMAGES];

  // init GpuMats in case they are repurposed later without reinit:
  for (int i = 0; i < MAX_ENQUEUED_CUDA_DEMOSAIC_IMAGES; i++)
  {
    CudaFrameData *currentCudaFrameData =
        &(appState->cudaFrameDataArray[i]);

    currentCudaFrameData->appStatePtr = appState;

    currentCudaFrameData->setupCropRect(
        appState->ximeaImageHeight,
        appState->ximeaImageWidth,
        &appState->config);

    currentCudaFrameData->gpuRawMatrix =
        cuda::GpuMat(
            appState->ximeaImageHeight,
            appState->ximeaImageWidth,
            CV_8UC1);

#if DO_USE_CROP_INSTEAD_RESIZE

    currentCudaFrameData->gpuCroppedRawMatrixRef =
        cuda::GpuMat(
            currentCudaFrameData->cropRectangle.height,
            currentCudaFrameData->cropRectangle.width,
            CV_8UC1);

    currentCudaFrameData->gpuCroppedColorMatrix =
        cuda::GpuMat(
            currentCudaFrameData->cropRectangle.height,
            currentCudaFrameData->cropRectangle.width,
            CV_8UC3);

    currentCudaFrameData->cpuCroppedColorMatrix_selfAllocedHeapMem =
        (gchar *)g_malloc(
            currentCudaFrameData->cropRectangle.height * currentCudaFrameData->cropRectangle.width * 4 //RGBx = four bytes
        );

    currentCudaFrameData->cpuCroppedColorMatrix =
        cv::Mat(
            currentCudaFrameData->cropRectangle.height,
            currentCudaFrameData->cropRectangle.width,
            //CV_8UC3
            // need to be 32bit RGBx/GBRx/xRGB for nvvidconv
            CV_8UC4,
            //provide self-managed memory
            currentCudaFrameData->cpuCroppedColorMatrix_selfAllocedHeapMem);

#else

    currentCudaFrameData->gpuColorMatrix =
        cuda::GpuMat(height, width, CV_8UC3);

    currentCudaFrameData->gpuResizedColorMatrix =
        cuda::GpuMat(
            appState->config.targetResolutionY > 0
                ? appState->config.targetResolutionY
                : height,
            appState->config.targetResolutionX > 0
                ? appState->config.targetResolutionX
                : width,
            CV_8UC3);

    currentCudaFrameData->cpuResizedColorMatrix =
        cv::Mat(
            appState->config.targetResolutionY > 0
                ? appState->config.targetResolutionY
                : height,
            appState->config.targetResolutionX > 0
                ? appState->config.targetResolutionX
                : width,
            CV_8UC3);

#endif
  }
  printf("SENSOR resolution to be fed into cv::cuda preprocessing:      %dx%d\n",
         appState->ximeaImageWidth, appState->ximeaImageHeight);
  printf("GPU-cropped resolution to be fed into GStreamer: %dx%d\n",
         appState->cudaFrameDataArray[0].cropRectangle.width,
         appState->cudaFrameDataArray[0].cropRectangle.height);

  //}


  //TODO do cuda error checking
  return true;


}








