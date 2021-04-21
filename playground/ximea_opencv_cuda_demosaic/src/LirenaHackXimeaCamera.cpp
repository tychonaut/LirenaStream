#include "LirenaHackXimeaCamera.h"

#include <opencv2/imgproc.hpp> //Bayer color pattern enum

#include <unistd.h> // sleep
#include <glib-2.0/glib.h> //g_assert

#include <iostream> //std::cout


LirenaHackXimeaCamera::LirenaHackXimeaCamera(LirenaConfig *config)
    : 
        config(config),
        xiH(nullptr)
{
    bool success = openDevice();
    success &= setupParams();
    g_assert(success);
}

LirenaHackXimeaCamera::~LirenaHackXimeaCamera()
{
    stopAcquisition();
    closeDevice();
}


bool LirenaHackXimeaCamera::openDevice()
{
    XI_RETURN xiStatus = XI_OK;

    // Initialize XI_IMG structure
    memset(&xiImage, 0, sizeof(XI_IMG));
    xiImage.size = sizeof(XI_IMG);

    // Get device handle for the camera
    xiStatus =  xiOpenDevice(0, &xiH);
    if (xiStatus != XI_OK)
    {
      throw "Opening device failed";
    }

    return xiStatus == XI_OK;
}








bool LirenaHackXimeaCamera::setupParams()
{
  //XI_RETURN xiStatus = XI_OK;

  bool success = true;
  
  success &= setupDownsamplingParams();
  g_assert(success);

  success &= setupBayerParams();
  g_assert(success);

  success &= setupTriggerParams();
  g_assert(success);

  success &= setupMiscParams();
  g_assert(success);

  return success;
}



bool LirenaHackXimeaCamera::setupDownsamplingParams()
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
  xiSetParamInt(xiH, XI_PRM_DECIMATION_VERTICAL, global_decimationMultiplierToSet);
  if (xiStatus != XI_OK)
  {
    printf(" XI_PRM_DECIMATION_VERTICAL := %d: return value not XI_OK: %d", global_decimationMultiplierToSet, xiStatus);
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
  xiSetParamInt(xiH, XI_PRM_DECIMATION_HORIZONTAL, global_decimationMultiplierToSet);
  if (xiStatus != XI_OK)
  {
    printf(" XI_PRM_DECIMATION_HORIZONTAL := %d: return value not XI_OK: %d", global_decimationMultiplierToSet, xiStatus);
    sleep(4);
  }

  return xiStatus == XI_OK;
}



bool LirenaHackXimeaCamera::setupBayerParams()
{

  XI_RETURN xiStatus = XI_OK;

  // Get type of camera color filter
  int cfa = 0;
  xiStatus = xiGetParamInt(xiH, XI_PRM_COLOR_FILTER_ARRAY, &cfa);
  if (xiStatus != XI_OK)
    throw "Could not get color filter array from camera";

  // Set correct demosaicing type according to camera color filter
  switch (cfa)
  {
  case XI_CFA_BAYER_RGGB:
  {
    std::cout << "BAYER_RGGB color filter." << std::endl;
    OCVbayerPattern = cv::COLOR_BayerRG2BGR;
    break;
  }
  case XI_CFA_BAYER_BGGR:
  {
    std::cout << "BAYER_BGGR color filter." << std::endl;
    OCVbayerPattern = cv::COLOR_BayerBG2BGR;
    break;
  }
  case XI_CFA_BAYER_GRBG:
  {
    std::cout << "BAYER_GRBG color filter." << std::endl;
    OCVbayerPattern = cv::COLOR_BayerGR2BGR;
    break;
  }
  case XI_CFA_BAYER_GBRG:
  {
    std::cout << "BAYER_GBRG color filter." << std::endl;
   OCVbayerPattern = cv::COLOR_BayerGB2BGR;
    break;
  }
  default:
  {
    throw "Not supported color filter for demosaicing.";
  }
  }

  return xiStatus == XI_OK;
}


bool LirenaHackXimeaCamera::setupTriggerParams()
{

  XI_RETURN xiStatus = XI_OK;

  printf("TODO implement LirenaHackXimeaCamera::setupTriggerParams()\n");


  return xiStatus == XI_OK;
}


