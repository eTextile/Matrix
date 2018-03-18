/*
  This file is part of the eTextile-matrix-sensor project - http://matrix.eTextile.org
  Copyright (c) 2014-2018 Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

// FPS with CPU speed to 120 MHz (overclock)
// 523 FPS with no interpolation & no blob tracking
// 23 FPS - with interpolation
// 24 FPS with interpolation & blob tracking.

#include "main.h"

void setup() {

  // au demarage lire le port serie pour voir si 
  
  pinMode(SS, OUTPUT);      // Set up slave mode
  digitalWrite(SS, LOW);    // Set latchPin LOW
  digitalWrite(SS, HIGH);   // Set latchPin HIGH
  SPI.begin();              // Start the SPI module
  SPI.beginTransaction(settings);

  pinMode(A0_PIN, INPUT);
  pinMode(A1_PIN, INPUT);

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

  adc->startSynchronizedContinuous(A0_PIN, A1_PIN);
  #ifdef DEBUG_OSC
    Serial.begin(BAUD_RATE); // Arduino serial standard library
    while (!Serial.dtr());  // Wait for user to start the serial monitor
  #else
    SLIPSerial.begin(BAUD_RATE); // Extended Arduino serial library
  #endif
  
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

void loop() {

  if ((millis() - lastFarme) >= 1000) {
    Serial.printf(F("\nFPS: %d"), fps);
    lastFarme = millis();
    fps = 0;
  }

  // Columns are digital OUTPUT PINS
  // Rows are analog INPUT PINS
  // uint16_t setCols = 0x8080; // Powering two cols at a time (NOTGOOD) -> 1000 0000 1000 0000
  uint16_t setCols = 0x8000;    // We must powering one col at a time (GOOD) -> 1000 0000 0000 0000
  uint8_t index = 0;

  for (uint8_t col = 0; col < COLS; col++) {
    for (uint8_t row = 0; row < DUAL_ROWS; row++) {

      digitalWrite(SS, LOW);              // Set latchPin LOW
      SPI.transfer(lowByte(setCols));     // Shift out the LSB byte to set up the OUTPUT shift register
      SPI.transfer(highByte(setCols));    // Shift out the MSB byte to set up the OUTPUT shift register
      SPI.transfer(dualSetRows[row]);     // Shift out one byte that setup the two 8:1 analog multiplexers
      digitalWrite(SS, HIGH);             // Set latchPin HIGH

      // result = adc->analogSynchronizedRead(A0_PIN, A1_PIN);
      result = adc->readSynchronizedContinuous();

      frameValues[index] = constrain(result.result_adc0 - minVals[index], 0, 255);
      frameValues[index + DUAL_ROW_FRAME] = constrain(result.result_adc1 - minVals[index + DUAL_ROW_FRAME], 0, 255);

      index += 1;
    }
    setCols = setCols >> 1;
  }

  //////////////////// Bilinear intrerpolation
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

