// E-256 eTextile matrix sensor shield V1.2

#include <SPI.h>
#include <PacketSerial.h>

PacketSerial serial;

/*
  PACKET SERIAL : Copyright (c) 2012-2014 Christopher Baker <http://christopherbaker.net>
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/

// The PacketSerial object.
// It cleverly wraps one of the Serial objects.
// While it is still possible to use the Serial object
// directly, it is recommended that the user let the
// PacketSerial object manage all serial communication.
// Thus the user should not call Serial.write(), etc.
// Additionally the user should not use the serialEvent()
// callbacks.

#define  BAUD_RATE            230400
#define  ROWS                 16
#define  COLS                 16
#define  DATAS                256
#define  CALIBRATION_CYCLES   4

// Control pins to send values to the 8-BITs shift registers used on the E-256 PCB
// shiftOut using SPI library : https://forum.arduino.cc/index.php?topic=52383.0
// Arduino UNO - SPI PINS
// DATA_PIN -> SPI:MOSI -> D11 // Pin connected to Data in (DS) of the firdt 74HC595 8-BIT shift register
// CLOCK_PIN -> SPI:SCK -> D13 // Pin connected to clock pin (SH_CP) of the first 74HC595 8-BIT shift register
// LATCH_PIN -> SPI:SS -> D10  // Pin connected to latch pin (ST_CP) of the first 74HC595 8-BIT shift register

#define A0_PIN        A0  // The output of MultiplexerA (SIG pin) is connected to Arduino Analog pin 0
#define A1_PIN        A1  // The output of MultiplexerB (SIG pin) is connected to Arduino Analog pin 1

byte value[ROWS][COLS];        // Array to store 256 analog valeus
unsigned int columnBitPos = 0; // variable to store column bit positionoutput

int minVal[DATAS] = {0};
uint8_t myPacket[DATAS] = {0};

boolean scan = true;
boolean calibration = true;
int calibrationCounter = 0;

boolean DEBUG = false;          // true for arduino debuging, false for processing.

/*
  E256 PIN MAPPING
  COLS = Two 8_Bits shift registers connected directly to the matrix columns
  ROWS = One 8_Bits shift register connected to two analog multiplexers that sens the matrix rows

  Shift register_0
  Q0 -> SO_A  // Pin Q0 connected to Analog MUX_A pin S0
  Q1 -> S1_A  // Pin Q1 connected to Analog MUX_A pin S1
  Q2 -> S2_A  // Pin Q2 connected to Analog MUX_A pin S2
  Q3 -> EN_A  // Pin Q3 connected to Analog MUX_A pin Enable (active LOW)
  Q4 -> SO_B  // Pin Q4 connected to Analog MUX_B pin S0
  Q5 -> S1_B  // Pin Q5 connected to Analog MUX_B pin S1
  Q6 -> S2_B  // Pin Q6 connected to Analog MUX_B pin S2
  Q7 -> EN_B  // Pin Q7 connected to Analog MUX_A pin Enable (active LOW)

  Analog MUX_A
  Y0 -> ROW_6 // Pin Y0 connected to ROW 6
  Y1 -> ROW_5 // Pin Y1 connected to ROW 5
  Y2 -> ROW_4 // Pin Y2 connected to ROW 4
  Y3 -> ROW_7 // Pin Y3 connected to ROW 7
  Y4 -> ROW_3 // Pin Y4 connected to ROW 3
  Y5 -> ROW_0 // Pin Y5 connected to ROW 0
  Y6 -> ROW_2 // Pin Y6 connected to ROW 2
  Y7 -> ROW_1 // Pin Y7 connected to ROW 1

  Analog MUX_B
  Y0 -> ROW_14 // Pin Y0 connected to ROW 14
  Y1 -> ROW_13 // Pin Y1 connected to ROW 13
  Y2 -> ROW_12 // Pin Y2 connected to ROW 12
  Y3 -> ROW_15 // Pin Y3 connected to ROW 15
  Y4 -> ROW_11 // Pin Y4 connected to ROW 11
  Y5 -> ROW_8  // Pin Y5 connected to ROW 8
  Y6 -> ROW_10 // Pin Y6 connected to ROW 10
  Y7 -> ROW_9  // Pin Y7 connected to ROW 9

  Shift register_1
  Q0 -> COL_7  // Pin Q0 connected to column 7
  Q1 -> COL_6  // Pin Q1 connected to column 6
  Q2 -> COL_5  // Pin Q2 connected to column 5
  Q3 -> COL_4  // Pin Q3 connected to column 4
  Q4 -> COL_3  // Pin Q4 connected to column 3
  Q5 -> COL_2  // Pin Q5 connected to column 2
  Q6 -> COL_1  // Pin Q6 connected to column 1
  Q7 -> COL_0  // Pin Q7 connected to column 0

  Shift register_2
  Q0 -> COL_15  // Pin Q0 connected to column 15
  Q1 -> COL_14  // Pin Q1 connected to column 14
  Q2 -> COL_13  // Pin Q2 connected to column 13
  Q3 -> COL_12  // Pin Q3 connected to column 12
  Q4 -> COL_11  // Pin Q4 connected to column 11
  Q5 -> COL_10  // Pin Q5 connected to column 10
  Q6 -> COL_9   // Pin Q6 connected to column 9
  Q7 -> COL_8   // Pin Q7 connected to column 8

  // Bytes to achieve the matrix scanning

   byteC
    COL_8 ->  Q7 : 10000000 -> HEX 0x80
    COL_9 ->  Q6 : 01000000 -> HEX 0x40
    COL_10 -> Q5 : 00100000 -> HEX 0x20
    COL_11 -> Q4 : 00010000 -> HEX 0x10
    COL_12 -> Q3 : 00001000 -> HEX 0x8
    COL_13 -> Q2 : 00000100 -> HEX 0x4
    COL_14 -> Q1 : 00000010 -> HEX 0x2
    COL_15 -> Q0 : 00000001 -> HEX 0x1

   byteB
    COL_0 -> Q7 : 10000000 -> HEX 0x80
    COL_1 -> Q6 : 01000000 -> HEX 0x40
    COL_2 -> Q5 : 00100000 -> HEX 0x20
    COL_3 -> Q4 : 00010000 -> HEX 0x10
    COL_4 -> Q3 : 00001000 -> HEX 0x8
    COL_5 -> Q2 : 00000100 -> HEX 0x4
    COL_6 -> Q1 : 00000010 -> HEX 0x2
    COL_7 -> Q0 : 00000001 -> HEX 0x1

   byteA
    ROW_0 -> Y5 : 10000101 -> HEX 0x85
    ROW_1 -> Y7 : 10000111 -> HEX 0x87
    ROW_2 -> Y6 : 10000011 -> HEX 0x83
    ROW_3 -> Y4 : 10000001 -> HEX 0x81
    ROW_4 -> Y2 : 10000010 -> HEX 0x82
    ROW_5 -> Y1 : 10000100 -> HEX 0x84
    ROW_6 -> Y0 : 10000000 -> HEX 0x80
    ROW_7 -> Y3 : 10000110 -> HEX 0x86

    ROW_8  -> Y5 : 01011000 -> HEX 0x58
    ROW_9  -> Y7 : 01111000 -> HEX 0x78
    ROW_10 -> Y6 : 00111000 -> HEX 0x38
    ROW_11 -> Y4 : 00011000 -> HEX 0x18
    ROW_12 -> Y2 : 00101000 -> HEX 0x28
    ROW_13 -> Y1 : 01001000 -> HEX 0x48
    ROW_14 -> Y0 : 00001000 -> HEX 0x8
    ROW_15 -> Y3 : 01101000 -> HEX 0x68
*/

