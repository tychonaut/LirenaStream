
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



gst-launch-1.0 -v -m \
  udpsrc port=5001 caps='application/x-rtp, media=(string)video, clock-rate=(int)90000, encoding-name=(string)MP2T' ! \
  rtpjitterbuffer latency=50 ! \
  rtpmp2tdepay ! \
  tsdemux name=myTsDemux  \
  \
  myTsDemux. ! \
  h264parse ! \
  tee name=fork_h264_to_show_and_mp2ts   \
  \
  fork_h264_to_show_and_mp2ts. ! \
  queue !    \
  avdec_h264 !   \
  videoconvert !   \
  xvimagesink sync=false   \
  \
  fork_h264_to_show_and_mp2ts. ! \
  queue ! \
  mpegtsmux name=mp2ts_muxer !  \
  filesink location=./udp_rtpmp2tdepay_klv1.mpg 




################################################################################
## new stuff after C- based KLV injection works (at least no freeze) (2021/03/31)




# Receive mp2ts per udp, dump directly to disk and show on screen:
# works, though playback of mpg file that contains KLV must be with VLC 
# (playbin cannot decode/ignore it for some reason)
gst-launch-1.0 -v -m \
  udpsrc port=5001 caps='application/x-rtp, media=(string)video, clock-rate=(int)90000, encoding-name=(string)MP2T' ! \
  rtpjitterbuffer latency=50 ! \
  rtpmp2tdepay ! \
  tee name=fork_mp2ts_to_disk_and_show \
  \
  fork_mp2ts_to_disk_and_show. ! \
  queue !   \
  filesink location=./udp_rtpmp2tdepay_klv4.mpg \
  \
  fork_mp2ts_to_disk_and_show. ! \
  queue !   \
  tsdemux name=myTsDemux  \
  \
  myTsDemux. ! \
  h264parse ! \
  queue !    \
  avdec_h264 !   \
  videoconvert !   \
  xvimagesink sync=false  




# extract KLV data from previously dumped mp2ts video+KLV-file to KLV-file
# works
gst-launch-1.0 -vm  \
  filesrc location=$(pwd)/udp_rtpmp2tdepay_klv2.mpg ! \
  tsdemux name=mydemuxer \
  \
  mydemuxer. ! \
    meta/x-klv,parsed=true ! \
  filesink location=./klvOnly_fromLocalMpgFile1.klv



# replay only video part 
# from previously dumped mp2ts video+KLV-file 
# works, though xvimagesink's  "sync=false" makes too fast playback,
#        and in the end, the error:
# 0:00:04.074952454 98071 0x7f9fb00d0600 ERROR                  libav :0:: negative number of zero coeffs at 8 94
# 0:00:04.075006095 98071 0x7f9fb00d0600 ERROR                  libav :0:: error while decoding MB 8 94
GST_DEBUG=2 \
gst-launch-1.0 -vm  \
  filesrc location=/home/markus/devel/streaming/LirenaStream/playground/udp_rtpmp2tdepay_klv3.mpg ! \
  tsdemux name=mydemuxer \
  \
  mydemuxer. ! \
    video/x-h264 ! \
  h264parse ! \
  queue !    \
  avdec_h264 !   \
  videoconvert !   \
  xvimagesink 






# receive, show video, dump both video+klv to disk and also klv-only
# works
gst-launch-1.0 -v -m \
  udpsrc port=5001 buffer-size=100000000 \
      caps='application/x-rtp, media=(string)video, clock-rate=(int)90000, encoding-name=(string)MP2T' ! \
  rtpjitterbuffer latency=50 ! \
  rtpmp2tdepay ! \
  tee name=fork_mp2ts_to_disk_and_show \
  \
  fork_mp2ts_to_disk_and_show. ! \
  queue !   \
  filesink location=./udp_rtpmp2tdepay_klv10.mpg \
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
  xvimagesink sync=false  \
  \
  myTsDemux. ! \
    meta/x-klv,parsed=true ! \
  queue ! \
  filesink location=./klvOnly_from_UDP_mp2ts_10.klv



