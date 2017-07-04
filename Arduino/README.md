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


# E256 PIN MAPPING

COLS = Two 8_Bits shift registers connected directly to the matrix columns
ROWS = One 8_Bits shift register connected to two analog multiplexers that sens the matrix rows

## Shift register_0
- Q0 -> SO_A  // Pin Q0 connected to Analog MUX_A pin S0
- Q1 -> S1_A  // Pin Q1 connected to Analog MUX_A pin S1
- Q2 -> S2_A  // Pin Q2 connected to Analog MUX_A pin S2
- Q3 -> EN_A  // Pin Q3 connected to Analog MUX_A pin Enable (active LOW)
- Q4 -> SO_B  // Pin Q4 connected to Analog MUX_B pin S0
- Q5 -> S1_B  // Pin Q5 connected to Analog MUX_B pin S1
- Q6 -> S2_B  // Pin Q6 connected to Analog MUX_B pin S2
- Q7 -> EN_B  // Pin Q7 connected to Analog MUX_A pin Enable (active LOW)

### Analog MUX_A
- Y0 -> ROW_6 // Pin Y0 connected to ROW 6
- Y1 -> ROW_5 // Pin Y1 connected to ROW 5
- Y2 -> ROW_4 // Pin Y2 connected to ROW 4
- Y3 -> ROW_7 // Pin Y3 connected to ROW 7
- Y4 -> ROW_3 // Pin Y4 connected to ROW 3
- Y5 -> ROW_0 // Pin Y5 connected to ROW 0
- Y6 -> ROW_2 // Pin Y6 connected to ROW 2
- Y7 -> ROW_1 // Pin Y7 connected to ROW 1

### Analog MUX_B
- Y0 -> ROW_14 // Pin Y0 connected to ROW 14
- Y1 -> ROW_13 // Pin Y1 connected to ROW 13
- Y2 -> ROW_12 // Pin Y2 connected to ROW 12
- Y3 -> ROW_15 // Pin Y3 connected to ROW 15
- Y4 -> ROW_11 // Pin Y4 connected to ROW 11
- Y5 -> ROW_8  // Pin Y5 connected to ROW 8
- Y6 -> ROW_10 // Pin Y6 connected to ROW 10
- Y7 -> ROW_9  // Pin Y7 connected to ROW 9

## Shift register_1
- Q0 -> COL_7  // Pin Q0 connected to column 7
- Q1 -> COL_6  // Pin Q1 connected to column 6
- Q2 -> COL_5  // Pin Q2 connected to column 5
- Q3 -> COL_4  // Pin Q3 connected to column 4
- Q4 -> COL_3  // Pin Q4 connected to column 3
- Q5 -> COL_2  // Pin Q5 connected to column 2
- Q6 -> COL_1  // Pin Q6 connected to column 1
- Q7 -> COL_0  // Pin Q7 connected to column 0

## Shift register_2
- Q0 -> COL_15  // Pin Q0 connected to column 15
- Q1 -> COL_14  // Pin Q1 connected to column 14
- Q2 -> COL_13  // Pin Q2 connected to column 13
- Q3 -> COL_12  // Pin Q3 connected to column 12
- Q4 -> COL_11  // Pin Q4 connected to column 11
- Q5 -> COL_10  // Pin Q5 connected to column 10
- Q6 -> COL_9   // Pin Q6 connected to column 9
- Q7 -> COL_8   // Pin Q7 connected to column 8

## Bytes to achieve the matrix scanning

### Byte_C
- COL_8 ->  Q7 : 10000000 -> HEX 0x80
- COL_9 ->  Q6 : 01000000 -> HEX 0x40
- COL_10 -> Q5 : 00100000 -> HEX 0x20
- COL_11 -> Q4 : 00010000 -> HEX 0x10
- COL_12 -> Q3 : 00001000 -> HEX 0x8
- COL_13 -> Q2 : 00000100 -> HEX 0x4
- COL_14 -> Q1 : 00000010 -> HEX 0x2
- COL_15 -> Q0 : 00000001 -> HEX 0x1

### Byte_B
- COL_0 -> Q7 : 10000000 -> HEX 0x80
- COL_1 -> Q6 : 01000000 -> HEX 0x40
- COL_2 -> Q5 : 00100000 -> HEX 0x20
- COL_3 -> Q4 : 00010000 -> HEX 0x10
- COL_4 -> Q3 : 00001000 -> HEX 0x8
- COL_5 -> Q2 : 00000100 -> HEX 0x4
- COL_6 -> Q1 : 00000010 -> HEX 0x2
- COL_7 -> Q0 : 00000001 -> HEX 0x1

### Byte_A
- ROW_0 -> Y5 : 10000101 -> HEX 0x85
- ROW_1 -> Y7 : 10000111 -> HEX 0x87
- ROW_2 -> Y6 : 10000011 -> HEX 0x83
- ROW_3 -> Y4 : 10000001 -> HEX 0x81
- ROW_4 -> Y2 : 10000010 -> HEX 0x82
- ROW_5 -> Y1 : 10000100 -> HEX 0x84
- ROW_6 -> Y0 : 10000000 -> HEX 0x80
- ROW_7 -> Y3 : 10000110 -> HEX 0x86

- ROW_8  -> Y5 : 01011000 -> HEX 0x58
- ROW_9  -> Y7 : 01111000 -> HEX 0x78
- ROW_10 -> Y6 : 00111000 -> HEX 0x38
- ROW_11 -> Y4 : 00011000 -> HEX 0x18
- ROW_12 -> Y2 : 00101000 -> HEX 0x28
- ROW_13 -> Y1 : 01001000 -> HEX 0x48
- ROW_14 -> Y0 : 00001000 -> HEX 0x8
- ROW_15 -> Y3 : 01101000 -> HEX 0x68


## TODO
- Add blob detection
 - Fork OpenMV
 - ...
- Add TUIO serveur
