#ifndef __LIRENA_HACK_XIMEA_CAMERA_H__
#define __LIRENA_HACK_XIMEA_CAMERA_H__

#include "LirenaConfig.h"

#include <m3api/xiApi.h>


namespace cv
{
    class Mat;    
}

class LirenaHackXimeaCamera
{
    public:
    explicit LirenaHackXimeaCamera(LirenaConfig * config);

    ~LirenaHackXimeaCamera();

    // to be called by app
    bool startAcquisition();
    XI_IMG & acquireNewFrame();
    XI_IMG & getCurrentFrame(){return xiImage;}
    int getOcvBayerPattern() const {return OCVbayerPattern;}

private:

    // called by ctr
    bool openDevice();
    bool setupParams();
    bool setupDownsamplingParams();
    bool setupBayerParams();
    bool setupTriggerParams();
    bool setupMiscParams();

    // called by dtr
    bool stopAcquisition();
    bool closeDevice();


    //member variables:

    //will change config
    LirenaConfig * config;

    HANDLE xiH = NULL;
    XI_IMG xiImage;
    
    int OCVbayerPattern = 0;

};


// hack
extern const int global_decimationMultiplierToSet;

#endif // __LIRENA_HACK_XIMEA_CAMERA_H__