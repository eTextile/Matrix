# E256 - eTextile matrix sensor
### Transforming textiles into an intuitive way to interact with computers
## eTextile hardware HOWTO

The eTextile hardware have been designed to be easy to make and customize with accesibles and cheap tools.
There is some tricks to make conductive patterns on regular cotton fabric.
First of all we will need some experience with cutting tip plotters to cutt a conductive fabric with hot melt adhesive.
Plotters are CNC tools that you can buy for cheap on second hand websites.
We suggest to look for an "obsolete" one that work with parallel communication port (NO USB!) this will be cheapest!
Then you will need to buy a parallel to USB adapter and use the inkscape plugin to plot your design...

- https://inkscape.org/
- https://github.com/codelv/inkcut

Under Debian

	sudo apt-get install python-dev
	sudo apt-get -y install python-qt4
    sudo apt-get install python-qtpy

	curl https://bootstrap.pypa.io/get-pip.py -o get-pip.py
	python get-pip.py
	sudo pip install twisted

	git clone https://github.com/frmdstryr/enamlx.git
	cd enamlx
	sudo python setup.py install



![Alt text](./E256_path.svg)

![alt tag](https://farm1.staticflickr.com/789/40837526952_12d6bf42cf_z_d.jpg)

http://wiki.datapaulette.org/doku.php/atelier/documentation/outillage/plotter

### TODO
- Adding 3D web simulator/generator
  - https://github.com/jscad/OpenJSCAD.org
