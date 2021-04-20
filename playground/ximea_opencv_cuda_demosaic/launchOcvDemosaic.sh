#!/bin/bash


#n.b. Sensore reolution of Ximea Cam is : 5328x4608


#{ works
#works
myResX=3840
#works
myResY=3112
#}

#works
#myResY=2160


#{ works! full fps (26.x) (WITH CPU upload, but WITHOUT local display!)
#  Resolution name: "Full Aperture 4x"
#  https://en.wikipedia.org/wiki/List_of_common_resolutions
myResX=4096
myResY=3112
#}


#half res when testing sensor pixel halfing ("decimation) 
# (no CLI param yet, have to change DO_USE_SENSOR_DECIMATION in C++ source file)
#myResX=2048
#myResY=1556


ninja -C build/ 

__GL_SYNC_TO_VBLANK=0 \
GST_DEBUG=4 \
./build/ocvDemosaic \
     192.168.1.101 5001 \
     --targetResolutionX=${myResX} \
     --targetResolutionY=${myResY} \
     --exposure=4 \
     $@

exit 0

     --localdisplay
