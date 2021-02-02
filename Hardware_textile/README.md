# E256 - eTextile matrix sensor
## HARDWARE_TEXTILE / HOWTO

![Textile_matrix_sensor](https://farm1.staticflickr.com/798/40904680852_d5c9b1b35d_z_d.jpg)

The eTextile hardware have been designed to be easy to make with accessible and cheap tools.
Here are the tricks to make the conductive textile pattern on a woven cotton fabric.
First of all we will need to add the fusing material (hot melt adhesive) to the conductive fabric.
Then we can use the CNC-Cutter to cut the two identicals (X and Y) matrix patterns.
CNC-Cutters or Plotters are tools that you can buy for cheap on second hand websites.
We suggest to look for an "obsolete" one that work with **parallel port** (NO USB!) This will be cheapest!
Then you can buy a **parallel to USB adapter** and use the inkscape plugin to cut the conductive textile matrix pattern.
After having cut the conductive fabric doubled with the bounding material you will need to fuse it to your woven fabric using a 
heat press.

### BOM
- **Conductive fabric** : silver-coated Polyester or nickel-coated Polyester
- **CNC cutter** : 
- **heat press** :
- **Fusing material** :
- **E256 path** : interdigitation zig-zag pattern with built-in eTextile-bus (pitch 4.2mm)

### SOFTWARE REQUIREMENTS
- https://inkscape.org/
- https://github.com/codelv/inkcut

#### Linux install
```
sudo apt-get install python-dev
sudo apt-get -y install python-qt4
sudo apt-get install python-qtpy

curl https://bootstrap.pypa.io/get-pip.py -o get-pip.py
python get-pip.py
sudo pip install twisted

git clone https://github.com/frmdstryr/enamlx.git
cd enamlx
sudo python setup.py install
```

## E256 cutting the conductive textile
Here is the unique file to cut both TOP and BOTTOM antena layers of the E256 matrix : [E256_path](./E256_path.svg)

## E256 fusing the conductive textile 
![Lost_parts](https://live.staticflickr.com/65535/48791432428_cccc5ecaf6_c_d.jpg)
![Heat_press](https://live.staticflickr.com/65535/48791938502_d166d7a995_c_d.jpg)

### TODO
- Add non-slip Silicon
