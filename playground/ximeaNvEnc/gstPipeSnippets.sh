#original pipeline:
gst-launch-1.0 \
 videotestsrc pattern=ball ! \
    video/x-raw,width=1280,height=720 ! \
  queue max-size-buffers=2 leaky=2 ! \
  videoscale add-borders=TRUE ! \
  capsfilter name=scale ! \
  videoconvert ! \
  xvimagesink
  
  
  

gst-launch-1.0 \
 videotestsrc pattern=ball ! \
    video/x-raw,width=640,height=480 ! \
  queue max-size-buffers=2 leaky=2 ! \
  videoscale add-borders=TRUE ! \
  capsfilter name=scale ! \
  videoconvert ! \
  xvimagesink
  
  
gst-launch-1.0 \
  videotestsrc pattern=ball ! \
    video/x-raw,width=1280,height=720 ! \
  videoscale add-borders=TRUE ! \
  videoconvert ! \
    capsfilter name=scale ! \
  xvimagesink
  
  
## tee: just two windows first:
GST_DEBUG=4 \
gst-launch-1.0 \
  videotestsrc pattern=ball ! \
    video/x-raw,width=1280,height=720 ! \
  tee name=myfork ! \
  queue ! \
  videoscale add-borders=TRUE ! \
  videoconvert ! \
    capsfilter name=scale ! \
  xvimagesink \
  myfork. ! \
  queue ! \
  videoscale add-borders=TRUE ! \
  videoconvert ! \
    capsfilter name=scale2 ! \
  xvimagesink 
  
  
  
## tee1: show and h264-GPU-encode+dump to disk
GST_DEBUG=4 \
gst-launch-1.0 \
  videotestsrc pattern=snow ! \
    video/x-raw,width=1280,height=720 ! \
  \
  tee name=forkRaw2ShowNEnc ! \
  queue ! \
  videoscale add-borders=TRUE ! \
  videoconvert ! \
    capsfilter name=scale ! \
  xvimagesink \
  \
  forkRaw2ShowNEnc. ! \
  queue ! \
  nvvidconv ! \
  nvv4l2h264enc ! \
  h264parse !\
  mpegtsmux !\
  filesink location=myJetsonTx2NvEncedMp2Dump.mp4 -e

#verify
gst-launch-1.0 playbin uri=file://$(pwd)/myJetsonTx2NvEncedMp2Dump.mp4
  
  


## tee2: first split raw to show and h264-GPU-encode,
##       + then split into "dump to disk" and "stream as UDP"
GST_DEBUG=4 \
gst-launch-1.0 -vvvv -m \
  videotestsrc pattern=ball ! \
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
  tee name=forkEnc2DiskNUDP ! \
  queue ! \
  filesink location=myJetsonTx2NvEncedMp2Dump.mp4 -e \
  forkEnc2DiskNUDP. ! \
  queue ! \
  rtph264pay ! \
  udpsink host=192.168.0.169 port=5001
  
  #udpsink host=127.0.0.1 port=5001

"application/x-rtp\,\ media\=\(string\)video\,\ clock-rate\=\(int\)90000\,\ encoding-name\=\(string\)H264\,\ packetization-mode\=\(string\)1\,\ sprop-parameter-sets\=\(string\)\"Z0JAH5ZUAoAtyA\\\=\\\=\\\,aM48gA\\\=\\\=\"\,\ payload\=\(int\)96\,\ seqnum-offset\=\(uint\)10023\,\ timestamp-offset\=\(uint\)3297281466\,\ ssrc\=\(uint\)2953094042\,\ a-framerate\=\(string\)30"


#verify
gst-launch-1.0 playbin uri=file://$(pwd)/myJetsonTx2NvEncedMp2Dump.mp4




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




GST_DEBUG=3 gst-launch-1.0 \
 pipeline \( latency=1000000000 \
  udpsrc port=5001 caps="application/x-rtp, media=(string)video, clock-rate=(int)90000, encoding-name=(string)H264, packetization-mode=(string)1, profile-level-id=(string)42401f, sprop-parameter-sets=(string)\"Z0JAH5ZUAoAtyA\=\=\,aM48gA\=\=\", payload=(int)96, ssrc=(uint)953940170, timestamp-offset=(uint)3318556906, seqnum-offset=(uint)11061, a-framerate=(string)30" ! \
  rtph264depay ! h264parse ! nvv4l2decoder ! nvvidconv ! xvimagesink \
 \)
 
 
 
GST_DEBUG=3 gst-launch-1.0 \
 pipeline \( latency=1000000000 \
  udpsrc port=5001 caps="application/x-rtp, media=(string)video, clock-rate=(int)90000, encoding-name=(string)H264, packetization-mode=(string)1, profile-level-id=(string)42401f, sprop-parameter-sets=(string)\"Z0JAH5ZUAoAtyA\=\=\,aM48gA\=\=\", payload=(int)96, ssrc=(uint)953940170, timestamp-offset=(uint)3318556906, seqnum-offset=(uint)11061, a-framerate=(string)30" ! \
  rtph264depay ! h264parse ! mpegtsmux name=muxer ! filesink location=./dumpUdp2Disk.mpg \
 \)
 
gst-launch-1.0 playbin uri=file://$(pwd)/dumpUdp2Disk.mpg


