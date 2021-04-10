#!/bin/bash

# launch script / snippets with common param combinations

# launch MagewellEco without GUI, but with local show&dump

ninja -C builddir

GST_DEBUG=3,appsrc:6 \
__GL_SYNC_TO_VBLANK=0 \
./builddir/lirenaCapture \
    192.168.1.169 5001 \
    -d MagewellEco \
    --localdisplay \
    -o mydump.mpg \
    $@
    
exit 0




# snippet to test CLI param parsing:
GST_DEBUG=3,appsrc:6 \
__GL_SYNC_TO_VBLANK=0 \
./builddir/lirenaCapture \
    192.168.1.169 5001 \
    -d MagewellEco \
    -x 4096 -y 2160 \
    -f 24 \
    --localdisplay \
    --localGUI \
    -c \
    -e 25 \
    -o mydump.mpg \
    -t \
    
exit 0
