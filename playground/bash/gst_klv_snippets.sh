# This file is a broken dump of initial KLV-related experiments.
#  Only to be used to extraac som KLV-related gstreamer pipeline snippets, then to be deleted


#gst-build snippets

meson configure -Dgood=enabled -Dgst-plugins-good:jpeg=enabled build 

meson configure  -Dgood=enabled -Dgst-examples=enabled build

meson configure  -Dbase=enabled -Dgst-plugins-base:vorbis=enabled -Dgood=enabled -Dgst-plugins-good:jpeg=enabled -Dgst-examples=enabled build

meson configure  -Dbase=enabled -Dgst-plugins-base:vorbis=enabled -Dgood=enabled -Dgst-plugins-good:jpeg=enabled -Dgst-examples=enabled     -Dbad=enabled -Dgst-plugins-bad:nvcodec=enabled     build

#-Dgst-plugins-base:gstopengl=enabled

meson configure  -Dbase=enabled  -Dgst-plugins-base:vorbis=enabled -Dgood=enabled -Dgst-plugins-good:jpeg=enabled -Dgst-examples=enabled     -Dbad=enabled -Dgst-plugins-bad:nvcodec=enabled   build

jpeg
+----GstJpegEnc




meson build
ninja -C build




ninja -C build devenv




GST_DEBUG=*:3 gst-launch-1.0 v4l2src ! autovideosink

GST_DEBUG=*:3 gst-launch-1.0 playbin uri=https://www.freedesktop.org/software/gstreamer-sdk/data/media/sintel_trailer-480p.webm


GST_DEBUG=*:5 gst-launch-1.0 videotestsrc num-buffers=50 ! video/x-raw, framerate='(fraction)'5/1 ! jpegenc ! avimux ! filesink location=mjpeg.avi


GST_DEBUG=*:5 .build/subprojects/gst-examples/playback/player/gst-play/gst-play ../../mjpeg.avi


build/subprojects/gst-examples/playback/player/gst-play/gst-play ../../mjpeg.avi

meson test -C build --list





## -------------------------------------------------------------------------
# http://gstreamer-devel.966125.n4.nabble.com/Muxing-h264-video-with-sparse-klv-with-Mpegtsmux-td4696089.html

## direct show without KLV stuff
gst-launch-1.0 -vm \
  udpsrc uri=udp://127.0.0.1:5001 ! \
    application/x-rtp,encoding-name=H264,payload=96 ! \
  rtph264depay ! \
    video/x-h264,stream-format=byte-stream, alignement=nal ! \
  avdec_h264 ! \
  autovideosink


## dump from UDP to disk without KLV stuff
gst-launch-1.0 -vm \
  udpsrc uri=udp://127.0.0.1:5001 ! \
    application/x-rtp,encoding-name=H264,payload=96 ! \
  rtph264depay ! \
    video/x-h264,stream-format=byte-stream, alignement=nal ! \
  filesink location=./test_noKLV.mpg 

## verify "dump to disk without KLV stuff"
gst-launch-1.0 playbin uri=file://$(pwd)/test_noKLV.mpg




## building up to (below):
## final pipeline to test (will have problems when kalv steream breaks/pauses...)

# sender of video data
gst-launch-1.0 -vm  \
  videotestsrc pattern=ball is-live=true ! \
    video/x-raw,width=300,height=300 ! \
  x264enc tune=4 ! \
  rtph264pay ! \
  udpsink host=127.0.0.1 port=5001

# receiver of both video and KLV data
gst-launch-1.0 -vm \
  udpsrc uri=udp://127.0.0.1:5001 ! \
    application/x-rtp,encoding-name=H264,payload=96 ! \
  rtph264depay ! \
    video/x-h264,stream-format=byte-stream, alignement=nal ! \
  muxer. \
  \
  udpsrc uri=udp://127.0.0.1:5000 ! \
    meta/x-klv,parsed=true ! \
  muxer. \
  \
  mpegtsmux name=muxer ! \
  filesink location=./test_withKLV.mpg 






## ------------------------------------------------------------------------
# trying to dump the KLV data from the gstr pipeline view a demuxer and a filesink...

