# eTextile-matrix-sensor / Blob tracking / Arduino

### Transforming textiles into an intuitive way to interact with computers. This project is part of an electronic textiles research on HCI gesture interaction that was started in 2005.

## Requirements
- eTextile-matrix-sensor (Teensy version)
- Arduino IDE
- Arduino library
  - PacketSerial: https://github.com/bakercp/PacketSerial
  - 

### Settings for Arduino-mk
    sudo apt-get install arduino-mk
    git clone https://github.com/sudar/Arduino-Makefile.git

## Program Synopsis
- The sketch implemant rows and columns scaning algorithm.
- The 16x16 matrix is interpolated to 64x64 with bicubic algorithm.
- The blob tracking is applyed on to the interpolated matrix.
- The blobs coordinates are transmit via OSC

## Teensy 3.1/3.2 SPI Pin mapping
Control pins to send values to the 8-BITs shift registers used on the E-256 PCB

    LATCH_PIN -> SPI:SS -> D10  // Pin connected to latch pin (ST_CP) of the first 74HC595 8-BIT shift register
    CLOCK_PIN -> SPI:SCK -> D13 // Pin connected to clock pin (SH_CP) of the first 74HC595 8-BIT shift register
    DATA_PIN -> SPI:MOSI -> D11 // Pin connected to Data in (DS) of the firdt 74HC595 8-BIT shift register

## Copyright
Except as otherwise noted, all files in the resistiveMatrix project folder

    Copyright (c) 2005-2017 Maurin Donneaud and others.

For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see LICENSE.txt included in the resistiveMatrix project folder.

## Acknowledgements
Thanks to Vincent Roudaut, Hannah Perner Willson, Cedric Honnet, Antoine Meisso, Paul Strohmeier

## TODO
- Add OSC/TUIO communication layer
- Optimise interpolation method
  - Retrieval method from Microchip TB3064 white paper (p12)
  - microchip.com/stellent/groups/techpub_sg/documents/devicedoc/en550192.pdf


