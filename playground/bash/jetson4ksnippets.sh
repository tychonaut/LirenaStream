#!/bin/bash

myResX=3840
myResY=3840

myFPS=8

numtotalFrames=32

# NOT works
#myResX=4096
#myResY=2160
#myResX=3840
#myResY=2160

GST_DEBUG=2 \
gst-launch-1.0 \
    videotestsrc num-buffers=${numtotalFrames} pattern=blink do-timestamp=true !  \
        "video/x-raw,width=${myResX},height=${myResY},framerate=${myFPS}/1" ! \
    tee name=myTee1  \
    \
    myTee1. ! \
    queue ! \
    nvvidconv ! \
        "video/x-raw(memory:NVMM),width=960,height=960" ! \
    nvoverlaysink \
    \
    myTee1. ! \
    nvvidconv ! \
    queue ! \
    queue max-size-bytes=1000000000 ! \
    nvv4l2h264enc ! \
    h264parse ! \
    mpegtsmux name=mp2ts_muxer alignment=7 \
    mp2ts_muxer. ! \
    tsparse ! \
    filesink location=a.mpg \
    
    
exit 0

GST_DEBUG=2 gst-launch-1.0 playbin uri=file://$(pwd)/a.mpg
        "video/x-raw(memory:NVMM),format=I420,width=${myResX},height=${myResY},framerate=${myFPS}/1" ! \
    queue max-size-bytes=1000000000 ! \

myResX=3840
myResY=2160

myFPS=5


# NOT works
#myResX=4096
#myResY=2160
#myResX=3840
#myResY=2160

GST_DEBUG=2 \
gst-launch-1.0 \
    videotestsrc num-buffers=100 pattern=ball do-timestamp=true !  \
        "video/x-raw,width=${myResX},height=${myResY},framerate=${myFPS}/1" ! \
        
    nvvidconv ! \
        "video/x-raw(memory:NVMM),width=960,height=540" ! \
    nvoverlaysink \
    
exit 0



#works
myResX=2800
myResY=2800

myResX=2048
myResY=2048

myResX=2560
myResY=1600

myResX=3840
myResY=2160

myFPS=5


# NOT works
#myResX=4096
#myResY=2160
#myResX=3840
#myResY=2160

GST_DEBUG=2 \
gst-launch-1.0 \
    videotestsrc num-buffers=100 pattern=ball do-timestamp=true !  \
        "video/x-raw,width=${myResX},height=${myResY},framerate=${myFPS}/1" ! \
    videoconvert ! \
    videoscale ! \
        "video/x-raw,width=512,height=512" ! \
    xvimagesink \
    
exit 0

echo finish
sleep 1

    tee name=myTee1  \
    \
    myTee1. ! \
    queue ! \

GST_DEBUG=2 gst-launch-1.0 playbin uri=file://$(pwd)/a.mp4

    \
    myTee1. ! \
    queue max-size-bytes=1000000000 ! \
    nvvidconv ! \
        "video/x-raw(memory:NVMM),format=I420,width=${myResX},height=${myResY},framerate=${myFPS}/1" ! \
    queue max-size-bytes=1000000000 ! \
    nvv4l2h264enc ! \
    h264parse ! \
    filesink location=a.mp4 \
    




 ##############################

#works
myResX=2800
myResY=2800
myFPS=25


# NOT works
#myResX=4096
#myResY=2160
#myResX=3840
#myResY=2160

GST_DEBUG=2 \
gst-launch-1.0 \
    videotestsrc num-buffers=25 !  \
        "video/x-raw,width=${myResX},height=${myResY},framerate=${myFPS}/1" ! \
    queue max-size-bytes=1000000000 ! \
    nvvidconv ! \
        "video/x-raw(memory:NVMM),format=I420,width=${myResX},height=${myResY}" ! \
    queue max-size-bytes=1000000000 ! \
    nvv4l2h264enc ! \
    h264parse ! \
    filesink location=a.mp4

GST_DEBUG=2 gst-launch-1.0 playbin uri=file://$(pwd)/a.mp4

exit 0


# ximae cam res: 5328 4608
# half res:2664 2304
GST_DEBUG=2 \
gst-launch-1.0 \
    videotestsrc num-buffers=60 !  \
        'video/x-raw,width=4000,height=4000,framerate=10/1' ! \
    nvvidconv ! \
    queue ! \
    nvv4l2h264enc ! \
    h264parse ! \
    filesink location=a.mp4
    
exit 0 

        'video/x-raw(memory:NVMM), format=I420, width=4096, height=4096' ! \

#    nvvidconv left=616 right=4712 top=256 bottom=4352 ! \
# ximae cam res: 5328 4608
# half res:2664 2304
GST_DEBUG=2 \
gst-launch-1.0 \
    videotestsrc num-buffers=60 !  \
        'video/x-raw,width=5328,height=4608,framerate=10/1' ! \
    queue ! \
    nvvidconv left=616 right=4712 top=256 bottom=4352 ! \
        'video/x-raw(memory:NVMM), format=I420, width=2048, height=2048' ! \
    queue ! \
    nvv4l2h264enc ! \
    h264parse ! \
    filesink location=a.mp4
    
exit 0 

       'video/x-raw(memory:NVMM), format=I420, width=2048, height=2048' ! \
    nvvidconv left=617 right=4711 top=257 bottom=4351 interpolation-method=1 ! \

#2048 4096
#nvv4l2h264enc maxperf-enable=1 bitrate=8000000 ! \
    h264parse  disable-passthrough=true ! \