# make gst dump the pipeline to dot format
export GST_DEBUG_DUMP_DOT_DIR=/home/markus/devel/streaming/playground/pipelinevisdump



# this file has KLV data and can be played 
gst-launch-1.0 playbin uri=file://$(pwd)/dayflight.mpg

#visualize playbin:
dot -Tpdf  pipelinevisdump/0.00.00.143527570-gst-launch.PAUSED_PLAYING.dot > test.pdf
evince test.pdf

#try to reconstruct the relevant parts of playbins pipline from the visualization:
# .. not started yet ..


#SUCCESS: Dump only KLV from MPEG TS file into klv file: 
GST_DEBUG=3 \
gst-launch-1.0 -vm  \
  filesrc location=$(pwd)/dayflight.mpg ! \
  tsdemux name=mydemuxer \
  \
  mydemuxer. ! \
    meta/x-klv,parsed=true ! \
  filesink location=./klvOnly_fromLocalMpgFile.klv


# SUCCESS: SEND out only KLV from file over UDP
GST_DEBUG=3 \
gst-launch-1.0 -vm  \
  filesrc location=$(pwd)/dayflight.mpg ! \
  tsdemux name=mydemuxer \
  \
  mydemuxer. ! \
    meta/x-klv,parsed=true ! \
  udpsink host=127.0.0.1 port=5001



#SUCCESS: RECEIVE only KLV over UDP, dump to KLV file
GST_DEBUG=3 \
gst-launch-1.0 -vm  \
  udpsrc uri=udp://127.0.0.1:5001 ! \
    meta/x-klv,parsed=true ! \
  filesink location=./klvOnly_fromUDP.klv




#medium success: (replay is much faster...): Dump only Video stuff  from MPEG TS file into mpg file: 
GST_DEBUG=3 \
gst-launch-1.0 -vm  \
  filesrc location=$(pwd)/dayflight.mpg ! \
  tsdemux name=mydemuxer \
  \
  mydemuxer. ! \
    video/x-h264 ! \
  filesink location=./videoOnly_fromLocalMpgFile.mpg

## verify "dump only video to disk without KLV stuff"
gst-launch-1.0 playbin uri=file://$(pwd)/videoOnly_fromLocalMpgFile.mpg
## just for comparison: replay WITH klv data is much faster:
#gst-launch-1.0 playbin uri=file://$(pwd)/dayflight.mpg


# SUCCESS: demux to filter out only video, then reumux only the video into ts (i.e. just wrap)
GST_DEBUG=3 \
gst-launch-1.0 -vm  \
  filesrc location=$(pwd)/dayflight.mpg ! \
  tsdemux name=mydemuxer \
  \
  mydemuxer. ! \
    video/x-h264 ! \
  muxer. \
  \
  mpegtsmux name=muxer ! \
  filesink location=./videoOnly_fromLocalMpgFile_ts.mpg

## verify "dump only video to disk without KLV stuff"
gst-launch-1.0 playbin uri=file://$(pwd)/videoOnly_fromLocalMpgFile_ts.mpg
## just for comparison: replay WITH klv data is much faster:
#gst-launch-1.0 playbin uri=file://$(pwd)/dayflight.mpg




#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
# PURE H264 VIDEO, WRAPPED IN MPEG2-TS container, payloaded into RTP, sent over UDP,
# THEN RECEIVED VIA UDP and REVERSED 
##SUCCESS: SEND only non-KLV-file (video only) over udp, WRAPPED in mpeg2 ts container
GST_DEBUG=3 \
gst-launch-1.0 -vvv -m  \
  filesrc location=$(pwd)/videoOnly_fromLocalMpgFile_ts.mpg ! \
  tsdemux name=mydemuxer \
  \
  mydemuxer. ! \
    video/x-h264 ! \
  muxer. \
  \
  mpegtsmux name=muxer ! \
  rtpmp2tpay ! \
  queue ! \
  udpsink host=127.0.0.1 port=5002


