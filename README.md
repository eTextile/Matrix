Textile matrix 1.0
=========
This project is part of my electronic textiles research for gesture interaction.
 - Date : 21/09/2014
 - Credit : Maurin Donneaud http://maurin.donneaud.free.fr/
 - Project : http://maurin.donneaud.free.fr/?-Keyboard-/
 - Repository : https://github.com/MaurinElectroTextile/Keyboard
 
![alt tag](https://c4.staticflickr.com/8/7637/16464397214_b98a5b9cde.jpg)

E-textile materials and assembling technics have been experimented in colaboration with Hannah and Mika from kobakant: http://www.kobakant.at during them Electronic Textiles Live Workshops 11-21 September 2014, at the Schmiede festival in Hallein near Salzburg, Austria.

This matrix is made out of rows and columns conductive fabric fused to a basic coton fabric. In between the e-textile matrix a 20K Eeonyx piezoresistive fabric allow pressure detection. The rows and columns are connected to a Teensy microcontroller which parses the rows and columns to measure pressure and location on the surface of the fabric matrix. Sensor data is read into a PD patch that synthesizes sound.

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
