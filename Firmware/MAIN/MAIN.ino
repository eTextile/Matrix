/*
  This file is part of the eTextile-matrix-sensor project - http://matrix.eTextile.org
  Copyright (c) 2014-2018 Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

// FPS with CPU speed to 120 MHz (Overclock)
// 523 FPS ADC input
//  24 FPS with BILINEAR_INTERPOLATION
//  23 FPS with interpolation & blob tracking

#include "main.h"

void setup() {
  // pinMode(BUILTIN_LED, OUTPUT); // FIXME - BUILTIN_LED is used for SPI hardware

#ifdef E256_SERIAL
  // Select MODE while booting
  Serial.begin(BAUD_RATE);   // Arduino serial standard library ** 230400 **
  while (!Serial.dtr());     // Wait for user to start the serial monitor
  uint8_t waitOn = 0;
  sensor = 0;

  Serial.printf(F("\nWaiting for config: "));
  while (1) {
    if (Serial.available()) {
      serialConf[sensor] = Serial.read();
      if (serialConf[sensor] == E256_EOF) {
        Serial.printf(F("\nMODE = %d\n"), serialConf[0]); // TODO
        // bootBlink(BUILTIN_LED, 3); // FIXME - BUILTIN_LED is used for SPI hardware
        break;
      }
      sensor++;
    }
    if ((millis() - lastFarme) >= 500) {
      Serial.printf(F("."));
      lastFarme = millis();
      fps = 0;
      waitOn++;
      if (waitOn == 5) {
        break;
      }
    }
  }
#endif /*__E256_SERIAL__*/

  // pinMode(BUTTON_PIN, INPUT_PULLUP);          // Set button pin as input and activate the input pullup resistor // FIXME - NO BUTTON_PIN ON the E256!
  // attachInterrupt(BUTTON_PIN, calib, RISING); // Attach interrrupt on button PIN // FIXME - NO BUTTON_PIN ON the E256

  // SPI.setSCK(E256_SS_PIN);        // D10 - Hardware SPI no need to specify it!
  // SPI.setSCK(E256_SCK_PIN);       // D13 - Hardware SPI no need to specify it!
  // SPI.setMOSI(E256_MOSI_PIN);     // D11 - Hardware SPI no need to specify it!

  pinMode(E256_SS_PIN, OUTPUT);
  pinMode(E256_SCK_PIN, OUTPUT);
  pinMode(E256_MOSI_PIN, OUTPUT);
  digitalWrite(E256_SS_PIN, LOW);    // Set latchPin LOW
  digitalWrite(E256_SS_PIN, HIGH);   // Set latchPin HIGH
  SPI.begin();                       // Start the SPI module
  SPI.beginTransaction(settings);    // (16000000, MSBFIRST, SPI_MODE0);

  pinMode(ADC0_PIN, INPUT);          // Teensy PIN_A9
  pinMode(ADC1_PIN, INPUT);          // Teensy PIN_A3

#ifdef E256_ADC_SYNCHRO
  adc->setAveraging(1, ADC_0);   // set number of averages
  adc->setResolution(8, ADC_0);  // set bits of resolution
  adc->setConversionSpeed(ADC_CONVERSION_SPEED::VERY_HIGH_SPEED, ADC_0); // Change the conversion speed
  // adc->setConversionSpeed(ADC_CONVERSION_SPEED::HIGH_SPEED, ADC_0);   // Change the conversion speed
  adc->setSamplingSpeed(ADC_SAMPLING_SPEED::VERY_HIGH_SPEED, ADC_0);     // Change the sampling speed
  // adc->setSamplingSpeed(ADC_SAMPLING_SPEED::HIGH_SPEED, ADC_0);       // Change the sampling speed
  // adc->enableCompare(1.0 / 3.3 * adc->getMaxValue(ADC_0), 0, ADC_0);  // Measurement will be ready if value < 1.0V

  adc->setAveraging(1, ADC_1);   // set number of averages
  adc->setResolution(8, ADC_1);  // set bits of resolution
  adc->setConversionSpeed(ADC_CONVERSION_SPEED::VERY_HIGH_SPEED, ADC_1); // Change the conversion speed
  // adc->setConversionSpeed(ADC_CONVERSION_SPEED::HIGH_SPEED, ADC_1);   // Change the conversion speed
  adc->setSamplingSpeed(ADC_SAMPLING_SPEED::VERY_HIGH_SPEED, ADC_1);     // Change the sampling speed
  // adc->setSamplingSpeed(ADC_SAMPLING_SPEED::HIGH_SPEED, ADC_1);       // Change the sampling speed
  // adc->enableCompare(1.0 / 3.3 * adc->getMaxValue(ADC_1), 0, ADC_1);  // Measurement will be ready if value < 1.0V
