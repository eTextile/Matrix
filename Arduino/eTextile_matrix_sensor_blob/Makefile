AVR_TOOLS_DIR = /usr/
ARDUINO_DIR  = $(shell dirname $(shell readlink -f `which arduino`))

BOARD_TAG    = teensy31
ARDUINO_LIBS = PacketSerial

include makefiles/Teensy.mk

MONITOR_PORT = /dev/ttyACM0
F_CPU        = 72000000L
USB_TYPE     = USB_SERIAL

#lazy "make clean" shortcut
c: clean

