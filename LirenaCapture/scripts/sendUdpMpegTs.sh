#!/bin/bash

# Markus' laptop 
#receiverIP=192.168.0.101

# Markus' office PC
receiverIP=192.168.0.102

ninja -C builddir  && \
\
GST_DEBUG=2,appsrc:4,mpegtsmux:0,tcpserversink:3  \
__GL_SYNC_TO_VBLANK=0 \
./builddir/lirenaCapture \
    ${receiverIP} 5001 \
    --exposure=45  \
    $@ \
    
    
exit 0

    --output=localOutput_20.mpg \
    --localdisplay \