# works, though slow:
# ximae cam res: 5328 4608
# half res:2664 2304
GST_DEBUG=2 \
gst-launch-1.0 \
    videotestsrc num-buffers=60 !  \
        'video/x-raw,width=5328,height=4608,framerate=10/1' ! \
    queue ! \
    nvvidconv left=616 right=4712 top=256 bottom=4352 ! \
       'video/x-raw(memory:NVMM), format=I420, width=2048, height=2048, framerate=10/1' ! \
    nvv4l2h264enc maxperf-enable=1 bitrate=8000000 ! \
    h264parse  disable-passthrough=true ! \
    filesink location=a.mp4
    
exit 0 


    queue ! \   
# works, though slow:
# ximae cam res: 5328 4608
# half res:2664 2304
GST_DEBUG=2 \
gst-launch-1.0 \
    videotestsrc num-buffers=60 !  \
        'video/x-raw,width=5328,height=4608,framerate=10/1' ! \
    queue ! \
       'video/x-raw(memory:NVMM), format=I420, width=2664, height=2304, framerate=10/1' ! \
    nvvidconv left=616 right=4712 top=256 bottom=4352 ! \
    nvv4l2h264enc maxperf-enable=1 bitrate=8000000 ! \
    h264parse  disable-passthrough=true ! \
    filesink location=a.mp4
    
exit 0 




#works:
GST_DEBUG=2 \
gst-launch-1.0 \
    videotestsrc num-buffers=60 !  \
    'video/x-raw,width=1024,height=1024,framerate=10/1' ! \
    queue ! \
    nvvidconv left=256 right=768 top=256 bottom=768 ! \
    nvv4l2h264enc maxperf-enable=1 bitrate=8000000 ! \
    h264parse  disable-passthrough=true ! \
    filesink location=a.mp4
    
exit 0 
#        'video/x-raw,width=5328,height=4608,framerate=10/1' ! \
    nvvidconv left=616 right=616 top=256 bottom=256 ! \
       'video/x-raw(memory:NVMM), format=I420, width=4096, height=4096, framerate=10/1' ! \
#interpolation-method=1


# works, though slow:
# ximae cam res: 5328 4608
# half res:2664 2304
GST_DEBUG=2 \
gst-launch-1.0 \
    videotestsrc num-buffers=60 !  \
        'video/x-raw,width=5328,height=4608,framerate=10/1' ! \
    queue ! \
    nvvidconv ! \
       'video/x-raw(memory:NVMM), format=I420, width=2664, height=2304, framerate=10/1' ! \
    nvv4l2h264enc maxperf-enable=1 bitrate=8000000 ! \
    h264parse  disable-passthrough=true ! \
    filesink location=a.mp4
    
exit 0 


#works (4096,height=4096) --> nvvidconf (width=640, height=480)
GST_DEBUG=2 \
gst-launch-1.0 \
    videotestsrc num-buffers=60 !  \
        'video/x-raw,width=4096,height=4096,framerate=10/1' ! \
    queue ! \
    nvvidconv ! \
       'video/x-raw(memory:NVMM), format=I420, width=640, height=480, framerate=10/1' ! \
    nvv4l2h264enc maxperf-enable=1 bitrate=8000000 ! \
    h264parse  disable-passthrough=true ! \
    filesink location=a.mp4
    
exit 0 



#works (4096,height=4096) --> nvvidconf (width=640, height=480)
GST_DEBUG=2 \
gst-launch-1.0 \
    videotestsrc num-buffers=60 !  \
        'video/x-raw,width=4096,height=4096,framerate=10/1' ! \
    queue ! \
    nvvidconv ! \
       'video/x-raw(memory:NVMM), format=I420, width=640, height=480, framerate=10/1' ! \
    nvv4l2h264enc maxperf-enable=1 bitrate=8000000 ! \
    h264parse  disable-passthrough=true ! \
    filesink location=a.mp4
    
exit 0 




# works:
# video/x-raw,width=2048,height=2048,framerate=10/1 ! 
# video/x-raw,width=2560,height=1600,framerate=10/1 !
# works NOT!!!11
# 4096
# 3072
#         video/x-raw,width=4096,height=2160,framerate=10/1 ! \

#works (3840,height=2160) --> nvvidconf (width=1920, height=1080)
GST_DEBUG=2 \
gst-launch-1.0 \
    videotestsrc num-buffers=60 !  \
        video/x-raw,width=3840,height=2160,framerate=10/1 ! \
    queue ! \
    nvvidconv ! \
       'video/x-raw(memory:NVMM), format=I420, width=1920, height=1080, framerate=10/1' ! \
    nvv4l2h264enc maxperf-enable=1 bitrate=8000000 ! \
    h264parse  disable-passthrough=true ! \
    filesink location=a.mp4
    
exit 0 
  
  
    
  #2896
  
    qtmux ! \
#    omxh264enc ! qtmux ! filesink location=a.mp4



#works, same problems like with nvoverlaysink
 gst-launch-1.0     videotestsrc num-buffers=30 !     video/x-raw,width=4096,height=4096,framerate=10/1 ! videoconvert ! videoscale ! video/x-raw,width=1024,height=1024 !   xvimagesink

exit 0 

 # works with 10fps, though painfully slopw, no image at all at 30fps
 gst-launch-1.0     videotestsrc num-buffers=30 !     video/x-raw,width=4096,height=4096,framerate=10/1 ! videoconvert ! videoscale ! video/x-raw,width=1024,height=1024 !   nvoverlaysink
 