#endif /*__E256_ADC_SYNCHRO__*/

  interpolate.numCols = COLS;
  interpolate.numRows = ROWS;
  interpolate.pData = frameValues; // 16 x 16

  inputFrame.numCols = NEW_COLS;
  inputFrame.numRows = NEW_ROWS;
  inputFrame.pData = bilinIntOutput; // 64 x 64

  llist_raz(&freeBlobs);
  llist_init(&freeBlobs, blobArray, MAX_NODES); // add 40 nodes in the freeBlobs linked list
  llist_raz(&blobs);
  llist_raz(&outputBlobs);

  calib();
  // bootBlink(BUILTIN_LED, 9); // FIXME - BUILTIN_LED is used for hardware SPI
  Serial.printf("\n Calibrated!");

#ifdef E256_OSC
  SLIPSerial.begin(BAUD_RATE); // Extended Arduino serial library
#endif /*__E256_OSC__*/
}

void loop() {

#ifdef E256_ADC
  // Columns are digital OUTPUT PINS - We scan one column at a time
  // Rows are analog INPUT PINS - We read two rows at a time
  uint16_t setCols = 0x8000;

  for (uint8_t col = 0; col < COLS; col++) {        // 0 to 15
    for (uint8_t row = 0; row < DUAL_ROWS; row++) { // 0 to 7

      digitalWriteFast(E256_SS_PIN, LOW);     // Set latchPin LOW
      // delayMicroseconds(10);
      SPI.transfer(setCols & 0xff);           // Shift out the LSB byte to set up the OUTPUT shift register
      SPI.transfer(setCols >> 8);             // Shift out the MSB byte to set up the OUTPUT shift register
      SPI.transfer(setDualRows[row]);         // Shift out one byte that setup the two 8:1 analog multiplexers
      digitalWriteFast(E256_SS_PIN, HIGH);    // Set latchPin HIGH
      // delayMicroseconds(100);

      uint8_t rowIndexA = row * COLS + col;    // Can be optimized later
      uint8_t rowIndexB = rowIndexA + 128;     // ROW_FRAME/2 == 128

#ifdef E256_ADC_SYNCHRO
      result = adc->analogSynchronizedRead(ADC0_PIN, ADC1_PIN);
      frameValues[rowIndexA] = constrain(result.result_adc0 - minVals[rowIndexA], 0, 255); // float32_t frameValues[ROW_FRAME]
      frameValues[rowIndexB] = constrain(result.result_adc1 - minVals[rowIndexB], 0, 255);
#else
      frameValues[rowIndexA] =  constrain(analogRead(ADC0_PIN) - minVals[rowIndexA], 0, 255); // float32_t frameValues[ROW_FRAME];
      frameValues[rowIndexB] =  constrain(analogRead(ADC1_PIN) - minVals[rowIndexB], 0, 255); // float32_t frameValues[ROW_FRAME];
#endif /*__E256_ADC_SYNCHRO__*/
    }
    setCols = setCols >> 1;
    // delay(10);
  }

#ifdef DEBUG_ADC
  sensor = 0;
  for (uint8_t col = 0; col < COLS; col++) {
    for (uint8_t row = 0; row < ROWS; row++) {
      Serial.printf(F("\t%d"), (uint8_t) frameValues[sensor]);
      sensor++;
    }
    Serial.println();
  }
  Serial.println();
  delay(500);
#endif /*__DEBUG_ADC__*/
#endif /*__E256_ADC__*/

  //////////////////// Bilinear intrerpolation
#ifdef E256_INTERP
  float rowInterPos = 0;
  float colInterPos = 0;
  sensor = 0;

  for (uint8_t col = 0; col < NEW_COLS; col++) {
    colInterPos = (float) col / SCALE;
    for (uint8_t row = 0; row < NEW_ROWS; row++) {
      rowInterPos = (float) row / SCALE;
      bilinIntOutput[sensor] = (uint8_t) arm_bilinear_interp_f32(&interpolate, rowInterPos, colInterPos);
      sensor++;
    }
  }

#ifdef DEBUG_INTERP
  sensor = 0;
  for (uint8_t col = 0; col < NEW_COLS; col++) {
    for (uint8_t row = 0; row < NEW_ROWS; row++) {
      Serial.printf(F(" %d"), (uint8_t) bilinIntOutput[sensor]);
      sensor++;
    }
    Serial.println();
  }
  Serial.println();
  delay(500);
#endif /*__DEBUG_INTERP__*/
#endif /*__E256_INTERP__*/

#ifdef E256_BLOB
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
#endif /*__E256_BLOB__*/

#ifdef E256_FPS
  if ((millis() - lastFarme) >= 1000) {
    Serial.printf(F("\nFPS: %d"), fps);
    lastFarme = millis();
    fps = 0;
  }
  fps++;
  Serial.println();
#endif /*__E256_FPS__*/

}

