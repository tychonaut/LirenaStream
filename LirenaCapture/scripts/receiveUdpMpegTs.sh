#!/bin/bash

## UDP RECEIVER :
# works
# udp -> rtpdepay -> dump to disk
#     -> tsdemux  -> h264 dec -> scale -> show
#                 -> klv      -> fakesink (dump klv to console)
#                             -> dump to disk



GST_DEBUG=0 \
__GL_SYNC_TO_VBLANK=0 \
gst-launch-1.0 \
  udpsrc port=5001  \
      caps='application/x-rtp, media=(string)video, clock-rate=(int)90000, encoding-name=(string)MP2T' ! \
  rtpjitterbuffer latency=50 ! \
  rtpmp2tdepay ! \
  tee name=fork_mp2ts_to_disk_and_show \
  \
  fork_mp2ts_to_disk_and_show. ! \
  queue !   \
  filesink location=./remoteOutput_010.mpg \
  \
  fork_mp2ts_to_disk_and_show. ! \
  queue !   \
  tsdemux name=myTsDemux  \
  \
  myTsDemux. ! \
    video/x-h264 ! \
  h264parse ! \
  queue !    \
  avdec_h264 !   \
  videoconvert !   \
  videoscale !   \
    video/x-raw,width=2048,height=1556 ! \
  fpsdisplaysink  video-sink="xvimagesink" sync=false async-handling=true \

exit 0

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