#send completely, i.e. with all metadata
GST_DEBUG=4 \
gst-launch-1.0 -vvv -m  \
  filesrc location=$(pwd)/videoOnly_fromLocalMpgFile_ts.mpg ! \
  tsparse set-timestamps=true ! \
  rtpmp2tpay ! \
  queue ! \
  udpsink host=127.0.0.1 port=5002 sync=1


# only receive and show
GST_DEBUG=3 \
gst-launch-1.0 -vm  \
  udpsrc uri=udp://127.0.0.1:5002  ! \
    application/x-rtp, media=video, clock-rate=90000, encoding-name=MP2T, payload=33, seqnum-offset=6185, timestamp-offset=1922319236, ssrc=4129585114 ! \
  rtpmp2tdepay ! \
  tsparse ! \
  queue ! \
  tsdemux name=mydemuxer \
  \
  mydemuxer. ! \
    video/x-h264 ! \
  h264parse ! \
  avdec_h264 ! \
  videoconvert ! \
  autovideosink


# only receive and write to disk
gst-launch-1.0 -vm  \
  udpsrc uri=udp://127.0.0.1:5002 ! \
    application/x-rtp, media=video, clock-rate=90000, encoding-name=MP2T, payload=33, seqnum-offset=6185, timestamp-offset=1922319236, ssrc=4129585114 ! \
  rtpmp2tdepay ! \
  queue ! \
  filesink location=./videoh264Only_ts_fromUDP.mpg




## verify "dump only video to disk without KLV stuff"
gst-launch-1.0 playbin uri=file://$(pwd)/videoh264Only_ts_fromUDP.mpg
#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
### TCP experiment
### Works! needed latency

#send completely, i.e. with all metadata
GST_DEBUG=4 \
gst-launch-1.0 -vvv -m  \
  filesrc location=$(pwd)/dayflight.mpg ! \
  tsparse set-timestamps=true ! \
  queue ! \
  tcpserversink port=5002


# only receive and show
GST_DEBUG=3 \
gst-launch-1.0 -vm  \
 pipeline. \( latency=2000000000 \
  tcpclientsrc host=127.0.0.1 port=5002  ! \
  tsparse ! \
  queue ! \
  tsdemux name=mydemuxer \
  \
  mydemuxer. ! \
    video/x-h264 ! \
  h264parse ! \
  avdec_h264 ! \
  videoconvert ! \
  autovideosink \
 \) 




#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
# UDP experiment 2: this time with latency:

#send completely, i.e. with all metadata
GST_DEBUG=4 \
gst-launch-1.0 -vvv -m  \
  filesrc location=$(pwd)/videoOnly_fromLocalMpgFile_ts.mpg ! \
  tsparse ! \
  tsdemux ! mpegtsmux ! \
  rtpmp2tpay ! \
  queue ! \
  udpsink host=127.0.0.1 port=5002 

sync=1
tsparse set-timestamps=true ! \

# only receive and show
GST_DEBUG=3 \
gst-launch-1.0 -vm  \
 pipeline. \( latency=1000000000 \
  udpsrc uri=udp://127.0.0.1:5002  ! \
    application/x-rtp, media=video, clock-rate=90000, encoding-name=MP2T, payload=33, seqnum-offset=6185, timestamp-offset=1922319236, ssrc=4129585114 ! \
  rtpmp2tdepay ! \
  tsparse ! \
  queue max-size-bytes=3000000000 ! \
  tsdemux name=mydemuxer \
  \
  mydemuxer. ! \
    video/x-h264 ! \
  h264parse ! \
  avdec_h264 ! \
  videoconvert ! \
  autovideosink \
 \) 

#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
# UDP experiment 3: this time raw video: works, but only for small resolutions
gst-launch-1.0 -vvv -m  \
  videotestsrc pattern=ball ! \
  videoconvert ! \
    video/x-raw,width=128,height=128,format=BGR ! \
  rtpvrawpay ! \
    application/x-rtp,payload=96 ! \
  udpsink host=192.168.0.169 port=5002 sync=1

