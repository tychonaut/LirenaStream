#include <m3api/xiApi.h>
#include <iostream>
#include <opencv2/highgui.hpp>
#include <opencv2/cudaimgproc.hpp>
#include <opencv2/cudaarithm.hpp>
#include <cuda_runtime.h>

#include <sys/time.h>

 #include <unistd.h> // usleep

//#include <set>


// Define the number of images acquired, processed and shown in this example
#define NUMBER_OF_IMAGES 10000


//set to 0 to get rid of overhead of setting up the GUI/showing the image
#define DO_SHOW_IMAGE 0
// don't let RAM overflow and let latency explode unrecoverably,
// rather skip frames
// "2" works for 2k@140FPS, but there is some jam in the beginning.
// "6" works without jam for 2k@140FPS,
#define MAX_ENQUEUED_CUDA_DEMOSAIC_IMAGES 12
#define STATUS_STRING_LENGTH 4096

// Define parameters for a static white balance
#define WB_BLUE 2
#define WB_GREEN 1
#define WB_RED 1.3

using namespace cv;
using namespace std;


struct CudaFrameData
{
    CudaFrameData()
    : bufferIndex(0),
      slotIsUsed(false),
      cudaHostMemory(nullptr),
      hostRawMat()
      //hostRawMatInputArray()
    {}

    int bufferIndex; // backtracking into ApplicationState::cudaFrameDataArray
    // Status flag: Is data slot used by cuda right now?
    bool slotIsUsed; 
    
    //pointer to CUDA host data where the Ximea captured frame is mapped to
    void* cudaHostMemory; 
    // wrappers for cudaHostMemory for async GPU upload
    cv::Mat hostRawMat;
    //cv::InputArray hostRawMatInputArray;
    
    cuda::GpuMat gpuRawMatrix;   // OpenCV+Cuda-representation of XIMEA image
    cuda::GpuMat gpuColorMatrix; // debayered result image
};

// init to zeros/false via memset
struct ApplicationState
{
    bool doDisplayProcessedImage; //yes, not showing anything is the default
        
    // for asynchronous processing of CUDA stuff: has to be initialized
    cv::cuda::Stream cudaDemoisaicStream;
    
    // minimalistic memory pool for GPU matrix objects, used by cuda stream
    CudaFrameData* cudaFrameDataArray; //[MAX_ENQUEUED_CUDA_DEMOSAIC_IMAGES];
    
    
    unsigned long current_time = 0;
    unsigned long prev_time = 0;
    
    char status_string[STATUS_STRING_LENGTH];
      
    unsigned long current_captured_frame_count = 0;
    unsigned long prev_captured_frame_count = 0;
    unsigned long last_camera_frame_count = 0;
    unsigned long lost_frames_count = 0;
    
    unsigned long cuda_processed_frame_count = 0;

};

ApplicationState globalAppState;




//microseconds
inline unsigned long getcurus() {
	struct timeval now;
	gettimeofday(&now, NULL);
	return now.tv_sec * 1000000 + now.tv_usec;
}




//typedef void(* 	StreamCallback) (int status, void *userData)
void cudaDemosaicStreamCallback(int status, void *userData)
{
    if(status != cudaSuccess)
    {
        printf("Error in Cuda stream! Aborting");
        exit(1);
    }
    
    CudaFrameData * currentCudaFrameData = 
      static_cast<CudaFrameData *>(userData);
     
    
    globalAppState.cuda_processed_frame_count ++;
    
    /*
    if(DO_SHOW_IMAGE)
    {   
        // Render image to the screen (using OpenGL) 
        //imshow("XIMEA camera", currentCudaFrameData->gpuColorMatrix);
        
        //only show every nth frame:
        if(globalAppState.cuda_processed_frame_count % 320 == 0)
        {
            //blocking copy
            cuda::GpuMat gpuColorMatrixToShow (
              currentCudaFrameData->gpuColorMatrix);
            
            imshow("XIMEA camera", gpuColorMatrixToShow);
       }
    }
    */

    //free slot for reusal:
    currentCudaFrameData->slotIsUsed=false;
}




