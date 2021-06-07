# E256 - eTextile matrix sensor
## Transforming textiles into an intuitive way to interact with computers

## Arduino Firmware
    /Matrix-master/Firmware/main/main.ino
 
### Requirements
- **Teensy 3.2 or 4.0**
- **Arduino IDE** : Arduino 1.8.14 or higher [DOWNLOAD](https://www.arduino.cc/en/Main/Software)
- **Arduino IDE additional board** : Teensyduino 1.54 or higher [DOWNLOAD](https://www.pjrc.com/teensy/td_download.html)
- **Arduino IDE additional library**
  - **Included in Teensyduino**
    - **SPI**: https://github.com/PaulStoffregen/SPI
    - **ADC**: https://github.com/pedvide/ADC
    - **Encoder** : https://github.com/PaulStoffregen/Encoder 
    - **elapsedMillis** : https://github.com/pfeerick/elapsedMillis
    - **MIDI** : https://github.com/PaulStoffregen/MIDI
  - **To install with Arduino library manager**
    - **Bounce2** : https://github.com/thomasfredericks/Bounce2
  - **To install by hand in /Documents/Arduino/library**
    - **OSC** : https://github.com/CNMAT/OSC

### Arduino IDE Settings for Teensy 4.0
- **Board** :     Teensy 3.2
- **USB Type** :  MIDI or Serial(SLIP-OSC)
- **CPUSpeed** :  600MHz
- **Optimize** :  Faster

### Program Synopsis
- **Bilinear interpolation** The 16x16 Analog pressure sensor values are interpolated with a bilinear algorithm
- **Blob tracking** The interpolated pressure matrix sensor values are analyzed with a Connected Component Labelling algorithm
- **Blob ID management** each blob is tracked in space and time using single chained linked list

### eTextile-Synthesizer / Benchmark
  - ADC_INPUT : 2500 FPS
  - ADC_INPUT / BILINEAR_INTERPOLATION : ... FPS
  - ADC_INPUT / BILINEAR_INTERPOLATION / BLOB_TRACKING : ... FPS
  - ADC_INPUT / BILINEAR_INTERPOLATION / BLOB_TRACKING / AUDIO : ...

## Configuring the system
Using the matrix sensor in combination with Application like Ableton live, Pure Data, MaxMsp...
  - **MIDI_USB** : digitized touch transmitted via MIDI
  - **USB_SLIP_OSC** : digitized touch transmitted via SLIP-OSC

## Copyright
Except as otherwise noted, all files in the eTextile-Synthesizer project folder

    Copyright (c) 20014- Maurin Donneaud

For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see LICENSE.txt included in the eTextile-Synthesizer project folder.

## TODO
- Optimise interpolation method (it have been already updated with windowing method)
- **Gesture Recognizer** could be a nice feature that fitt the eTextile tactile surface.
- **MIDI MPE** : MIDI in general does not have continuous control of note parameters (pitch and velocity) after the note has been played. Some companies like Roli with MPE have tried to address these limitations, but support is not as wide as standard control changes.