# only receive and show
GST_DEBUG=5 \
gst-launch-1.0 -vm  \
 pipeline. \( latency=500000000 \
  udpsrc uri=udp://192.168.0.169:5002 caps="application/x-rtp, media=(string)video, clock-rate=(int)90000, encoding-name=(string)RAW, sampling=(string)BGR, depth=(string)8, width=(string)128, height=(string)128, colorimetry=(string)SMPTE240M, payload=(int)96, ssrc=(uint)3795765443, timestamp-offset=(uint)1160118158, seqnum-offset=(uint)16715, a-framerate=(string)30" ! \
  rtpvrawdepay ! \
  videoconvert ! \
  queue max-size-bytes=1000000000  ! \
  xvimagesink \
 \) 





GST_DEBUG=5 gst-launch-1.0 udpsrc port=5003 ! application/x-rtp, encoding-name=H264, payload=96 ! rtph264depay ! h264parse ! avdec_h264 ! autovideosink

gst-launch-1.0  videotestsrc  do-timestamp=true !   autovideoconvert  !   x264enc  !   queue !   h264parse  config-interval=1 !   rtph264pay pt=96 !   udpsink host=192.168.0.169 port=5003 







GST_DEBUG=4 gst-launch-1.0 \
 pipeline \( latency=1000000000 \
  udpsrc port=5001 caps="application/x-rtp, media=(string)video, clock-rate=(int)90000, encoding-name=(string)H264, packetization-mode=(string)1, profile-level-id=(string)42401f, sprop-parameter-sets=(string)\"Z0JAH5ZUAoAtyA\=\=\,aM48gA\=\=\", payload=(int)96, ssrc=(uint)953940170, timestamp-offset=(uint)3318556906, seqnum-offset=(uint)11061, a-framerate=(string)30" ! \
  rtph264depay ! h264parse ! mpegtsmux name=muxer ! filesink location=./dumpUdp2Disk.mpg \
 \)
 
gst-launch-1.0 playbin uri=file://$(pwd)/dumpUdp2Disk.mpg













##FAILED (result file empty)
: Write an Mpeg2 TS file that contains only KLV data
## same style like above with video: full_mp2 --> demux --> KLV --> mux --> file
GST_DEBUG=3 \
gst-launch-1.0 -vm  \
  filesrc location=$(pwd)/dayflight.mpg ! \
  tsdemux name=mydemuxer \
  \
  mydemuxer. ! \
    meta/x-klv,parsed=true ! \
  muxer. \
  \
  mpegtsmux name=muxer ! \
  filesink location=./klvOnly_fromLocalMpgFile_ts.mpg




##SUCCESS (in the beginning, 288 bytes are written, later it is more: 977): 
## Send only meta data via udp, ignore the rest 
## (except the time stamps, but those are in the meta date themselves...)
## same style like above with video: full_mp2 --> demux --> KLV --> mux --> file
GST_DEBUG=3 \
gst-launch-1.0 -vm  \
  filesrc location=$(pwd)/dayflight.mpg ! \
  tsdemux name=mydemuxer \
  \
  mydemuxer. ! \
    meta/x-klv,parsed=true ! \
  udpsink host=127.0.0.1 port=5001

GST_DEBUG=3 \
gst-launch-1.0 -vm  \
  udpsrc uri=udp://127.0.0.1:5001 ! \
    meta/x-klv,parsed=true ! \
  filesink location=./klvOnly_fromUDP.klv








# ~~~~~~~~~~~~~~~







## experiment: maybe final receiver: stream different parts of the daylfight file
## from diffent procecces and out of sync, combine both streams in the receiver
## and write a metadata-enhanced video file:

##sender of KLV 1: fails when sending second of 3 datasets
GST_DEBUG=4 \
gst-launch-1.0 -vm  \
  filesrc location=$(pwd)/dayflight.mpg ! \
  tsdemux name=mydemuxer \
  \
  mydemuxer. ! \
    meta/x-klv,parsed=true ! \
  udpsink host=127.0.0.1 port=5001



##sender of video (h264, wrapped in MPEG2 TS):
GST_DEBUG=4 \
gst-launch-1.0 -vvv -m  \
  filesrc location=$(pwd)/videoOnly_fromLocalMpgFile_ts.mpg ! \
  tsdemux name=mydemuxer \
  \
  mydemuxer. ! \
    video/x-h264 ! \
  muxer. \
  \
  mpegtsmux name=muxer ! \
  rtpmp2tpay ! \
  queue ! \
  udpsink host=127.0.0.1 port=5002