byte byteC;
byte byteB;
byte byteA;

const byte setCols[COLS / 2] = {
  0x80, 0x40, 0x20, 0x10, 0x8, 0x4, 0x2, 0x1
};
const byte setRows[COLS] = {
  0x85, 0x87, 0x83, 0x81, 0x82, 0x84, 0x80, 0x86,
  0x58, 0x78, 0x38, 0x18, 0x28, 0x48, 0x8, 0x68
};

////////////////////////////////////// SETUP
void setup() {

  serial.setPacketHandler(&onPacket); // We must specify a packet handler method so that
  serial.begin(BAUD_RATE);  // Start the serial module

  SPI.begin();              // Start the SPI module
  pinMode(SS, OUTPUT);      // Set up slave mode
  SPI.transfer(0xFFFFFF);   // All OFF
  digitalWrite(SS, LOW);    // set latchPin LOW
  digitalWrite(SS, HIGH);   // set latchPin HIGH
}

////////////////////////////////////// LOOP
void loop() {

  if (scan) {

    for (byte col = 0; col < COLS; col++) {

      for (byte row = 0; row < ROWS / 2; row++) {
        byteB = setCols[col % (COLS / 2) ];
        byteA = setRows[row];
        SPI.transfer(0x0);      // shift out the first byte
        SPI.transfer(byteB);    // shift out the secound byte
        SPI.transfer(byteA);    // shift out the terird byte
        digitalWrite(SS, LOW);  // set latchPin LOW
        digitalWrite(SS, HIGH); // set latchPin HIGH
        int value = analogRead(A0_PIN); // Reding use to store analog inputs values
        int sensorID = col * COLS + row; // Calculate the index of the unidimensional array

        if (calibration) {
          minVal[sensorID] += value;
          calibrationCounter++;
          if (calibrationCounter >= CALIBRATION_CYCLES * DATAS) {
            for (int i = 0; i < DATAS; i++) {
              minVal[i] = minVal[i] / CALIBRATION_CYCLES;
            }
            calibration = false;
          }
        } else {
          value = map(value, minVal[sensorID], 1024, 0, 255);
          myPacket[sensorID] = (byte)value;
        }
      }
      for (byte row = 8; row < ROWS; row++) {
        byteC = setCols[col % (COLS / 2) ];
        byteA = setRows[row];
        SPI.transfer(byteC);    // shift out the first byte
        SPI.transfer(0x0);      // shift out the secound byte
        SPI.transfer(byteA);    // shift out the terird byte
        digitalWrite(SS, LOW);  // set latchPin LOW
        digitalWrite(SS, HIGH); // set latchPin HIGH
        int value = analogRead(A1_PIN);
        int sensorID = col * COLS + row; // Calculate the index of the unidimensional array

        if (calibration) {
          minVal[sensorID] += value;
          calibrationCounter++;
          if (calibrationCounter >= CALIBRATION_CYCLES * DATAS) {
            for (int i = 0; i < DATAS; i++) {
              minVal[i] = minVal[i] / CALIBRATION_CYCLES;
            }
            calibration = false;
          }
        } else {
          value = map(value, minVal[sensorID], 1024, 0, 255);
          myPacket[sensorID] = (byte)value;
        }
      }
    }
    scan = false;
  }
  // The update() method attempts to read in
  // any incoming serial data and emits packets via
  // the user's onPacket(const uint8_t* buffer, size_t size)
  // method registered with the setPacketHandler() method.
  // The update() method should be called at the end of the loop().
  serial.update();
}

// This is our packet callback.
// The buffer is delivered already decoded.
void onPacket(const uint8_t* buffer, size_t size) {
  // The send() method will encode the buffer
  // as a packet, set packet markers, etc.
  serial.send(myPacket, DATAS);
  scan = true;
}

