# eTextile matrix sensor

### Transforming textiles into an intuitive way to interact with computers. This project is part of an electronic textiles research on HCI gesture interaction that was started in 2005.

 - Credit : Maurin Donneaud <maurin@datapaulette.org>
 - Repository : https://github.com/MaurinElectroTextile/eTextile_matrix_sensor
 - Project web page : http://eTextile.org

![alt tag](https://farm6.staticflickr.com/5572/30306414062_22bba76566_z_d.jpg)

## About The project

From the beginning this project stimulates the development of technologies in artistic use cases : playing music, combine graphics and sound, imagine objects that can have tactile properties, etc.
Using a fabric to sense the touch is now possible with this eTextile matrix sensor that includes conductive fibers and a layer of piezoresistive textile.
Some new E-textile materials and assembling techniques were experimented during the 2014 batch of the Schmiede residency in Hallein (Austria), with Hannah Perner Wilson and Mika Satomi from Kobakant who organised the Electronic Textiles Live Workshops.
Made of conductive textile shaped in rows and columns fused onto a basic cotton fabric, this matrix is designed to be easy to make.
To allow presure measurement, a 20Kohm Eeonyx piezoresistive fabric is added in between the e-textile conductive matrix.
For the electronic frontend, we still work on a new version that will integrate a dedicated microcontroller.
On the software side we can visualise a representation of the eTextile matrix sensor that shows the pressure points and their positions on the virtual fabric surface.
This 3D real time model analyses the touch to trigger OSC messages that can be used in audio application such as PureData, SuperCollider, etc.

## Project content
- Arduino firmware
- Eagle PCB
- openframeworks software

## Docs
- [A video made at Schmiede 2014](http://www.kobakant.at/DIY/?p=4305/)
- [Pictures of the project and context](https://www.flickr.com/photos/maurin/albums/72157673740361510)
- [The tutorial on the DataPaulette eTextile hakerspace wiki](http://wiki.datapaulette.org/doku.php/atelier/projets/matrice_textile)

## USING OSC FOR DEVELOPPMENT
You can test the openframework program without the textile device nor arduino.
To do so, simply send an OSC message on port 1234 (defined as UDP_INPUT_PORT in ofApp.h).
The message should be something like (/point (x,y,z) ) where x,y and z are floats between 0 and 1.
you can send a /reset message to clear the map (remove all points).

##TODO
 - See the README inside the project content files