void calib(void) {

  memset(minVals, 0, sizeof(uint8_t) * ROW_FRAME); // Set all values to 0

  for (uint8_t i = 0; i < CYCLES; i++) {

    // Columns are digital OUTPUT PINS
    // Rows are analog INPUT PINS
    // uint16_t setCols = 0x8080; // Powering two cols at a time (NOTGOOD) -> 1000 0000 1000 0000
    uint16_t setCols = 0x8000;    // We must powering one col at a time (GOOD) -> 1000 0000 0000 0000

    for (uint8_t col = 0; col < COLS; col++) {
      for (uint8_t row = 0; row < DUAL_ROWS; row++) {

        digitalWriteFast(E256_SS_PIN, LOW);   // Set latchPin LOW
        // delayMicroseconds(10);
        SPI.transfer(setCols & 0xff);         // Shift out the LSB byte to set up the OUTPUT shift register
        SPI.transfer(setCols >> 8);           // Shift out the MSB byte to set up the OUTPUT shift register
        SPI.transfer(setDualRows[row]);       // Shift out one byte that setup the two 8:1 analog multiplexers
        digitalWriteFast(E256_SS_PIN, HIGH);  // Set latchPin HIGH
        // delayMicroseconds(100);

#ifdef E256_ADC_SYNCHRO
        result = adc->analogSynchronizedRead(ADC0_PIN, ADC1_PIN);
        uint8_t ADC0_val = result.result_adc0;
        uint8_t ADC1_val = result.result_adc1;
#else
        uint8_t ADC0_val =  analogRead(ADC0_PIN);; // float32_t frameValues[ROW_FRAME]
        uint8_t ADC1_val =  analogRead(ADC1_PIN);;
#endif /*__E256_ADC_SYNCHRO__*/

        uint8_t rowIndexA = row * COLS + col;    // Can be optimized later
        uint8_t rowIndexB = rowIndexA + 128;     // ROW_FRAME/2 == 128

        if (ADC0_val > minVals[rowIndexA]) {
          minVals[rowIndexA] = ADC0_val;
        }
        if (ADC1_val > minVals[rowIndexB]) {
          minVals[rowIndexB] = ADC1_val;
        }
      }
      setCols = setCols >> 1;
      delay(1);
    }
  }
}

void bootBlink(const uint8_t pin, uint8_t flash) {
  for (uint8_t i = 0; i < flash; i++) {
    digitalWrite(pin, HIGH);
    delay(50);
    digitalWrite(pin, LOW);
    delay(50);
  }
}

