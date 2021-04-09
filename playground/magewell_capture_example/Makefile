TARGET_CAPTUREBYINPUT := CaptureByInput

#LOCAL_CFLAGS += -I../../../../Include
LOCAL_CFLAGS += -I../../deps/MagewellSDK/include/ -I./include

CC = gcc
CXX = g++
LD = ld

CFLAGS += -O0 \
          -Wall \
          -Wextra \
          -Wno-unused-parameter \
          -Wno-sign-compare \
          -Wno-unused-variable \
          -Wno-unused-but-set-variable \
          -Wno-missing-field-initializers  \
          -fPIC \
          $(LOCAL_CFLAGS) \
	  

ifeq ($(ARCH),)
ARCH =$(shell uname -m)
endif

ifeq ($(ARCH),x86_64)
override ARCH = x64
else
ifeq ($(ARCH),i386)
CFLAGS += -m32
else
ifeq ($(ARCH),i686)
CFLAGS += -m32
override ARCH = i386
else
ifeq ($(ARCH),aarch64)
override ARCH = arm64
else
override ARCH = arm32
endif
endif
endif
endif


CLIB =  -lpthread -ldl -ludev -lasound -lv4l2
#MWCAPTURE_LIBA = ../../../../Lib/$(ARCH)/libMWCapture.a
MWCAPTURE_LIBA = ../../deps/MagewellSDK/lib/$(ARCH)/libMWCapture.a

all: $(TARGET_CAPTUREBYINPUT)

CAPTUREBYINPUT_OBJS = ./src/CaptureByInput.o
	
$(TARGET_CAPTUREBYINPUT) : $(CAPTUREBYINPUT_OBJS) $(MWCAPTURE_LIBA)
	$(CXX) -o $(TARGET_CAPTUREBYINPUT) $(CAPTUREBYINPUT_OBJS) $(MWCAPTURE_LIBA) $(CLIB)

%.o : %.cpp
	$(CXX) -c $(CFLAGS) $(LOCAL_CFLAGS) $< -o $*.o

clean:	
	rm -rf  $(TARGET_CAPTUREBYINPUT) $(CAPTUREBYINPUT_OBJS)

