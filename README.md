# eTextile matrix sensor

### Transforming textiles into an intuitive way to interact with computers. This project is part of an electronic textiles research on HCI gesture interaction that was started in 2005.

- Author: Maurin Donneaud <maurin@datapaulette.org> - Industrial designer
- Contributors:
    - Cedric Honnet <cedric@datapaulette.org> - Electronic engineering 
    - Antoine Messonier <ameisso@gmail.com> - Software developer
- Repository: https://github.com/eTextile/resistiveMatrix
- Project web page: http://eTextile.org
- License : CC-BY-SA (see the License file)

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

##TODO
- See the README inside the project content files
- https://github.com/openframeworks/openFrameworks/issues/5607 FIXME (add ofJson.h to OF_ROOT/addons/ofxIO/src/ofxIO.h ?)