# replay video and extract KLV data 
# from previously dumped mp2ts video+KLV-file to KLV-file
# works, but  in the end, corupted stuff is reported:
# 0:00:16.246408611 98634 0x7f0ce4002350 ERROR                  libav :0:: corrupted macroblock 7 96 (total_coeff=-1)
# 0:00:16.246463961 98634 0x7f0ce4002350 ERROR                  libav :0:: error while decoding MB 7 96
# FAILS now, although they once worked, and still work for each demuxed stream (video and KLV) alone
# suspected reason: corrupted VIDEO frames due to UDP losses are dropped,
# but KLV buffers are NOT!
# The way tsdemux works is "ASYNC (non-1:1) relation" between KLV and video frame buffers!
# So with this, this goues out of sync pretty much all the time.
# But tsdemux blocks if thre is no 1:1 relation. epic fail:
# http://gstreamer-devel.966125.n4.nabble.com/Tsdemux-synchronous-file-klv-metadata-extraction-failed-td4677807.html 
GST_DEBUG=2 \
gst-launch-1.0 -vm  \
  filesrc location=/home/markus/devel/streaming/LirenaStream/playground/vlcStreamDump6_allElemStreams.ts ! \
  tsdemux name=mydemuxer \
  \
  mydemuxer. ! \
    video/x-h264 ! \
  h264parse ! \
  queue !    \
  avdec_h264 !   \
  videoconvert !   \
  xvimagesink \
  \
  mydemuxer. ! \
    meta/x-klv,parsed=true ! \
  queue ! \
  filesink location=./vlcStreamDump6_allElemStreams.klv


GST_DEBUG=2 \
gst-launch-1.0 -vm  \
  filesrc location=/home/markus/devel/streaming/LirenaStream/playground/vlcStreamDump6_allElemStreams.ts ! \
  tsdemux name=mydemuxer \
  \
  mydemuxer. ! \
    meta/x-klv,parsed=true ! \
  queue ! \
  filesink location=./vlcStreamDump6_allElemStreams.klv



GST_DEBUG=2 \
gst-launch-1.0 -vm  \
  filesrc location=/home/markus/devel/streaming/LirenaStream/playground/vlcStreamDump6_allElemStreams.ts ! \
  tsdemux name=mydemuxer \
  \
  mydemuxer. ! \
    video/x-h264 ! \
  h264parse ! \
  queue !    \
  avdec_h264 !   \
  videoconvert !   \
  xvimagesink 



###----------------------------------------



#TCP experiment 1:
# receive, show video
# this one works, though with low fps and high latency
GST_DEBUG=2 \
__GL_SYNC_TO_VBLANK=0 \
gst-launch-1.0 -v -m \
  \
  tcpclientsrc host=192.168.0.124 port=5001  timeout=0   ! \
   application/x-rtp-stream,encoding-name=MP2T,media=video,clock-rate=90000 ! \
  rtpstreamdepay ! \
  \
  rtpmp2tdepay ! \
  tsparse set-timestamps=true  ! \
  tee name=fork_mp2ts_to_disk_and_show \
    fork_mp2ts_to_disk_and_show. ! \
    queue !   \
    tsdemux name=myTsDemux  \
    \
      myTsDemux. ! \
        video/x-h264 ! \
      h264parse ! \
        video/x-h264,streaming-format=byte-stream ! \
      queue !    \
      avdec_h264 !   \
      videoconvert !   \
      xvimagesink sync=false



#TCP experiment 2:
# receive, dump combined video+klv to disk 
# works with VLC and streamer replay, see below
GST_DEBUG=2 \
__GL_SYNC_TO_VBLANK=0 \
gst-launch-1.0 -v -m \
  \
  tcpclientsrc host=192.168.0.124 port=5001  timeout=0   ! \
   application/x-rtp-stream,encoding-name=MP2T,media=video,clock-rate=90000 ! \
  rtpstreamdepay ! \
  \
  rtpmp2tdepay ! \
  tsparse set-timestamps=true  ! \
  tee name=fork_mp2ts_to_disk_and_show \
    fork_mp2ts_to_disk_and_show. ! \
    queue !   \
    filesink location=./tcp_mp2ts_h264_klv_30.mpg



# replay video part of tcp-dumped file: 
# works, though again some errors:
#0:00:06.528460976 124560 0x7f9a64001a30 ERROR                  libav :0:: corrupted macroblock 19 95 (total_coeff=-1)
#0:00:06.528514546 124560 0x7f9a64001a30 ERROR                  libav :0:: error while decoding MB 19 95
GST_DEBUG=2 \
__GL_SYNC_TO_VBLANK=0 \
gst-launch-1.0 -vm  \
  filesrc location=/home/markus/devel/streaming/LirenaStream/playground/tcp_mp2ts_h264_klv_30.mpg ! \
  tsdemux name=mydemuxer \
  \
  mydemuxer. ! \
    video/x-h264 ! \
  h264parse ! \
  queue !    \
  avdec_h264 !   \
  videoconvert !   \
  xvimagesink 

