#!/bin/bash

## FILE PLAYBACK :
# works
#  filesrc -> tsdemux  -> h264 dec -> show
#                      -> klv      -> fakesink (dump klv to console)
#                                  -> dump to disk
GST_DEBUG=2 \
__GL_SYNC_TO_VBLANK=0 \
gst-launch-1.0  \
  filesrc location=$(pwd)/myAss2.mpg ! \
  tee name=myT1 \
  \
  myT1. ! \
    queue ! \
    tsdemux ! \
      'video/x-h264' ! \
    h264parse ! \
    nvv4l2decoder enable-frame-type-reporting=true enable-max-performance=true ! \
    nvvidconv ! \
    nv3dsink -e

exit 0

    nvoverlaysink display-id=0 \
    nvv4l2h264dec
    
    avdec_h264 output-corrupt=false  !   \
    videoconvert !  \
    xvimagesink   sync=true async=true \


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
    filesink location=fromFile_remoteOutput_010.klv sync=true async=true
    
    
    

