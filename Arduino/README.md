#E-TEXTILE MATRIX SENSOR

- Purpose: 16x16 e-textile matrix sensors
- Copyright (c) 2014 Maurin Donneaud
- Licence : GNU GENERAL PUBLIC LICENSE
- Site web : : http://etextile.org

## Requirements
Programed with Arduino IDE ans Teensy 3.1 support
This Arduino Sketch requires :
 - Teensyduino : http://pjrc.com/teensy/teensyduino.html
 - PacketSerial : https://github.com/bakercp/PacketSerial

### Settings for Arduino IDE
 - Board:           Teensy 3.2 / 3.1
 - USB Type:        Serial
 - CPU Speed:       72 MHz
 - Keyboard Layout: US English
 
## Program Synopsis
To get each sensor value, the microcontrôleur performs lines and columns sweeping. To do these measurements, the lines of the matrix are connected to digital IO pins and the columns are connected to analog input pins. The lines and columns sweeping activate a digital pin and measure each of the analog inputs pins. The digital pins that are not in use are set in the high impedance state.

## TODO
- Add 