# dump KLV- part of tcp-dumped file:
# works 
GST_DEBUG=2 \
gst-launch-1.0 -vm  \
  filesrc location=/home/markus/devel/streaming/LirenaStream/playground/tcp_mp2ts_h264_klv_30.mpg  ! \
  tsdemux name=mydemuxer \
  \
  mydemuxer. ! \
    meta/x-klv,parsed=true ! \
  queue ! \
  filesink location=./tcp_filedump_mp2ts_h264_klv_30.klv






#TCP experiment 3:
# receive TCP, dump KLV directly to  file
# 
GST_DEBUG=2 \
__GL_SYNC_TO_VBLANK=0 \
gst-launch-1.0 -v -m \
  \
  tcpclientsrc host=192.168.0.124 port=5001  timeout=0   ! \
   application/x-rtp-stream,encoding-name=MP2T,media=video,clock-rate=90000 ! \
  rtpstreamdepay ! \
  \
  rtpmp2tdepay ! \
  tsparse set-timestamps=true  ! \
  tee name=fork_mp2ts_to_disk_and_show \
    \
    fork_mp2ts_to_disk_and_show. ! \
      queue max-size-time=5000000000 !   \
      tsdemux name=myTsDemux  \
      \
        myTsDemux. ! \
            video/x-h264 ! \
          h264parse ! \
            video/x-h264! \
          queue max-size-time=5000000000 !   \
          avdec_h264 !   \
          videoconvert !   \
          xvimagesink sync=false  
#only works for single-output

\
    \
    fork_mp2ts_to_disk_and_show. ! \
      queue max-size-time=5000000000 !   \
      filesink location=./tcp_mp2ts_h264_klv_30.mpg



    \
      myTsDemux. ! \
        meta/x-klv,parsed=true ! \
      queue ! \
      filesink location=./tcp_streamdump_mp2ts_h264_klv_30.klv


 video/x-h264,streaming-format=byte-stream ! \
sync=false 















#try to workaround the continuity-mismatch-problem by forking ans recombining,
# hoing that invalid frames will get dropped also on the KLV side:
# receive, show video, dump both video+klv to disk and also klv-only
# FAIL!
GST_DEBUG=2 \
__GL_SYNC_TO_VBLANK=0 \
gst-launch-1.0 -v -m \
  udpsrc port=5001 buffer-size=100000000 \
      caps='application/x-rtp, media=(string)video, clock-rate=(int)90000, encoding-name=(string)MP2T' ! \
  rtpjitterbuffer latency=50 ! \
  rtpmp2tdepay ! \
  tee name=fork_mp2ts_to_disk_and_show \
  \
  fork_mp2ts_to_disk_and_show. ! \
  queue !   \
  filesink location=./udp_rtpmp2tdepay_klvAndVid_dump_1.mpg \
  \
  fork_mp2ts_to_disk_and_show. ! \
  queue !   \
  tsparse ! \
  tsdemux name=myTsDemux parse-private-sections=TRUE  \
  \
  myTsDemux. ! \
    video/x-h264 ! \
  h264parse ! \
  tee name=fork_h264_to_show_and_remux \
  \
  fork_h264_to_show_and_remux. ! \
  queue ! \
  avdec_h264 ! \
  videoconvert ! \
  xvimagesink sync=false  \
  \
  fork_h264_to_show_and_remux. ! \
  queue ! \
  mpegtsmux name=myTsRemuxer  \
  \
  myTsRemuxer. ! \
  tsparse ! \
  queue ! \
  filesink location=./udp_rtpmp2tdepay_klvAndVid_forkedAndRecombined_1.mpg \
  \
  myTsDemux. ! \
    meta/x-klv,parsed=true ! \
  tee name=fork_klv_to_dump_and_remux \
  \
  fork_klv_to_dump_and_remux. ! \
  queue ! \
  filesink location=./udp_rtpmp2tdepay_klvAndVid_dump_1.klv \
  \
  fork_klv_to_dump_and_remux. ! \
  queue ! \
  myTsRemuxer. 
  

