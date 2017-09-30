# eTextile-matrix-sensor / Blob tracking / Arduino

### Transforming textiles into an intuitive way to interact with computers. This project is part of an electronic textiles research on HCI gesture interaction that was started in 2005.

## Requirements
- eTextile-matrix-sensor (Teensy version)
- Arduino IDE
- Arduino library
  - PacketSerial: https://github.com/bakercp/PacketSerial

### Settings for Arduino-mk
- sudo apt-get install arduino-mk
- git clone https://github.com/sudar/Arduino-Makefile.git
- ...

## Program Synopsis
The sketch implemant rows and columns scaning algorithm.
The 16*16 matrix is interpolated to 64*64 with bicubic algorithm.
The blob tracking is apayed on to the interpolated matrix.
The blobs coordinates are transmit via OSC/

## Copyright
Except as otherwise noted, all files in the resistiveMatrix project folder

    Copyright (c) 2005-2017 Maurin Donneaud and others.

For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see LICENSE.txt included in the resistiveMatrix project folder.

## Acknowledgements
Thanks to Vincent Roudaut, Hannah Perner Willson, Cedric Honnet, Antoine Meisso, Paul Strohmeier

## TODO
- Add OSC/TUIO serveur

