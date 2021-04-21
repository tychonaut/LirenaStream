#!/bin/bash

# first test pipeline for preparing cv::cuda:demosaiced stuff:

# cv::cuda has already cropped the image to "Full Aperture 4x" 
# resolution (4096x3112). 
#
# To keep it simple: cv  has also downloaded cv::cuda::GpuMat back to
# a cv::Mat, whose image data memory has been g_alloc()'ed, so that
# a destructio of the cv::Mat won't f*** up the memory that we will pass
# to GStreamer (will be destroyed by a sink; TODO research GstBufferPool to 
# omit constant allocation!).
# 
# Now, a CPU-Buffer 



#					appsrc_video_caps = gst_caps_new_simple(
#						"video/x-raw",
#						"format", G_TYPE_STRING, "BGRx",
#						"bpp", G_TYPE_INT, 32,
#						"depth", G_TYPE_INT, 24,
#						"endianness", G_TYPE_INT, G_BIG_ENDIAN,
#						"red_mask", G_TYPE_INT, 0x0000ff00,
#						"green_mask", G_TYPE_INT, 0x00ff0000,
#						"blue_mask", G_TYPE_INT, 0xff000000,
#						"framerate", GST_TYPE_FRACTION, 0, 1,
#						"width", G_TYPE_INT, image.width,
#						"height", G_TYPE_INT, image.height,
#						NULL);


# works:  real Cam sensor res:
mySensorResX=5328
mySensorResY=4608

# works:
#  Resolution name: "Full Aperture 4x"
#  https://en.wikipedia.org/wiki/List_of_common_resolutions
myStreamingResX=4096
myStreamingResY=3112




myTargetFPS=2

numtotalFrames=32

# maybe capsfilter after videotestsrc: format=(string):
# BGRx or xRGB or RGBx or RGBA .. have to try

# maybe capsfilter after first nvvidconv : format=(string):
# NV12 NV16 or whatever

GST_DEBUG=2 \
gst-launch-1.0 \
    videotestsrc num-buffers=${numtotalFrames}  do-timestamp=true !  \
        "video/x-raw,width=${myStreamingResX},height=${myStreamingResY},framerate=${myTargetFPS}/1" ! \
    nvvidconv ! \
         "video/x-raw(memory:NVMM),format=(string)NV12,width=${myStreamingResX},height=${myStreamingResY},framerate=${myTargetFPS}/1" ! \
    tee name=T \
    \
    T. ! \
    queue max-size-bytes=1000000000 ! \
    nvivafilter cuda-process=true customer-lib-name=libnvsample_cudaprocess.so ! \
        'video/x-raw(memory:NVMM),format=(string)NV12' ! \
    nvvidconv ! \
         "video/x-raw(memory:NVMM),format=(string)NV12,width=512,height=512,framerate=${myTargetFPS}/1" ! \
    nvoverlaysink display-id=0  \
    
exit 0
    
    
    
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
