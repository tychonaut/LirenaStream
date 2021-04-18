#!/bin/bash







# works:  real Cam sensor res:
mySensorResX=5328
mySensorResY=4608

# works:
#  Resolution name: "Full Aperture 4x"
#  https://en.wikipedia.org/wiki/List_of_common_resolutions
myStreamingResX=4096
myStreamingResY=3112


#mySensorResX=1920
#mySensorResY=1080

#mySensorResX=3840
#mySensorResY=2160

#mySensorResX=4096
#mySensorResY=3112

#works
#mySensorResX=3840
#mySensorResY=3840






myTargetFPS=2

numtotalFrames=32

# NOT works
#myResX=4096
#myResY=2160
#myResX=3840
#myResY=2160


#TODO make first nvvidconv crop instead of resize!
#TODO check if non-in-plce stuff can work with nvivafilter

GST_DEBUG=4 \
gst-launch-1.0 \
    videotestsrc num-buffers=${numtotalFrames}  do-timestamp=true !  \
        "video/x-raw,width=${mySensorResX},height=${mySensorResY},framerate=${myTargetFPS}/1" ! \
    nvvidconv ! \
         "video/x-raw(memory:NVMM),format=(string)NV12,width=${mySensorResX},height=${mySensorResY},framerate=${myTargetFPS}/1" ! \
    tee name=T \
    \
    T. ! \
    queue max-size-bytes=1000000000 ! \
    nvivafilter cuda-process=true customer-lib-name="libnvsample_cudaprocess.so" ! \
        'video/x-raw(memory:NVMM),format=(string)NV12' ! \
    nvvidconv ! \
         "video/x-raw(memory:NVMM),format=(string)NV12,width=512,height=512,framerate=${myTargetFPS}/1" ! \
    nvoverlaysink display-id=0  \
    \
    T. ! \
    queue max-size-bytes=1000000000 ! \
    nvvidconv ! \
        "video/x-raw(memory:NVMM),width=${myStreamingResX},height=${myStreamingResY}" ! \
    nvv4l2h264enc ! \
    h264parse ! \
    mpegtsmux name=mp2ts_muxer alignment=7 \
    mp2ts_muxer. ! \
    tsparse ! \
    filesink location=a.mpg  \


exit 0