bool setDownsamplingParams(HANDLE xiH)
{
		
		XI_RETURN xiStatus = XI_OK;


    //"decimation"
    int decimation_selector = 0;
    xiGetParamInt(xiH, XI_PRM_DECIMATION_SELECTOR, &decimation_selector);
    // is 0 (XI_DEC_SELECT_SENSOR), despite api saying 1  
    printf("previous XI_PRM_DECIMATION_SELECTOR: %d\n",decimation_selector); 
    xiStatus = xiSetParamInt(xiH, XI_PRM_DECIMATION_SELECTOR, 
      XI_DEC_SELECT_SENSOR  // no error, but not works
      //XI_BIN_SELECT_DEVICE_FPGA  //<-- returns XI_PARAM_NOT_SETTABLE             =114
      //XI_BIN_SELECT_HOST_CPU
    );
    if(xiStatus != XI_OK)
		{
			printf(" XI_PRM_DECIMATION_SELECTOR, XI_DEC_SELECT_SENSOR: return value not XI_OK: %d\n", xiStatus);
			sleep(4);
		}


    int decimation_pattern = 0;
    int decimation_multiplier = 0;


    xiGetParamInt(xiH, XI_PRM_DECIMATION_VERTICAL_PATTERN, &decimation_pattern);
    printf("previous XI_PRM_DECIMATION_VERTICAL_PATTERN: %d\n",decimation_pattern); 
    xiStatus = xiSetParamInt(xiH, XI_PRM_DECIMATION_VERTICAL_PATTERN, XI_DEC_BAYER);
    if(xiStatus != XI_OK)
		{
			printf(" XI_PRM_DECIMATION_VERTICAL_PATTERN, XI_DEC_BAYER: return value not XI_OK: %d", xiStatus);
			sleep(4);
		}
    xiGetParamInt(xiH, XI_PRM_DECIMATION_VERTICAL, &decimation_multiplier);
    printf("previous XI_PRM_DECIMATION_VERTICAL multiplier: %d\n",decimation_multiplier); 
    xiSetParamInt(xiH, XI_PRM_DECIMATION_VERTICAL, 4);
    if(xiStatus != XI_OK)
		{
			printf(" XI_PRM_DECIMATION_VERTICAL := 2: return value not XI_OK: %d", xiStatus);
			sleep(4);
		}



    xiGetParamInt(xiH, XI_PRM_DECIMATION_HORIZONTAL_PATTERN, &decimation_pattern);
    printf("previous XI_PRM_DECIMATION_HORIZONTAL_PATTERN: %d\n",decimation_pattern); 
    xiStatus = xiSetParamInt(xiH, XI_PRM_DECIMATION_HORIZONTAL_PATTERN, XI_DEC_BAYER);
    if(xiStatus != XI_OK)
		{
			printf(" XI_PRM_DECIMATION_HORIZONTAL_PATTERN, XI_DEC_BAYER: return value not XI_OK: %d", xiStatus);
			sleep(4);
		}
    xiGetParamInt(xiH, XI_PRM_DECIMATION_HORIZONTAL, &decimation_multiplier);
    printf("previous XI_PRM_DECIMATION_HORIZONTAL multiplier: %d\n",decimation_multiplier); 
    xiSetParamInt(xiH, XI_PRM_DECIMATION_HORIZONTAL, 2);
    if(xiStatus != XI_OK)
		{
			printf(" XI_PRM_DECIMATION_HORIZONTAL := 2: return value not XI_OK: %d", xiStatus);
			sleep(4);
		}


    return xiStatus == XI_OK;

}






