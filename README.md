Textile matrix 1.0
=========
This project is part of electronic textiles research on IHM gesture interaction.
 - Date : 21/09/2014
 - Credit : Maurin Donneaud
 - Project : http://maurin.donneaud.free.fr/?-Keyboard-/
 - Repository : https://github.com/MaurinElectroTextile/Keyboard
 
![alt tag](https://c4.staticflickr.com/8/7637/16464397214_b98a5b9cde.jpg)

Some E-textile materials and assembling technics have been experimented during the Schmiede festival 2014 in Hallein (Austria), with Hannah and Mika from kobakant: http://www.kobakant.at who organise the Electronic Textiles Live Workshops.

This matrix is made out of rows and columns conductive fabric fused onto a basic cotton fabric. In between the e-textile matrix a 20K Eeonyx piezoresistive fabric allow pressure detection. The rows and columns are connected to a Teensy microcontroller to scan and measure all pressures positions on the fabric surface. All values are send to an application that show a real time 3D fabric model who produce OSC messages that can be used in audio application such PureData, etc.

The Teensy firmware is arduino IDE compatible
see : http://www.pjrc.com/teensy/teensyduino.html
The only components used with the Teesy board are 10K resistor ladder.
To make a voltage divider for each column, every resistor are connected between an analog input pins and VCC (3.3V).

TODO
=========
 - add SupperColider driver ;-)
 - ...
 
 Docs
 =========
 - http://www.kobakant.at/DIY/?p=4305/
 - https://www.flickr.com/photos/maurin/15378383106/
