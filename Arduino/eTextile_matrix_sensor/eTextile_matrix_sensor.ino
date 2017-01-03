/*
  E-TEXTILE MATRIX SENSOR : Copyright (c) 2014 Maurin Donneaud http://etextile.org
  Purpose: 16x16 e-textile sensors matrix
  Licence : GNU GENERAL PUBLIC LICENSE

  Programed with Arduino IDE ans Teensy 3.1 support
    Install Teensyduino : http://pjrc.com/teensy/teensyduino.html
    Install PacketSerial : https://github.com/bakercp/PacketSerial

  Settings for Arduino IDE
    Board:           Teensy 3.2 / 3.1
    USB Type:        Serial
    CPU Speed:       72 MHz
    Keyboard Layout: US English
*/

#include <PacketSerial.h>
// #include "tools.h"

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

// The PacketSerialsqrt object.
// It cleverly wraps one of the Serial objects.
// While it is still possible to use the Serial object
// directly, it is recommended that the user let the
// PacketSerial object manage all serial communication.
// Thus the user should not call Serial.write(), etc.
// Additionally the user should not use the serialEvent()
// callbacks.

#define  BAUD_RATE            230400 // With Teensy, it's always the same native speed. The baud rate setting is ignored.
#define  ROWS                 16
#define  COLS                 16
#define  DATAS                256   // 
#define  LED_PIN              13    // Teensy  built-in LED
#define  BUTTON_PIN           32    // 
#define  CALIBRATION_CYCLES   4     // 
#define  CURVE                5     // Set the sensors logarithmic curve responds 0-10

// Digital pins array
// See the attached home made PCB (Eagle file) to understand the Digital and Analog pin mapping
const int rowPins[ROWS] = {
  27, 26, 25, 24, 12, 11, 10, 9, 8, 7, 6, 5, 33, 2, 1, 0
};

// Analog pins array
const int columnPins[COLS] = {
  A17, A18, A19, A0, A20, A1, A2, A3, A4, A5, A6, A7, A11, A8, A10, A9
};

int minVal[DATAS];
uint8_t myPacket[DATAS];

boolean scan = true;
boolean calibration = true;
int calibrationCounter = 0;

void setup() {
  // We must specify a packet handler method so that
  serial.setPacketHandler(&onPacket);
  serial.begin(BAUD_RATE);

  analogReadRes(10);                     // Set the ADC converteur resolution to 10 bit
  pinMode(LED_PIN, OUTPUT);              // Set rows pins in high-impedance state
  pinMode(BUTTON_PIN, INPUT_PULLUP);     // Set button pins as input and activate the input pullup resistor
  attachInterrupt(BUTTON_PIN, pushButton, RISING); // interrrupt 1 is data ready

  for (int i = 0; i < ROWS; i++) {
    pinMode(rowPins[ROWS], INPUT);        // Set rows pins in high-impedance state
  }
  bootBlink(9);

  // while (!Serial.dtr());                   // wait for user to start the serial monitor
  bootBlink(6);
  delay(500);
}

void loop() {

  if (scan) {
    for (int row = 0; row < ROWS; row++) {
      // Set row pin as output + 3.3V
      pinMode(rowPins[row], OUTPUT);
      digitalWrite(rowPins[row], HIGH);
      for (int column = 0; column < COLS; column++) {
        int value = analogRead(columnPins[column]); // Read the sensor value
        int sensorID = row * ROWS + column; // Calculate the index of the unidimensional array

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
          value = constrain(value, 0, 255);
          myPacket[sensorID] = (byte)value;
          // myPacket[sensorID] = (byte)log2optim(value); // NOT WOKING
        }
      }
      // Set row pin in high-impedance state
      pinMode(rowPins[row], INPUT);
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

/////////// Called with interrupt triggered with push button attached to I/O pin 32
void  pushButton() {
  cli();
  calibrationCounter = 0;
  calibration = true; // Activate the calibration process
  bootBlink(3);
  sei();
}

/////////// Blink fonction
void bootBlink(int flash) {
  for (int i = 0; i < flash; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(50);
    digitalWrite(LED_PIN, LOW);
    delay(50);
  }
}

