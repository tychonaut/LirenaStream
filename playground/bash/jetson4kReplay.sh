#!/bin/bash



#trying to adapt to corrupted remote dump:
# seems to work relatively stable now, though no idea why (and why not before)
GST_DEBUG=2 \
__GL_SYNC_TO_VBLANK=0 \
gst-launch-1.0  \
  filesrc location=/home/nvidia/devel/streaming/LirenaStream/playground/bash/a.mpg ! \
  tee name=myT1 \
  \
  myT1. ! \
    queue ! \
    tsdemux ! \
      'video/x-h264' ! \
    h264parse ! \
    avdec_h264 output-corrupt=false  !   \
    videoconvert !  \
    xvimagesink   sync=true async=true
    
exit 0

