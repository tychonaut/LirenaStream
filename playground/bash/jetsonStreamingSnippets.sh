
#-------------------------------------------------------------------------------------------------------------------------------------
#{ Jetson-to desktop-stream, pure h264 via RTP & UDP


# Jetson h264 encoder and sender:
#   cam -> show on screen
#      \-> hardware encode h264 -> write to disk
#                              \-> stream per as RTP over UDP
GST_DEBUG=4 \
gst-launch-1.0 -vvvv -m \
  videotestsrc is-live=1 pattern=ball ! \
    video/x-raw,width=1280,height=720 ! \
  tee name=forkRaw2ShowNEnc ! \
  queue ! \
  videoscale add-borders=TRUE ! \
  videoconvert ! \
  capsfilter name=scale ! \
  xvimagesink \
  forkRaw2ShowNEnc. ! \
  queue ! \
  nvvidconv ! \
  nvv4l2h264enc ! \
  h264parse ! \
  tee name=forkEnc2DiskNUDP ! \
  queue ! \
  filesink location=myJetsonTx2NvEncedMp2Dump.mp4 -e \
  forkEnc2DiskNUDP. ! \
  queue ! \
  rtph264pay ! \
  udpsink host=192.168.0.169 port=5001 sync=false



#  Desktop receiver, both show on screen and dump to disk
gst-launch-1.0 -v -m \
  udpsrc port=5001 caps='application/x-rtp, media=(string)video, clock-rate=(int)90000, encoding-name=(string)H264' ! \
  rtph264depay ! \
  h264parse ! \
  tee name=fork_h264_to_show_and_mp2ts ! \
  queue ! \
  avdec_h264 ! \
  videoconvert ! \
  xvimagesink sync=false \
  fork_h264_to_show_and_mp2ts. ! \
  queue ! \
  mpegtsmux name=mp2ts_muxer ! \
  filesink location=./udp_rtp264depay_mpegtsmux.mpg

#check on jetson:
gst-launch-1.0 playbin uri=file://$(pwd)/udp_rtp264depay_mpegtsmux.mpg



# rest of code is obsolete and outdated in this section

#  Desktop receiver, show on screen: from marco 
#  (h264parse added by me, in hope for greater compatibility)
GST_DEBUG=4 \
gst-launch-1.0 -vvv -m \
  udpsrc port=5001 caps='application/x-rtp, media=(string)video, clock-rate=(int)90000, encoding-name=(string)H264' ! \
  rtph264depay ! \
  h264parse ! \
  avdec_h264 ! \
  videoconvert ! \
  xvimagesink sync=false


# Desktop receiver, dump to disk
GST_DEBUG=5 gst-launch-1.0 \
 pipeline \( latency=1000000000 \
  udpsrc port=5001 caps="application/x-rtp, media=(string)video, clock-rate=(int)90000, encoding-name=(string)H264, packetization-mode=(string)1, profile-level-id=(string)42401f, sprop-parameter-sets=(string)\"Z0JAH5ZUAoAtyA\=\=\,aM48gA\=\=\", payload=(int)96, ssrc=(uint)953940170, timestamp-offset=(uint)3318556906, seqnum-offset=(uint)11061, a-framerate=(string)30" ! \
  rtph264depay ! \
  h264parse ! \
  mpegtsmux name=mp2ts_muxer ! \
  filesink location=./dumpUdp2Disk.mpg \
 \)

#check:
gst-launch-1.0 playbin uri=file://$(pwd)/dumpUdp2Disk.mpg


#} END Jetson-to desktop-stream, pure h264 via RTP & UDP
#-------------------------------------------------------------------------------------------------------------------------------------



#-------------------------------------------------------------------------------------------------------------------------------------
#{ Jetson-to desktop-stream, h264 wrapped by MPEG2 TS, sent  via RTP & UDP
#  maybe some help from:
#  https://stackoverflow.com/questions/38968396/streaming-a-local-mpeg-ts-file-to-udp-using-gstreamer


# Jetson h264 encoder and sender:
#   cam -> show on screen
#      \-> hardware encode h264 -> write to disk
#                              \-> stream per as RTP over UDP
GST_DEBUG=4 \
gst-launch-1.0 -vvvv -m \
  videotestsrc is-live=1 pattern=ball ! \
    video/x-raw,width=1280,height=720 ! \
  tee name=fork_raw_to_show_and_enc ! \
  queue ! \
  videoscale add-borders=TRUE ! \
  videoconvert ! \
  capsfilter name=scale ! \
  xvimagesink \
  fork_raw_to_show_and_enc. ! \
  queue ! \
  nvvidconv ! \
  nvv4l2h264enc ! \
  h264parse disable-passthrough=true ! \
  mpegtsmux name=mp2ts_muxer ! \
  tsparse set-timestamps=true ! \
  tee name=fork_mp2ts_to_disk_and_UDP ! \
  queue ! \
  filesink location=jetsontx2_h264_mpeg2ts.mpg -e \
  fork_mp2ts_to_disk_and_UDP. ! \
  queue ! \
  rtpmp2tpay ! \
  udpsink host=192.168.0.169 port=5001 sync=false

#check on jetson:
gst-launch-1.0 playbin uri=file://$(pwd)/jetsontx2_h264_mpeg2ts.mpg




#  Desktop receiver, both show on screen and dump to disk
GST_DEBUG=4 \
gst-launch-1.0 -vvv -m \
  udpsrc port=5001 caps='application/x-rtp, media=(string)video, clock-rate=(int)90000, encoding-name=(string)MP2T' ! \
  rtpjitterbuffer latency=200 ! \
  rtpmp2tdepay ! \
  tee name=fork_mp2ts_to_show_and_disk ! \
  queue ! \
  tsdemux ! \
  h264parse ! \
  avdec_h264 ! \
  videoconvert ! \
  xvimagesink sync=true \
  fork_mp2ts_to_show_and_disk. ! \
  queue ! \
  filesink location=./udp_mpegts.mpg 

#check on desktop:
gst-launch-1.0 playbin uri=file://$(pwd)/udp_mpegts.mpg






,\ payload\=\(int\)33\,\ seqnum-offset\=\(uint\)29794\,\ timestamp-offset\=\(uint\)3126696144\,\ ssrc\=\(uint\)2511193135
 pipeline \( latency=2000000000 \
 \)






gst-launch-1.0 filesrc location=dummy_h264.ts !
 tsparse set-timestamps=true ! video/mpegts ! tsdemux ! video/x-h264 ! h264parse disable-passthrough=true ! rtph264pay ! udpsink -v host=127.0.0.1 port=9999



