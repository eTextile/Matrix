Textile matrix 1.0
=========
 - Date : 21/09/2014
 - Credit : Maurin Donneaud http://maurin.donneaud.free.fr/
 - Project : http://maurin.donneaud.free.fr/?-Keyboard-/
 - Repository : https://github.com/MaurinElectroTextile/

This project is part of my electronic textiles research for gesture interaction.

E-textile materials and assembling techniques ave ben experimented in colaboration with Hannah and Mika from kobakant : http://www.kobakant.at
during them Electronic Textiles Live Workshops 11-21 September 2014, at the Schmiede festival in Hallein near Salzburg, Austria. 

This matrix is made from conductive fabric rows and columns fused to a base fabric with Eeonyx piezoresistive fabric inbetween.
The rows and columns are connected to a Teensy microcontroller which parses the rows and columns to measure pressure and location on the surface of the fabric matrix.
Sensor data is read into a PD patch that synthesizes sound. All the parts are products that you can buy from company or webshops.

The Teensy firmware is arduino IDE compatible
see : http://www.pjrc.com/teensy/teensyduino.html

The only components used with the Teesy board are resistors.
One resistor is connected between each analog input column pins and VCC to make a voltage divider.
In order to get the best pressure sensitive resolution, the resistors values are calculated in relation with the maximum and minimum Eeonyx piezoresistive fabric values.

To edit the programme : cd /Teensy3.1_matrix/VoltageDividerResistorCalculator.py

 - Rc_min = the minimum resistance of one matrix intersection
 - Rc Max = the maximum resistance of one matrix intersection
 - Vcc = the electronic reference voltage

You will also need to adjust the values of the range function to focus on the more precise resistor value
 - range(4000, 10000, 100):

Then you can start the programme by typing in a terminal : python VoltageDividerResistorCalculator.py

TODO
=========
 - add SupperColider driver ;-)
 - use the new Teensy (3.1) to make 16 by 16 matrix
 - make a PCB to connect the Teensy board to the e-Textile bus
 - ...
 
 Docs
 =========
 - http://www.kobakant.at/DIY/?p=4305/
 - https://www.flickr.com/photos/maurin/15378383106/
