// E-256 eTextile matrix sensor shield V2.0

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
#define  COLS                 16
#define  ROWS                 16
#define  DATAS                256
#define  CALIBRATION_CYCLES   4

// Control pins to send values to the 8-BITs shift registers used on the E-256 PCB
// shiftOut using SPI library : https://forum.arduino.cc/index.php?topic=52383.0
// Arduino UNO - SPI PINS
// DATA_PIN -> SPI:MOSI -> D11 // Pin connected to Data in (DS) of the firdt 74HC595 8-BIT shift register
// CLOCK_PIN -> SPI:SCK -> D13 // Pin connected to clock pin (SH_CP) of the first 74HC595 8-BIT shift register
// LATCH_PIN -> SPI:SS -> D10  // Pin connected to latch pin (ST_CP) of the first 74HC595 8-BIT shift register

#define A0_PIN        A0  // The output of multiplexerA (SIG pin) is connected to Arduino Analog pin 0
#define A1_PIN        A1  // The output of multiplexerB (SIG pin) is connected to Arduino Analog pin 1

byte value[ROWS][COLS];        // Array to store 256 analog valeus
unsigned int columnBitPos = 0; // variable to store column bit positionoutput

int minVal[DATAS] = {0};
uint8_t myPacket[DATAS] = {0};

boolean scan = true;
boolean calibration = true;

byte byteC;
byte byteB;
byte byteA;

const byte setCols[COLS] = {
  0x80, 0x40, 0x20, 0x10, 0x8, 0x4, 0x2, 0x1,
  0x80, 0x40, 0x20, 0x10, 0x8, 0x4, 0x2, 0x1
};

const byte setRows[COLS] = {
  0x85, 0x87, 0x83, 0x81, 0x82, 0x84, 0x80, 0x86,
  0x58, 0x78, 0x38, 0x18, 0x28, 0x48, 0x8, 0x68
};

boolean DEBUG = false;          // true for arduino debuging, false for processing.
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
      for (byte row = 0; row < ROWS; row++) {

        if (row < 8) {
          byteA = setRows[row];
          byteB = setCols[col];
          byteC = 0x0;
          SPI.transfer(byteA);    // shift out the first byte
          SPI.transfer(byteB);    // shift out the secound byte
          SPI.transfer(byteC);    // shift out the terird byte
          digitalWrite(SS, LOW);  // set latchPin LOW
          digitalWrite(SS, HIGH); // set latchPin HIGH
        } else {
          byteA = setRows[row];
          byteB = 0x0;
          byteC = setCols[col];
          SPI.transfer(byteA);    // shift out the first byte
          SPI.transfer(byteB);    // shift out the secound byte
          SPI.transfer(byteC);    // shift out the terird byte
          digitalWrite(SS, LOW);  // set latchPin LOW
          digitalWrite(SS, HIGH); // set latchPin HIGH
        }

        int value = analogRead(A0_PIN);  // Reding use to store analog inputs values
        int sensorID = col * COLS + row; // Calculate the index of the unidimensional array

        if (calibration) {
          calibrate(sensorID, value);
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


void calibrate( byte id, int val ) {

  static int calibrationCounter = 0;

  minVal[id] += val;
  calibrationCounter++;
  if (calibrationCounter >= CALIBRATION_CYCLES * DATAS) {
    for (int i = 0; i < DATAS; i++) {
      minVal[i] = minVal[i] / CALIBRATION_CYCLES;
    }
    calibrationCounter = 0;
  }
  calibration = false;

}

// This is our packet callback.
// The buffer is delivered already decoded.
void onPacket(const uint8_t* buffer, size_t size) {
  // The send() method will encode the buffer
  // as a packet, set packet markers, etc.
  serial.send(myPacket, DATAS);
  scan = true;
}