# receiver of both video and KLV data 1: fails when sending second of 3 datasets
GST_DEBUG=5 \
gst-launch-1.0 -vm \
  udpsrc uri=udp://127.0.0.1:5001 ! \
    meta/x-klv,parsed=true ! \
  queue ! \
  muxer. \
  \
  udpsrc uri=udp://127.0.0.1:5002 ! \
    application/x-rtp, media=video, clock-rate=90000, encoding-name=MP2T, payload=33, seqnum-offset=6185, timestamp-offset=1922319236, ssrc=4129585114 ! \
  rtpjitterbuffer ! \
  rtpmp2tdepay ! \
  tsdemux name=mydemuxer \
  \
  mydemuxer. ! \
    video/x-h264 ! \
  queue max-size-buffers=10000 max-size-time=100000000000 max-size-bytes=1000000000 ! \
  muxer. \
  \
  mpegtsmux name=muxer ! \
  filesink location=./klvAndVideo_ts_fromUDP.mpg 





## ******* next try: *************************

##sender of KLV 2: use rtpklvpay
GST_DEBUG=4 \
gst-launch-1.0 -vvvv -m  \
  filesrc location=$(pwd)/dayflight.mpg ! \
  tsdemux name=mydemuxer ! \
  rtpklvpay ! \
  udpsink  host=127.0.0.1 port=5001



##sender of video (h264, wrapped in MPEG2 TS): like above
GST_DEBUG=4 \
gst-launch-1.0 -vvv -m  \
  filesrc location=$(pwd)/videoOnly_fromLocalMpgFile_ts.mpg ! \
  tsdemux name=mydemuxer \
  \
  mydemuxer. ! \
    video/x-h264 ! \
  muxer. \
  \
  mpegtsmux name=muxer ! \
  rtpmp2tpay ! \
  queue ! \
  udpsink host=127.0.0.1 port=5002


# receiver of both video and KLV data 2: use rtpjitterbuffer and rtpklvdepay
GST_DEBUG=5 \
gst-launch-1.0 -vm \
  udpsrc uri=udp://127.0.0.1:5001 caps='application/x-rtp, media=(string)application, clock-rate=(int)90000, encoding-name=(string)SMPTE336M, payload=(int)96, ssrc=(uint)2911320883, timestamp-offset=(uint)3103523187, seqnum-offset=(uint)21823' ! \
  rtpjitterbuffer latency=1 do-lost=true max-dropout-time=50 ! \
  rtpklvdepay ! \
  muxer. \
  \
  udpsrc uri=udp://127.0.0.1:5002 ! \
    application/x-rtp, media=video, clock-rate=90000, encoding-name=MP2T, payload=33, seqnum-offset=6185, timestamp-offset=1922319236, ssrc=4129585114 ! \
  rtpjitterbuffer do-lost=false ! \
  rtpmp2tdepay ! \
  tsdemux name=mydemuxer \
  \
  mydemuxer. ! \
    video/x-h264 ! \
  muxer. \
  \
  mpegtsmux name=muxer ! \
  filesink location=./klvAndVideo_ts_fromUDP.mpg 


# receiver of both video and KLV data 3: also use tee element to also watch incoming stuff
 

#TODO



GST_DEBUG=3 gst-launch-1.0 \
 pipeline \( latency=1000000000 \
  udpsrc port=5001 caps="application/x-rtp, media=(string)video, clock-rate=(int)90000, encoding-name=(string)H264, packetization-mode=(string)1, profile-level-id=(string)42401f, sprop-parameter-sets=(string$
  rtph264depay ! h264parse ! mpegtsmux name=muxer ! filesink location=./dumpUdp2Disk.mpg \
 \)
 
gst-launch-1.0 playbin uri=file://$(pwd)/dumpUdp2Disk.mpg