int main()
{
  // Initialize XI_IMG structure
  XI_IMG image;
  memset(&image, 0, sizeof(XI_IMG));
  image.size = sizeof(XI_IMG);

  HANDLE xiH = NULL;
  XI_RETURN stat = XI_OK;

  // Simplyfied error handling (just for demonstration)
  try
  {
    int cfa = 0;
    int OCVbayer = 0;

    // Get device handle for the camera
    stat =  xiOpenDevice(0, &xiH);
    if (stat != XI_OK)
      throw "Opening device failed";



    setDownsamplingParams(xiH);


	  // Get type of camera color filter
    stat = xiGetParamInt(xiH, XI_PRM_COLOR_FILTER_ARRAY, &cfa);
    if (stat != XI_OK)
      throw "Could not get color filter array from camera";

	  // Set correct demosaicing type according to camera color filter
    switch (cfa) {
    case XI_CFA_BAYER_RGGB:
    {
      cout << "BAYER_RGGB color filter." << endl;
      OCVbayer = COLOR_BayerRG2BGR;
      break;
    }
    case XI_CFA_BAYER_BGGR:
    {
      cout << "BAYER_BGGR color filter." << endl;
      OCVbayer = COLOR_BayerBG2BGR;
      break;
    }
    case XI_CFA_BAYER_GRBG:
    {
      cout<<"BAYER_GRBG color filter." << endl;
      OCVbayer = COLOR_BayerGR2BGR;
      break;
    }
    case XI_CFA_BAYER_GBRG:
    {
      cout<<"BAYER_GBRG color filter." << endl;
      OCVbayer = COLOR_BayerGB2BGR;
      break;
    }
    default:
    {
      throw "Not supported color filter for demosaicing.";
    }
    }
    
    // Use transport data format (no processing done by the API)
    stat = xiSetParamInt(xiH, XI_PRM_IMAGE_DATA_FORMAT, XI_FRM_TRANSPORT_DATA);
    if (stat != XI_OK)
      throw "Setting image data format failed";
        
    // Make data from the camera stream to zerocopy memory
    stat = xiSetParamInt(xiH, XI_PRM_TRANSPORT_DATA_TARGET, XI_TRANSPORT_DATA_TARGET_ZEROCOPY);
    if (stat != XI_OK)
      throw "Setting transport data target failed";

    // Using 8-bit images here
    stat = xiSetParamInt(xiH, XI_PRM_OUTPUT_DATA_BIT_DEPTH, 8);
    if (stat != XI_OK)
      throw "Setting bit depth failed";
    
    
    // Exposure 7 ms --> more than 120 fps (if no other bottleneck is there)
    stat = xiSetParamInt(xiH, XI_PRM_EXPOSURE, 7000);
    if (stat != XI_OK)
      throw "Setting exposure failed";
      
    // gain
    float mingain, maxgain;	
    xiGetParamFloat(xiH, XI_PRM_GAIN XI_PRM_INFO_MIN, &mingain);
    xiGetParamFloat(xiH, XI_PRM_GAIN XI_PRM_INFO_MAX, &maxgain);
    float mygain = mingain +  (maxgain - mingain) * 1.0f;
    xiSetParamFloat(xiH, XI_PRM_GAIN, mygain);     
      


    // do auto whitebalance -> sems not applied to image, 
    // but the three multipliers are in aquried image struct (?)
    stat = xiSetParamInt(xiH, XI_PRM_AUTO_WB, 
      //0
      XI_ON
    );
    if (stat != XI_OK)
      throw "Setting auto white balance failed";

    // xiSetParamFloat(xiH, XI_PRM_WB_KR, WB_RED);
    // xiSetParamFloat(xiH, XI_PRM_WB_KG, WB_GREEN);
    // xiSetParamFloat(xiH, XI_PRM_WB_KB, WB_BLUE);





    // Get width of image
    int width = -1;
    stat = xiGetParamInt(xiH, XI_PRM_WIDTH, &width);
    if (stat != XI_OK)
      throw "Could not get image width from camera";    

    // Get height of image
    int height = -1;
    stat = xiGetParamInt(xiH, XI_PRM_HEIGHT, &height);
    if (stat != XI_OK)
      throw "Could not get image height from camera";













    // Start the image acquisition
    stat = xiStartAcquisition(xiH);
    if (stat != XI_OK)
    {
      throw "Starting image acquisition failed";
    }
    
    if(DO_SHOW_IMAGE)
    {
        // Create a GUI window with OpenGL support
        namedWindow("XIMEA camera", WINDOW_OPENGL);
        // OpenGL window makes problems!
        //namedWindow("XIMEA camera");
        resizeWindow("XIMEA camera", width/2, height/2);
        //resizeWindow("XIMEA camera", 1600 , 1000);
    }
 
    // Define pointer used for (CUDA) data on GPU
    //void *imageGpu;


    //{ init app state:
      // to zeros ...
      memset(&globalAppState, 0, sizeof(ApplicationState));

      globalAppState.cudaDemoisaicStream = cv::cuda::Stream();
      
      globalAppState.cudaFrameDataArray = 
        new CudaFrameData[MAX_ENQUEUED_CUDA_DEMOSAIC_IMAGES];
         
      // init GpuMats in case they are repurposed later without reinit:
      for(int i = 0; i < MAX_ENQUEUED_CUDA_DEMOSAIC_IMAGES; i++)
      {
        CudaFrameData * currentCudaFrameData = 
          & (globalAppState.cudaFrameDataArray[i]);
      
        currentCudaFrameData->gpuRawMatrix = 
          cuda::GpuMat(height, width, CV_8UC1);  
        currentCudaFrameData->gpuColorMatrix =
          cuda::GpuMat(height, width, CV_8UC3);
      }
    //}

  
  
    globalAppState.prev_time = getcurus();
  
    // Acquire a number of images, process and render them
    for (int i = 0; i < NUMBER_OF_IMAGES; i++)
    {
    
      // Get host-pointer to image data
      stat = xiGetImage(xiH, 5000, &image);
      if (stat != XI_OK)
      {
        throw "Getting image from camera failed";
      }
 
      //{ trying to parallelize cam image acquisition and cuda processing:
      int currentGpuMatIndex = 0;
      while(currentGpuMatIndex < MAX_ENQUEUED_CUDA_DEMOSAIC_IMAGES)
      {
        // is slot already used?
        if(globalAppState.cudaFrameDataArray[currentGpuMatIndex].slotIsUsed)
        {
            currentGpuMatIndex++;
        }
        else
        {
          // set slot to "used" and stop checking the rest
          globalAppState.cudaFrameDataArray[currentGpuMatIndex].slotIsUsed = 
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
        & (globalAppState.cudaFrameDataArray[currentGpuMatIndex]);

      currentCudaFrameData->bufferIndex = currentGpuMatIndex;
      currentCudaFrameData->slotIsUsed = true;
        

      //{ Blocking impl:
      /*
      // Convert Ximea image to cuda device pointer
      cudaHostGetDevicePointer(
        &currentCudaFrameData->cudaHostMemory, image.bp, 0);
      // Create GpuMat from the cuda device pointer
      currentCudaFrameData->gpuRawMatrix =
        cuda::GpuMat(height, width, CV_8UC1, &currentCudaFrameData->cudaHostMemory); 
      // Create GpuMat for the result image
      currentCudaFrameData->gpuColorMatrix =
        cuda::GpuMat(height, width, CV_8UC3); 
      //}
      */
      
      //{ Non-Blocking impl:
      cudaHostGetDevicePointer(
        &currentCudaFrameData->cudaHostMemory, image.bp, 0);
      //CudaMem hostSrc(height, width, CV_8UC1, CudaMem::ALLOC_PAGE_LOCKED);

      // upload data from host: 
      // wrapper arond host mem:   
      const int	sizes[] = {height, width};
      currentCudaFrameData->hostRawMat =
        cv::Mat(
          2, //int 	ndims,
          sizes, //const int * 	sizes,
          CV_8UC1, //int 	type,
          currentCudaFrameData->cudaHostMemory, // void * 	data,
          0 //const size_t * 	steps = 0 
        ); 
      currentCudaFrameData->gpuRawMatrix.upload(
        cv::InputArray(currentCudaFrameData->hostRawMat),
        globalAppState.cudaDemoisaicStream);
      //}
      



   
      // Demosaic raw bayer image to color image
      cuda::demosaicing(
        currentCudaFrameData->gpuRawMatrix, 
        currentCudaFrameData->gpuColorMatrix, 
        OCVbayer, 
        0, // derive channel layout from other params
        globalAppState.cudaDemoisaicStream );
     
      //  all allways 1 even with auto wb enabled 0o
      // printf("white balance params from ximea: %f, %f, %f\n",
      //   image.wb_red, image.wb_green, image.wb_blue);
        
      //Apply static white balance by multiplying the channels
      cuda::multiply(
        currentCudaFrameData->gpuColorMatrix, 
        cv::Scalar(WB_BLUE, WB_GREEN, WB_RED), 
        //cv::Scalar(image.wb_red, image.wb_green, image.wb_blue), 
        currentCudaFrameData->gpuColorMatrix,
        1.0, 
        -1,
        globalAppState.cudaDemoisaicStream
      );


   
      globalAppState.cudaDemoisaicStream.enqueueHostCallback(	
        cudaDemosaicStreamCallback, // StreamCallback 	callback,
        currentCudaFrameData // void * 	userData 
      );	
      

      //{ FPS calcs: ----------------------------------------------------------
	  globalAppState.current_captured_frame_count++;
	  if(image.nframe > globalAppState.last_camera_frame_count)
	  {
			globalAppState.lost_frames_count += image.nframe - (globalAppState.last_camera_frame_count + 1);
	  } 
	  globalAppState.last_camera_frame_count = image.nframe;
      globalAppState.current_time = getcurus();
      //update around each second:
	  if(globalAppState.current_time - globalAppState.prev_time > 1000000) 
	  {
			snprintf(
			    globalAppState.status_string, 
			    STATUS_STRING_LENGTH, 
			    "Acquisition [ Acquired: %lu, processed: %lu, skipped: %lu, fps: %.2f ]\n", 
			    globalAppState.current_captured_frame_count, 
			    globalAppState.cuda_processed_frame_count,
			    globalAppState.lost_frames_count, 
			    1000000.0 * 
			      (globalAppState.current_captured_frame_count - globalAppState.prev_captured_frame_count) 
			     / (globalAppState.current_time - globalAppState.prev_time)
			);
			printf("%s",globalAppState.status_string);
			
			globalAppState.prev_captured_frame_count = globalAppState.current_captured_frame_count;
			globalAppState.prev_time = globalAppState.current_time;  
	  }
      //}  --------------------------------------------------------------------



     if(DO_SHOW_IMAGE)
     {   
        // Render image to the screen (using OpenGL) 
        //imshow("XIMEA camera", currentCudaFrameData->gpuColorMatrix);
        
        //only show every nth frame:
        if(globalAppState.cuda_processed_frame_count % 3 == 0)
        {
            //block for show:
            globalAppState.cudaDemoisaicStream.waitForCompletion();
            
           
            //cuda::GpuMat gpuColorMatrixToShow (
            //  currentCudaFrameData->gpuColorMatrix);
            
            imshow("XIMEA camera", currentCudaFrameData->gpuColorMatrix);
       }
     }


      /* outsourced to cuda callback
      if(DO_SHOW_IMAGE)
      {
          // Render image to the screen (using OpenGL) 
          imshow("XIMEA camera", gpu_mat_color); 
           
          //imshow("XIMEA camera", gpu_mat_raw);      
          //test without OpenGL:
          //imshow("XIMEA camera non-GL", debugCPURawMat);      
      }
      */
      
      
      /*
      //{  pure CPU test: for debugging if cuda makes problems:  --------------
      // answer: cuda works fine, but OpenCV has problems with Qt5:
      // https://forums.developer.nvidia.com/t/black-images-with-qt5-and-opengl/70864/7
      // Building with QT4 resolves the issues!
      
      //https://docs.opencv.org/master/d3/d63/classcv_1_1Mat.html#a5fafc033e089143062fd31015b5d0f40
      cv::Mat debugCPURawMat(
        2, //int 	ndims,
        sizes, //const int * 	sizes,
        CV_8UC1, //int 	type,
        image.bp, // void * 	data,
        0 //const size_t * 	steps = 0 
      );
      cv::InputArray debugCPURawCVImage(debugCPURawMat);
      //} ---------------------------------------------------------------------
      */
      
      // prevent window gray-out
      waitKey(1);
    }
    
    // Stop image acquitsition and close device
    xiStopAcquisition(xiH);
    xiCloseDevice(xiH); 
     
    // Print errors
   }catch(const char* message){
    std::cerr << message << std::endl;
   }

}
