/*
  This file is part of the eTextile-matrix-sensor project - http://matrix.eTextile.org
  Copyright (c) 2014-2018 Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "main.h"

void setup() {

  // Serial.begin(BAUD_RATE); // Arduino serial standard library
  // while (!Serial.dtr());  // Wait for user to start the serial monitor
  SLIPSerial.begin(BAUD_RATE); // Extended Arduino serial library

  for (uint8_t i = 0; i < COLS; i++) {
    pinMode(colPins[i], INPUT);
  }

  adc->setAveraging(1, ADC_0);   // set number of averages
  adc->setResolution(8, ADC_0);  // set bits of resolution
  adc->setConversionSpeed(ADC_CONVERSION_SPEED::VERY_HIGH_SPEED, ADC_0); // Change the conversion speed
  adc->setSamplingSpeed(ADC_SAMPLING_SPEED::VERY_HIGH_SPEED, ADC_0);     // Change the sampling speed
  adc->enableCompare(1.0 / 3.3 * adc->getMaxValue(ADC_0), 0, ADC_0);  // Measurement will be ready if value < 1.0V

  adc->setAveraging(1, ADC_1);   // set number of averages
  adc->setResolution(8, ADC_1);  // set bits of resolution
  adc->setConversionSpeed(ADC_CONVERSION_SPEED::VERY_HIGH_SPEED, ADC_1); // Change the conversion speed
  adc->setSamplingSpeed(ADC_SAMPLING_SPEED::VERY_HIGH_SPEED, ADC_1);     // Change the sampling speed
  adc->enableCompare(1.0 / 3.3 * adc->getMaxValue(ADC_1), 0, ADC_1);  // Measurement will be ready if value < 1.0V

  pinMode(BUILTIN_LED, OUTPUT);          // Set BUILTIN_LED pin as output
  pinMode(BUTTON_PIN, INPUT_PULLUP);     // Set button pin as input and activate the input pullup resistor
  attachInterrupt(BUTTON_PIN, calib, RISING); // Attach interrrupt on button PIN

  interpolate.numCols = COLS;
  interpolate.numRows = ROWS;
  interpolate.pData = frameValues; // 16 x 16

  inputFrame.numCols = NEW_COLS;
  inputFrame.numRows = NEW_ROWS;
  inputFrame.pData = bilinIntOutput; // 64 x 64

  llist_raz(&freeBlobs);
  llist_init(&freeBlobs, blobArray, MAX_NODES); // 1O nodes
  llist_raz(&blobs);
  llist_raz(&outputBlobs);

  calib();
  bootBlink(BUILTIN_LED, 9);
}

// FPS with CPU speed to 120 MHz (overclock)
// 523 FPS with no interpolation & no blob tracking
// 23 FPS - with interpolation
// 24 FPS with interpolation & blob tracking.

void loop() {

  if ((millis() - lastFarme) >= 1000) {
    Serial.printf(F("\nFPS: %d"), fps);
    lastFarme = millis();
    fps = 0;
  }

  for (uint8_t row = 0; row < ROWS; row++) {
    pinMode(rowPins[row], OUTPUT);    // Set row pin as output
    digitalWrite(rowPins[row], HIGH); // Set row pin HIGH (3.3V)
    for (uint8_t col = 0; col < COLS; col++) {
      uint8_t rowValue = adc->analogRead(colPins[col]); // Read the sensor value
      frameValues[sensorID] = constrain(rowValue - minVals[sensorID], 0, 255);
      if (DEBUG_ADC_INPUT) Serial.printf(F("%d "), (uint8_t)frameValues[sensorID]);
      sensorID++;
    }
    if (DEBUG_ADC_INPUT) Serial.printf(F("\n"));
    // pinMode(rowPins[row], INPUT); // Set row pin in high-impedance state
    digitalWrite(rowPins[row], LOW); // Set row pin to GND (Should avoid ghosts)
  }
  if (DEBUG_ADC_INPUT) Serial.printf(F("\n"));
  sensorID = 0;

  float rowPos = 0;
  float colPos = 0;

  for (uint8_t row = 0; row < NEW_ROWS; row++) {
    rowPos = (float) row / SCALE;
    for (uint8_t col = 0; col < NEW_COLS; col++) {
      colPos = (float) col / SCALE;
      bilinIntOutput[sensorID] = (uint8_t) arm_bilinear_interp_f32(&interpolate, rowPos, colPos);
      if (DEBUG_INTERP) Serial.printf(" %d", bilinIntOutput[sensorID]);
      sensorID++;
    }
    if (DEBUG_INTERP) Serial.println();
  }
  if (DEBUG_INTERP) Serial.println();
  sensorID = 0;

  find_blobs(
    &inputFrame,        // image_t 64 x 64 (1D array) uint8_t
    bitmap,             // char Array
    NEW_ROWS,           // const int
    NEW_COLS,           // const int
    THRESHOLD,          // const int
    MIN_BLOB_PIX,       // const int
    MAX_BLOB_PIX,       // const int
    &freeBlobs,         // list_t
    &blobs,             // list_t
    &outputBlobs,       // list_t
    SLIPSerial          // SLIPEncodedUSBSerial
  );

  fps++;
}

void calib() {
  memset(minVals, 0, sizeof(uint8_t) * ROW_FRAME); // Set all values to 0
  for (uint8_t i = 0; i < CYCLES; i++) {
    uint8_t pos = 0;
    for (uint8_t row = 0; row < ROWS; row++) {
      pinMode(rowPins[row], OUTPUT);  // Set row pin as output
      digitalWrite(rowPins[row], HIGH);
      for (uint8_t col = 0; col < COLS; col++) {
        uint8_t rowVal = adc->analogRead(colPins[col]); // Read the sensor value
        if (rowVal > minVals[pos]) minVals[pos] = rowVal;
        pos++;
      }
      // pinMode(rowPins[row], INPUT); // Set row pin in high-impedance state
      digitalWrite(rowPins[row], LOW); // Set row pin to GND (Should avoid ghosts)
    }
  }
  bootBlink(BUILTIN_LED, 3);
  Serial.printf("\n Calibrated!");
}

void bootBlink(const uint8_t pin, uint8_t flash) {
  for (uint8_t i = 0; i < flash; i++) {
    digitalWrite(pin, HIGH);
    delay(50);
    digitalWrite(pin, LOW);
    delay(50);
  }
}

