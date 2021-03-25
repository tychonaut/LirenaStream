#include <m3api/xiApi.h>
#include <iostream>
#include <opencv2/highgui.hpp>
#include <opencv2/cudaimgproc.hpp>
#include <opencv2/cudaarithm.hpp>
#include <cuda_runtime.h>

// Define the number of images acquired, processed and shown in this example
#define NUMBER_OF_IMAGES 1000

// Define parameters for a static white balance
#define WB_BLUE 2
#define WB_GREEN 1
#define WB_RED 1.3

using namespace cv;
using namespace std;

int main(){
  // Initialize XI_IMG structure
  XI_IMG image;
  memset(&image, 0, sizeof(XI_IMG));
  image.size = sizeof(XI_IMG);

  HANDLE xiH = NULL;
  XI_RETURN stat = XI_OK;

  // Simplyfied error handling (just for demonstration)
  try{
    int cfa = 0;
    int OCVbayer = 0;

    // Get device handle for the camera
    stat = xiOpenDevice(0, &xiH);
    if (stat != XI_OK)
      throw "Opening device failed";

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
      
      
    float mingain, maxgain;	
	xiGetParamFloat(xiH, XI_PRM_GAIN XI_PRM_INFO_MIN, &mingain);
	xiGetParamFloat(xiH, XI_PRM_GAIN XI_PRM_INFO_MAX, &maxgain);
	float mygain = mingain +  (maxgain - mingain) * 1.0f;
	xiSetParamFloat(xiH, XI_PRM_GAIN, mygain);     
      
      

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
      throw "Starting image acquisition failed";

    // Create a GUI window with OpenGL support
    namedWindow("XIMEA camera", WINDOW_OPENGL);
    // OpenGL window makes problems!
    //namedWindow("XIMEA camera");
    resizeWindow("XIMEA camera", width/3, height/3);
    resizeWindow("XIMEA camera", width, height);
 
    // Define pointer used for data on GPU
    void *imageGpu;

    // Create GpuMat for the result images
    cuda::GpuMat gpu_mat_color(height, width, CV_8UC3);
  
  
    // Acquire a number of images, process and render them
    for (int i = 0; i < NUMBER_OF_IMAGES; i++){
      // Get host-pointer to image data
      stat = xiGetImage(xiH, 5000, &image);
      if (stat != XI_OK)
        throw "Getting image from camera failed";
        
      int sizes[2] = {height, width};
      
      /*
      // pure CPU test: for debugging if cuda makes problems: 
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
      */
       
      
      // Convert to device pointer
      cudaHostGetDevicePointer(&imageGpu, image.bp, 0);
      // Create GpuMat from the device pointer
      cuda::GpuMat gpu_mat_raw(height, width, CV_8UC1, imageGpu);
      // Demosaic raw bayer image to color image
      cuda::demosaicing(gpu_mat_raw, gpu_mat_color, OCVbayer);
      // Apply static white balance by multiplying the channels
      cuda::multiply(gpu_mat_color, cv::Scalar(WB_BLUE, WB_GREEN, WB_RED), gpu_mat_color);

      
      // Render image to the screen (using OpenGL) 
      imshow("XIMEA camera", gpu_mat_color);  
      //imshow("XIMEA camera", gpu_mat_raw);      
      //test without OpenGL:
      //imshow("XIMEA camera non-GL", debugCPURawMat);      
      
      
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
