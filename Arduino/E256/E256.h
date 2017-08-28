// #ifndef __E256_H__
// #define __E256_H__

#include <arm_math.h>
#include <SPI.h>
#include <PacketSerial.h>
#include "blob.h" // Part of the OpenMV project : https://github.com/openmv/openmv

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

// Control pins to send values to the 8-BITs shift registers used on the E-256 PCB
// shiftOut using SPI library : https://forum.arduino.cc/index.php?topic=52383.0
// Arduino UNO - SPI PINS
// DATA_PIN -> SPI:MOSI -> D11 // Pin connected to Data in (DS) of the firdt 74HC595 8-BIT shift register
// CLOCK_PIN -> SPI:SCK -> D13 // Pin connected to clock pin (SH_CP) of the first 74HC595 8-BIT shift register
// LATCH_PIN -> SPI:SS -> D10  // Pin connected to latch pin (ST_CP) of the first 74HC595 8-BIT shift register


#define  BAUD_RATE            230400
#define  COLS                 16
#define  ROWS                 16
#define  SCALE                4
#define  ROW_FRAME            COLS*ROWS
#define  NEW_FRAME            COLS*ROWS*SCALE
#define  CALIBRATION_CYCLES   4

#define MIN_BLOB_PIX          4 // Only blobs that with more pixels than 4

#define A0_PIN                A0  // The output of multiplexerA (SIG pin) is connected to Arduino Analog pin 0
#define A1_PIN                A1  // The output of multiplexerB (SIG pin) is connected to Arduino Analog pin 1

float32_t inputVals[ROW_FRAME] = {0};      // Array to store row input values
int minVals[ROW_FRAME] = {0};              // Array to store smallest values
uint8_t myPacket[ROW_FRAME] = {0};         // Array to store values to transmit
float32_t bilinIntOutput[NEW_FRAME] = {0}; // Bilinear interpolation Output buffer

arm_bilinear_interp_instance_f32 S;

void onPacket(const uint8_t* buffer, size_t size);
void calibrate(uint8_t* id, int val, int frame);
void bilinearInterpolation(float inc);

boolean scan = true;
boolean calibration = true;

uint8_t byteC;
uint8_t byteB;
uint8_t byteA;

// Array to store all parameters used to configure the two shift registers
const byte setCols[COLS] = {
  0x80, 0x40, 0x20, 0x10, 0x8, 0x4, 0x2, 0x1,
  0x80, 0x40, 0x20, 0x10, 0x8, 0x4, 0x2, 0x1
};

// Array to store all parameters used to configure the two analog multiplexeurs
const byte setRows[ROWS] = {
  0x85, 0x87, 0x83, 0x81, 0x82, 0x84, 0x80, 0x86,
  0x58, 0x78, 0x38, 0x18, 0x28, 0x48, 0x8, 0x68
};

