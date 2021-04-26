#!/bin/bash

## UDP RECEIVER :
# works
# udp -> rtpdepay -> dump to disk
#     -> tsdemux  -> h264 dec -> scale -> show
#                 -> klv      -> fakesink (dump klv to console)
#                             -> dump to disk

#myDisplayResX=2048
#myDisplayResY=1556

myDisplayResX=1024
myDisplayResY=778


myOutputFilePath=./deepStream_000.mpg


#GST_PLUGIN_PATH=$GST_PLUGIN_PATH:/usr/local/lib/gstreamer-1.0/ gst-inspect-1.0 nvdec


GST_PLUGIN_PATH=$GST_PLUGIN_PATH:/usr/local/lib/gstreamer-1.0/ \
GST_DEBUG=2,nvdec*:4,nvenc*:0 \
__GL_SYNC_TO_VBLANK=0 \
gst-launch-1.0 \
  udpsrc port=5001  \
      caps='application/x-rtp, media=(string)video, clock-rate=(int)90000, encoding-name=(string)MP2T' ! \
  rtpjitterbuffer !\
  rtpmp2tdepay ! \
  tee name=fork_mp2ts_to_disk_and_show \
  \
  fork_mp2ts_to_disk_and_show. ! \
  queue !   \
  tsdemux name=myTsDemux  \
  \
  myTsDemux. ! \
  queue !  \
    "video/x-h264" ! \
  h264parse ! \
  nvdec ! \
  glcolorconvert ! \
  glimagesink async-handling=true \
  \
  fork_mp2ts_to_disk_and_show. ! \
  queue !   \
  filesink location=${myOutputFilePath} async=true
  
exit 0
  
  
  latency=50 ! 
  
  sync=false async=true \
  sync=false async=true \


    "video/x-raw(memory:GLMemory),format=(string)NV12" ! \
    "video/x-raw(memory:GLMemory),format=(string)RGBA" ! \

  gtkglsink sync=false async=true \
  
  
  sync=false async=true
  
   sync=true async=true
  
  
  
    \
  avdec_h264 !   \
  videoconvert !   \
  videoscale !   \
    video/x-raw,width=${myDisplayResX},height=${myDisplayResY} ! \
  fpsdisplaysink  video-sink="xvimagesink" sync=false async-handling=true \


#klv not works atm with "expanded standalone app"
  \
  myTsDemux. ! \
    meta/x-klv,parsed=true ! \
    queue ! \
    tee name=myT2 \
    \
    myT2. ! \
      queue ! \
      fakesink dump=true sync=true async=true \
    \
    myT2. ! \
      queue ! \
    filesink location=fromUdp_remoteOutput_010.klv sync=true async=true
    
    


  xvimagesink \
video/x-raw,width=4096,height=3112 ! \
xvimagesink sync=false async=true \
