# eTextile matrix sensor
- Purpose: 16x16 e-textile matrix sensors
- Copyright (c) 2014 Maurin Donneaud
- Licence : GNU GENERAL PUBLIC LICENSE
- Site web : : http://etextile.org

## Requirements
- Programed with Arduino IDE
- This Arduino Sketch require:
 - PacketSerial : https://github.com/bakercp/PacketSerial

### Settings for Arduino IDE
- Board:           Arduino UNO
- USB Type:        Serial
 
## Program Synopsis
Yhe eTextile matrix rows and columns are connected to the E256 PCB.
To get each sensor value, the Arduino communicate with the E256 PCB via a fast  parallelised data transfer.


## Data Transmission
The datas are then sent through frames.
Each frame is an array of ROW*COL = 256 values. 
The values are ordered by rows (the first values corrsponds to the first collumn then all the collumns of the second row ...);
For now, the values are encoded on two bytes.
As an example here is how a 4*4 frame would be encoded : 

(R0 C0 LSB) (R0 C0 MSB) (R0 C1 LSB) (R0 C1 MSB) (R0 C2 LSB) (R0 C2 MSB) (R0 C3 LSB) (R0 C3 MSB)
(R1 C0 LSB) (R1 C0 MSB) (R1 C1 LSB) (R1 C1 MSB) (R1 C2 LSB) (R1 C2 MSB) (R1 C3 LSB) (R1 C3 MSB)
(R2 C0 LSB) (R2 C0 MSB) (R2 C1 LSB) (R2 C1 MSB) (R2 C2 LSB) (R2 C2 MSB) (R2 C3 LSB) (R2 C3 MSB)
(R3 C0 LSB) (R3 C0 MSB) (R3 C1 LSB) (R3 C1 MSB) (R3 C2 LSB) (R3 C2 MSB) (R3 C3 LSB) (R3 C3 MSB)

![alt tag](http://etextile-summercamp.org/swatch-exchange/wp-content/uploads/2015/05/Matrix_011.png)

## TODO
- Add 
