#
# Copyright (c) 2014 Rafael Vega <rvega@elsoftwarehamuerto.org>
# 
# BSD Simplified License.
# For information on usage and redistribution, and for a DISCLAIMER OF ALL
# WARRANTIES, see the file, "LICENSE.txt," in this distribution.
# 
# See https://github.com/libpd/libpd for documentation
#

# detect platform, move libpd dylib to local folder on mac
UNAME = $(shell uname)
SOLIB_PREFIX = lib

ifeq ($(UNAME), Darwin)  # Mac
  SOLIB_EXT = dylib
  PLATFORM = mac
  CXXFLAGS = -D__MACOSX_CORE__
  AUDIO_API = -framework Foundation -framework CoreAudio
else
  ifeq ($(OS), Windows_NT)  # Windows, use Mingw
    SOLIB_EXT = dll
    SOLIB_PREFIX = 
    PLATFORM = windows
    CXXFLAGS = -D__WINDOWS_DS__
    AUDIO_API = -lole32 -loleaut32 -ldsound -lwinmm
  else  # assume Linux
    SOLIB_EXT = so
    PLATFORM = linux
    CXXFLAGS = -D__UNIX_JACK__ -D__LINUX_ALSA__
    AUDIO_API = -ljack -lasound -pthread
  endif
endif

LIBPD_DIR = ../../..
LIBPD = $(LIBPD_DIR)/libs/libpd.$(SOLIB_EXT)

SRC_FILES = src/PdObject.cpp src/main.cpp src/RtAudio.cpp
TARGET = pdtest_rtaudio

CXXFLAGS += -I$(LIBPD_DIR)/pure-data/src -I$(LIBPD_DIR)/libpd_wrapper \
            -I$(LIBPD_DIR)/libpd_wrapper/util -I$(LIBPD_DIR)/cpp \
            -I./src -std=c++11 -DLIBPD_USE_STD_MUTEX -O3

.PHONY: clean clobber

$(TARGET): ${SRC_FILES:.cpp=.o} $(LIBPD) src/comport.o src/slipenc.o src/slipdec.o src/packOSC.o src/unpackOSC.o src/routeOSC.o
	g++ -o $(TARGET) $^ $(LIBPD) $(AUDIO_API)

src/comport.o: src/comport.c
	gcc -o src/comport.o -c src/comport.c

src/slipenc.o: src/slipenc.c
	gcc -o src/slipenc.o -c src/slipenc.c

src/slipdec.o: src/slipdec.c
	gcc -o src/slipdec.o -c src/slipdec.c

src/packOSC.o: src/packOSC.c
	gcc -o src/packOSC.o -c src/packOSC.c

src/unpackOSC.o: src/unpackOSC.c
	gcc -o src/unpackOSC.o -c src/unpackOSC.c

src/routeOSC.o: src/routeOSC.c
	gcc -o src/routeOSC.o -c src/routeOSC.c

$(LIBPD):
	cd $(LIBPD_DIR) && make UTIL=true EXTRA=true

clean:
	rm -f src/*.o

clobber: clean
	rm -f $(TARGET)
	if [ $(PLATFORM) == "mac" ]; then rm -rf ./libs; fi
	cd $(LIBPD_DIR) && make clobber