##################
## verify: watch video
gst-launch-1.0 playbin uri=file://$(pwd)/klvAndVideo_ts_fromUDP.mpg
## verify: dump KLV data
GST_DEBUG=3 \
gst-launch-1.0 -vm  \
  filesrc location=$(pwd)/klvAndVideo_ts_fromUDP.mpg ! \
  tsdemux name=mydemuxer \
  \
  mydemuxer. ! \
    meta/x-klv,parsed=true ! \
  filesink location=./klvOnly_fromRecombinedUDPMpgFile.klv
## verify: view klv file:
ghex klvOnly_fromRecombinedUDPMpgFile.klv








######################################################################

notes, snippets:

GST_EVENT_GAP
queue max-size-buffers=10000 max-size-time=100000000000 max-size-bytes=1000000000  ! \
rtpjitterbuffer latency=30 do-lost=true max-dropout-time=100 ! \




####---------------------- failed stuff below ..........-----------------------



##experiment: SEND only non-KLV-file (video only) over udp
GST_DEBUG=3 \
gst-launch-1.0 -vvvv -m  \
  filesrc location=$(pwd)/videoOnly_fromLocalMpgFile.mpg ! \
  h264parse ! \
    video/x-h264 ! \
  rtph264pay sprop-parameter-sets=Z00AKZp0AoAtyA\\\=\\\=\\\,aO44gA\\\=\\\= ! \
  udpsink host=127.0.0.1 port=5002


# fails so send more then the first frame....
GST_DEBUG=3 \
gst-launch-1.0 -vvvv -m  \
  filesrc location=$(pwd)/videoOnly_fromLocalMpgFile.mpg ! \
  h264parse ! \
    video/x-h264 ! \
  queue ! \
  rtph264pay ! \
  udpsink host=127.0.0.1 port=5002



##experiment failed:: RECEIVE only non-KLV-file (video only) over udp and show
#GST_DEBUG=3 \
#gst-launch-1.0 -vm  \
#   udpsrc uri=udp://127.0.0.1:5002 caps = "application/x-rtp\,\ media\=\(string\)video\,\ clock-rate\=\(int\)90000\,\ #encoding-name\=\(string\)H264\,\ packetization-mode\=\(string\)1\,\ profile-level-id\=\(string\)4d0029\,\ sprop-#parameter-sets\=\(string\)\"Z00AKZp0AoAtyA\\\=\\\=\\\,aO44gA\\\=\\\=\"\,\ payload\=\(int\)96\,\ ssrc\=\(uint\)2382037038\,\ timestamp-offset\=\(uint\)3381899919\,\ seqnum-offset\=\(uint\)29298" ! \
#  rtph264depay sprop-parameter-sets=Z00AKZp0AoAtyA\\\=\\\=\\\,aO44gA\\\=\\\=, ! \
#  avdec_h264 ! \
#  autovideosink


##experiment failed: RECEIVE only non-KLV-file (video only) over udp and show
GST_DEBUG=3 \
gst-launch-1.0 -vm  \
   udpsrc uri=udp://127.0.0.1:5002 ! \
application/x-rtp, media=video, clock-rate=90000, encoding-name=H264, packetization-mode=1, profile-level-id=4d0029,  payload=96, ssrc=3294599955, timestamp-offset=908585591, seqnum-offset=13291 ! \
  rtph264depay ! \
  avdec_h264 ! \
  autovideosink


GST_DEBUG=3 \
gst-launch-1.0 -vm  \
   udpsrc uri=udp://127.0.0.1:5002 ! \
application/x-rtp, media=video, clock-rate=90000, encoding-name=H264, packetization-mode=1, profile-level-id=4d0029,  payload=96, ssrc=3294599955, timestamp-offset=908585591, seqnum-offset=13291 ! \
  rtph264depay ! \
  queue ! \
  muxer. \
  \
  mpegtsmux name=muxer ! \
  filesink location=./videoh264Only_fromUDP.mpg 

## verify "dump only video from UDP to disk without KLV stuff"
gst-launch-1.0 playbin uri=file://$(pwd)/videoh264Only_fromUDP.mpg



#snippet for h264 decode and show (not tested yet)
 #queue ! h264parse ! avdec_h264 ! videoconvert ! videoscale ! ximagesink






