/*
  This file is part of the eTextile-matrix-sensor project - http://matrix.eTextile.org
  Copyright (c) 2014-2018 Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

// FPS with CPU speed to 120 MHz (overclock)
// 523 FPS with no interpolation & no blob tracking
// 24 FPS - with interpolation
// 23 FPS with interpolation & blob tracking.

#include "main.h"

void setup() {

#ifdef E256_SERIAL
  // Select MODE while booting
  Serial.begin( BAUD_RATE ); // Arduino serial standard library ** 230400 **
  while (!Serial.dtr());     // Wait for user to start the serial monitor
  /*
    while (1) {
      Serial.println("Waiting for SETUP!");
      if (Serial.available()) {
        debugMode = Serial.read();
        if (debugMode == 'A') {
          break;
        }
        Serial.println("A");
        bootBlink(BUILTIN_LED, 3);
      }
    }
  */
  delay(500);
#endif

#ifdef E256_OSC
  SLIPSerial.begin( BAUD_RATE ); // Extended Arduino serial library
#endif

  pinMode(BUILTIN_LED, OUTPUT);          // Set BUILTIN_LED pin as output

  // pinMode(BUTTON_PIN, INPUT_PULLUP);  // Set button pin as input and activate the input pullup resistor // NO BUTTON_PIN ON BOARD E256 (V2.0)
  // attachInterrupt(BUTTON_PIN, calib, RISING); // Attach interrrupt on button PIN // NO BUTTON_PIN ON BOARD E256 (V2.0)

  SPI.setMOSI(E256_MOSI_PIN);   // D11
  SPI.setMISO(E256_MISO_PIN);   // D10
  SPI.setSCK(E256_SCK_PIN);     // D14

  pinMode(SS, OUTPUT);      // Set up slave mode
  digitalWrite(SS, LOW);    // Set latchPin LOW
  digitalWrite(SS, HIGH);   // Set latchPin HIGH
  SPI.begin();              // Start the SPI module
  SPI.beginTransaction(settings); // (16000000, MSBFIRST, SPI_MODE0);

  pinMode(A0_PIN, INPUT);
  pinMode(A1_PIN, INPUT);

  adc->setAveraging(1, ADC_0);   // set number of averages
  adc->setResolution(8, ADC_0);  // set bits of resolution
  adc->setConversionSpeed(ADC_CONVERSION_SPEED::VERY_HIGH_SPEED, ADC_0); // Change the conversion speed
  adc->setSamplingSpeed(ADC_SAMPLING_SPEED::VERY_HIGH_SPEED, ADC_0);     // Change the sampling speed
  // adc->enableCompare(1.0 / 3.3 * adc->getMaxValue(ADC_0), 0, ADC_0);  // Measurement will be ready if value < 1.0V

  adc->setAveraging(1, ADC_1);   // set number of averages
  adc->setResolution(8, ADC_1);  // set bits of resolution
  adc->setConversionSpeed(ADC_CONVERSION_SPEED::VERY_HIGH_SPEED, ADC_1); // Change the conversion speed
  adc->setSamplingSpeed(ADC_SAMPLING_SPEED::VERY_HIGH_SPEED, ADC_1);     // Change the sampling speed
  // adc->enableCompare(1.0 / 3.3 * adc->getMaxValue(ADC_1), 0, ADC_1);  // Measurement will be ready if value < 1.0V

  adc->startSynchronizedContinuous(A0_PIN, A1_PIN);

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
  delay(100);
}

void loop() {
  // Columns are digital OUTPUT PINS
  // Rows are analog INPUT PINS
  // uint16_t setCols = 0x8080; // Powering two cols at a time (NOTGOOD) -> 1000 0000 1000 0000
  uint16_t setCols = 0x8000;    // We must powering one col at a time (GOOD) -> 1000 0000 0000 0000
  uint8_t index = 0;

  for (uint8_t col = 0; col < COLS; col++) { // 0 to 15
    for (uint8_t row = 0; row < DUAL_ROWS; row++) { // 0 to 7

      digitalWrite(SS, LOW);              // Set latchPin LOW
      SPI.transfer(lowByte(setCols));     // Shift out the LSB byte to set up the OUTPUT shift register
      SPI.transfer(highByte(setCols));    // Shift out the MSB byte to set up the OUTPUT shift register
      SPI.transfer(setDualRows[row]);     // Shift out one byte that setup the two 8:1 analog multiplexers
      digitalWrite(SS, HIGH);             // Set latchPin HIGH

      result = adc->analogSynchronizedRead(A0_PIN, A1_PIN);
      // result = adc->readSynchronizedContinuous();

      frameValues[index] = (float32_t) constrain(result.result_adc0 - minVals[index], 0, 255); // float32_t frameValues[ROW_FRAME]
      frameValues[127 + index] = (float32_t) constrain(result.result_adc1 - minVals[127 + index], 0, 255);
      Serial.println((uint16_t)frameValues[127 + index]);
      index += 1;

    }
    setCols = setCols >> 1;
  }

#ifdef DEBUG_ADC_INPUT
  for (uint8_t index = 0; index < ROW_FRAME; index++) { // 16_ROWS x 16_COLC
    Serial.print((int)frameValues[index]);
  }
  Serial.print("\n DEBUG_ADC_INPUT!");
  delay(5000);
#endif

  /*
    //////////////////// Bilinear intrerpolation
    float rowInterPos = 0;
    float colInterPos = 0;
    uint16_t sensorID = 0;

    for (uint8_t rowPos = 0; rowPos < NEW_ROWS; rowPos++) {
      rowInterPos = (float) rowInterPos / SCALE;
      for (uint8_t colPos = 0; colPos < NEW_COLS; colPos++) {
        colInterPos = (float) colInterPos / SCALE;
        bilinIntOutput[sensorID] = (uint8_t) arm_bilinear_interp_f32(&interpolate, rowInterPos, colInterPos);
        if (DEBUG_INTERP) Serial.printf(" %d", bilinIntOutput[sensorID]);
        sensorID++;
        Serial.printf(F("\n sensorID = %d "), sensorID);
      }
      if (DEBUG_INTERP) Serial.println();
    }
    if (DEBUG_INTERP) Serial.println();


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
  */

  if ((millis() - lastFarme) >= 1000) {
    Serial.printf(F("\nFPS: %d"), fps);
    lastFarme = millis();
    fps = 0;
  }
  fps++;

  Serial.println();
}

void calib() {
  memset(minVals, 0, sizeof(uint8_t) * ROW_FRAME); // Set all values to 0

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
      SPI.transfer(setDualRows[row]);     // Shift out one byte that setup the two 8:1 analog multiplexers
      digitalWrite(SS, HIGH);             // Set latchPin HIGH

      // result = adc->analogSynchronizedRead(A0_PIN, A1_PIN);
      result = adc->readSynchronizedContinuous();

      minVals[index] = (uint8_t) constrain(result.result_adc0 - minVals[index], 0, 255);
      minVals[127 + index] = (uint8_t) constrain(result.result_adc1 - minVals[127 + index], 0, 255);

      index += 1;
    }
    setCols = setCols >> 1;
  }

  bootBlink(BUILTIN_LED, 9);
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

