#!/bin/bash


#myOutputFilePath=./deepStream_000.mpg

myOutputFilePath=./backup.mpg



#GST_PLUGIN_PATH=$GST_PLUGIN_PATH:/usr/local/lib/gstreamer-1.0/ \
#GST_DEBUG=nvdec*:6,nvenc*:6 \
#    gst-inspect-1.0 nvdec
#exit 0




### FILE PLAYBACK :
# works
#  filesrc -> tsdemux  -> h264 dec -> show
#                      -> klv      -> fakesink (dump klv to console)
#                                  -> dump to disk
GST_PLUGIN_PATH=$GST_PLUGIN_PATH:/usr/local/lib/gstreamer-1.0/ \
GST_DEBUG=nvdec*:0,nvenc*:0 \
__GL_SYNC_TO_VBLANK=0 \
gst-launch-1.0  \
  filesrc location=$(pwd)/${myOutputFilePath} ! \
  tee name=myT1 \
  \
  myT1. ! \
    queue ! \
    tsdemux ! \
      "video/x-h264" ! \
    h264parse ! \
    nvdec ! \
      "video/x-raw(memory:GLMemory),format=(string)NV12" ! \
    glcolorconvert ! \
    "video/x-raw(memory:GLMemory),format=(string)RGBA" ! \
    glimagesink sync=true async=true    
    
exit 0
    
    gtkglsink sync=true async=true
    
    videoconvert !  \
    xvimagesink   sync=true async=true \


    avdec_h264 output-corrupt=false  !   \

  \
  myT1. ! \
    queue ! \
    tsdemux ! \
      'meta/x-klv' ! \
    tee name=myT2 \
    \
    myT2. ! \
      queue ! \
      fakesink dump=true sync=true async=true \
    \
    myT2. ! \
      queue ! \
    filesink location=${myOutputFilePath}.klv sync=true async=true
    
    
    