bool LirenaHackXimeaCamera::setupMiscParams()
{

  XI_RETURN xiStatus = XI_OK;


  // Use transport data format (no processing done by the API)
  xiStatus = xiSetParamInt(xiH, XI_PRM_IMAGE_DATA_FORMAT, XI_FRM_TRANSPORT_DATA);
  if (xiStatus != XI_OK)
    throw "Setting image data format failed";

  // Make data from the camera stream to zerocopy memory
  xiStatus = xiSetParamInt(xiH, XI_PRM_TRANSPORT_DATA_TARGET, XI_TRANSPORT_DATA_TARGET_ZEROCOPY);
  if (xiStatus != XI_OK)
    throw "Setting transport data target failed";

  // Using 8-bit images here
  xiStatus = xiSetParamInt(xiH, XI_PRM_OUTPUT_DATA_BIT_DEPTH, 8);
  if (xiStatus != XI_OK)
    throw "Setting bit depth failed";

  // Exposure 4 ms is what they need in the project
  xiStatus = xiSetParamInt(xiH, XI_PRM_EXPOSURE, 
    1000* config->ximeaparams.exposure_ms);
  if (xiStatus != XI_OK)
    throw "Setting exposure failed";

  // gain
  float mingain, maxgain;
  xiGetParamFloat(xiH, XI_PRM_GAIN XI_PRM_INFO_MIN, &mingain);
  xiGetParamFloat(xiH, XI_PRM_GAIN XI_PRM_INFO_MAX, &maxgain);

  float mygain = mingain + (maxgain - mingain) * 1.0f;
  xiSetParamFloat(xiH, XI_PRM_GAIN, mygain);

  // do auto whitebalance -> sems not applied to image,
  // but the three multipliers are in aquried image struct (?)
  xiStatus = xiSetParamInt(xiH, XI_PRM_AUTO_WB,
                       //0
                       XI_ON);
  if (xiStatus != XI_OK)
    throw "Setting auto white balance failed";

  // xiSetParamFloat(xiH, XI_PRM_WB_KR, WB_RED);
  // xiSetParamFloat(xiH, XI_PRM_WB_KG, WB_GREEN);
  // xiSetParamFloat(xiH, XI_PRM_WB_KB, WB_BLUE);

  // Get width of image

  xiStatus = xiGetParamInt(xiH, XI_PRM_WIDTH, 
    &config->ximeaparams.activeSensorResolutionX);
  if (xiStatus != XI_OK)
  { 
      throw "Could not get image width from camera";
  }

  // Get height of image
  xiStatus = xiGetParamInt(xiH, XI_PRM_HEIGHT, 
    &config->ximeaparams.activeSensorResolutionY);
  if (xiStatus != XI_OK)
  {
    throw "Could not get image height from camera";
  }

  return xiStatus == XI_OK;
}





XI_IMG & LirenaHackXimeaCamera::acquireNewFrame()
{
    // Get host-pointer to image data
    XI_RETURN xiStatus =
        xiGetImage(xiH, 5000, &xiImage);
    if (xiStatus != XI_OK)
    {
        throw "Getting image from camera failed";
    }

    return xiImage;
}

bool LirenaHackXimeaCamera::startAcquisition()
{
    XI_RETURN xiStatus = XI_OK;

    xiStatus = xiStartAcquisition(xiH);
    if (xiStatus != XI_OK)
    {
      throw "Starting image acquisition failed";
    }

    return xiStatus == XI_OK;
}


bool LirenaHackXimeaCamera::stopAcquisition()
{
    XI_RETURN xiStatus = XI_OK;

    // Stop image acquitsition and close device
    xiStatus = xiStopAcquisition(xiH);


    return xiStatus == XI_OK;
}


bool LirenaHackXimeaCamera::closeDevice()
{
    XI_RETURN xiStatus = XI_OK;

    // Stop image acquitsition and close device
    xiStatus = xiCloseDevice(xiH); 
    xiH = nullptr;

    return xiStatus == XI_OK;
}



