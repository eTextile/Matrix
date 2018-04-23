/*
  This file is part of the eTextile-matrix-sensor project - http://matrix.eTextile.org
  Copyright (c) 2014-2018 Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "main.h"

// FPS with CPU speed to 120 MHz (Overclock)
// 523 FPS ADC input
//  24 FPS with BILINEAR_INTERPOLATION
//  23 FPS with interpolation & blob tracking

//////////////////////////////////////////////////// SETUP

void setup() {

  // pinMode(BUILTIN_LED, OUTPUT); // FIXME - BUILTIN_LED is used for SPI hardware

#ifdef E256_BLOBS_SLIP_OSC
  SLIPSerial.begin(BAUD_RATE);   // Arduino serial library ** 230400 ** extended with SLIP encoding
#else
  Serial.begin(BAUD_RATE);       // Arduino serial library ** 230400 **
  while (!Serial.dtr());         // Wait for user to start the serial monitor
#endif /*__E256_BLOBS_OSC__*/

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

  pinMode(ADC0_PIN, INPUT);          // Teensy PIN A9
  pinMode(ADC1_PIN, INPUT);          // Teensy PIN A3

#ifdef E256_ADC_SYNCHRO
  adc->setAveraging(1, ADC_0);                                           // Set number of averages
  adc->setResolution(8, ADC_0);                                          // Set bits of resolution
  adc->setConversionSpeed(ADC_CONVERSION_SPEED::VERY_HIGH_SPEED, ADC_0); // Change the conversion speed
  // adc->setConversionSpeed(ADC_CONVERSION_SPEED::HIGH_SPEED, ADC_0);   // Change the conversion speed
  adc->setSamplingSpeed(ADC_SAMPLING_SPEED::VERY_HIGH_SPEED, ADC_0);     // Change the sampling speed
  // adc->setSamplingSpeed(ADC_SAMPLING_SPEED::HIGH_SPEED, ADC_0);       // Change the sampling speed
  // adc->enableCompare(1.0 / 3.3 * adc->getMaxValue(ADC_0), 0, ADC_0);  // Measurement will be ready if value < 1.0V

  adc->setAveraging(1, ADC_1);                                           // Set number of averages
  adc->setResolution(8, ADC_1);                                          // Set bits of resolution
  adc->setConversionSpeed(ADC_CONVERSION_SPEED::VERY_HIGH_SPEED, ADC_1); // Change the conversion speed
  // adc->setConversionSpeed(ADC_CONVERSION_SPEED::HIGH_SPEED, ADC_1);   // Change the conversion speed
  adc->setSamplingSpeed(ADC_SAMPLING_SPEED::VERY_HIGH_SPEED, ADC_1);     // Change the sampling speed
  // adc->setSamplingSpeed(ADC_SAMPLING_SPEED::HIGH_SPEED, ADC_1);       // Change the sampling speed
  // adc->enableCompare(1.0 / 3.3 * adc->getMaxValue(ADC_1), 0, ADC_1);  // Measurement will be ready if value < 1.0V
#else
  analogReadRes(8); // Set the ADC converteur resolution to 10 bit
#endif /*__E256_ADC_SYNCHRO__*/

  // Raw frame init
  rawFrame.numCols = COLS;
  rawFrame.numRows = ROWS;
  rawFrame.pData = frameValues; // 16 x 16 // float32_t frameValues[ROW_FRAME];

  // Interpolate config init
  interp.scale_X = SCALE_X;
  interp.scale_Y = SCALE_Y;
  interp.outputStride_Y = SCALE_X * SCALE_Y * COLS;
  interp.pCoefA = coef_A;
  interp.pCoefB = coef_B;
  interp.pCoefC = coef_C;
  interp.pCoefD = coef_D;

  // Interpolated frame init
  interpolatedFrame.numCols = NEW_COLS;
  interpolatedFrame.numRows = NEW_ROWS;
  interpolatedFrame.pData = bilinIntOutput; // 64 x 64

  // Blobs list init
  llist_raz(&freeBlobs);
  llist_init(&freeBlobs, blobArray, MAX_NODES); // Add 40 nodes in the freeBlobs linked list
  llist_raz(&blobs);
  llist_raz(&outputBlobs);

  bilinear_interp_init(&interp);
  // bootBlink(BUILTIN_LED, 9); // FIXME - BUILTIN_LED is used for hardware SPI

}

//////////////////////////////////////////////////// LOOP

void loop() {

  if (scan) {
    matrix_scan();
#ifdef E256_INTERP
    bilinear_interp(&interpolatedFrame, &rawFrame, &interp);
#endif /*__E256_INTERP__*/
    scan = false;
  }

  int size = 0;

  while (!SLIPSerial.endofPacket()) {
    if ((size = SLIPSerial.available()) > 0) {
      while (size--)
        inputBundle.fill(SLIPSerial.read());
    }
  }
  if (!inputBundle.hasError()) {
    inputBundle.dispatch("/config", matrix_config);
    inputBundle.dispatch("/calibrate", matrix_calibration);
    inputBundle.dispatch("/rowData", matrix_raw_data);
    inputBundle.dispatch("/blobs", matrix_blobs);
  }

#ifdef E256_FPS
  if ((millis() - lastFarme) >= 1000) {
    Serial.printf(F("\nFPS: %d"), fps);
    lastFarme = millis();
    fps = 0;
  }
  fps++;
#endif /*__E256_FPS__*/

}

//////////////////////////////////////////////////// FONCTIONS

inline void matrix_scan() {

  // Columns are digital OUTPUT PINS - We supply one column at a time
  // Rows are analog INPUT PINS - We sens two rows at a time
  uint16_t setCols = 0x8000;

  for (uint8_t col = 0; col < COLS; col++) {        // 0 to 15
    for (uint8_t row = 0; row < DUAL_ROWS; row++) { // 0 to 7

      digitalWriteFast(E256_SS_PIN, LOW);     // Set latchPin LOW
      SPI.transfer(setCols & 0xff);           // Shift out the LSB byte to set up the OUTPUT shift register
      SPI.transfer(setCols >> 8);             // Shift out the MSB byte to set up the OUTPUT shift register
      SPI.transfer(setDualRows[row]);         // Shift out one byte that setup the two 8:1 analog multiplexers
      digitalWriteFast(E256_SS_PIN, HIGH);    // Set latchPin HIGH
      // delayMicroseconds(1);                // See switching time of the 74HC4051BQ multiplexeur

      uint8_t rowIndexA = row * COLS + col;    // Row IndexA computation
      uint8_t rowIndexB = rowIndexA + 128;     // Row IndexB computation (ROW_FRAME/2 == 128)

#ifdef E256_ADC_SYNCHRO
      result = adc->analogSynchronizedRead(ADC0_PIN, ADC1_PIN);
      frameValues[rowIndexA] = constrain(result.result_adc0 - minVals[rowIndexA], 0, 255);
      frameValues[rowIndexB] = constrain(result.result_adc1 - minVals[rowIndexB], 0, 255);
#else
      frameValues[rowIndexA] = constrain(analogRead(ADC0_PIN) - minVals[rowIndexA], 0, 255);
      frameValues[rowIndexB] = constrain(analogRead(ADC1_PIN) - minVals[rowIndexB], 0, 255);
#endif /*__E256_ADC_SYNCHRO__*/
    }
    setCols = setCols >> 1;
  }

#ifdef DEBUG_ADC
  for (uint16_t i = 0; i < NEW_FRAME; i++) {
    if ((i % NEW_COLS) == (NEW_COLS - 1)) Serial.println();
    Serial.printf(F("\t%d"), frameValues[i]);
    delay(1);
  }
  Serial.println();
  delay(500);
#endif /*__DEBUG_ADC__*/
}

void matrix_calibration(OSCMessage &msg) {

  for (uint8_t i = 0; i < CYCLES; i++) {
    // Columns are digital OUTPUT PINS - We supply one column at a time
    // Rows are analog INPUT PINS - We sens two rows at a time
    uint16_t setCols = 0x8000;

    for (uint8_t col = 0; col < COLS; col++) {
      for (uint8_t row = 0; row < DUAL_ROWS; row++) {

        digitalWriteFast(E256_SS_PIN, LOW);   // Set latchPin LOW
        SPI.transfer(setCols & 0xff);         // Shift out the LSB byte to set up the OUTPUT shift register
        SPI.transfer(setCols >> 8);           // Shift out the MSB byte to set up the OUTPUT shift register
        SPI.transfer(setDualRows[row]);       // Shift out one byte that setup the two 8:1 analog multiplexers
        digitalWriteFast(E256_SS_PIN, HIGH);  // Set latchPin HIGH
        delayMicroseconds(5);                 // See switching time of the 74HC4051BQ multiplexeur

#ifdef E256_ADC_SYNCHRO
        result = adc->analogSynchronizedRead(ADC0_PIN, ADC1_PIN);
        uint8_t ADC0_val = result.result_adc0;
        uint8_t ADC1_val = result.result_adc1;
#else
        uint8_t ADC0_val = analogRead(ADC0_PIN);
        uint8_t ADC1_val = analogRead(ADC1_PIN);
#endif /*__E256_ADC_SYNCHRO__*/

        uint8_t rowIndexA = row * COLS + col;    // Row IndexA computation
        uint8_t rowIndexB = rowIndexA + 128;     // Row IndexB computation (ROW_FRAME/2 == 128)

        if (ADC0_val > minVals[rowIndexA]) {
          minVals[rowIndexA] = ADC0_val;
        }
        if (ADC1_val > minVals[rowIndexB]) {
          minVals[rowIndexB] = ADC1_val;
        }
      }
      setCols = setCols >> 1;
    }
  }
}

/// Blobs detection
void matrix_blobs(OSCMessage &msg) {
  find_blobs(
    &interpolatedFrame, // image_t uint8_t [NEW_FRAME] - 1D array
    bitmap,             // char array [NEW_FRAME] - 1D array
    NEW_ROWS,           // const int
    NEW_COLS,           // const int
    THRESHOLD,          // const int
    MIN_BLOB_PIX,       // const int
    MAX_BLOB_PIX,       // const int
    &freeBlobs,         // list_t
    &blobs,             // list_t
    &outputBlobs        // list_t
  );
}

/// Send raw frame values in SLIP-OSC formmat
void matrix_raw_data(OSCMessage &msg) {
  OSCBundle bndl;
  bndl.fill(frameValues, ROW_FRAME * sizeof(uint8_t));
  SLIPSerial.beginPacket();  // Begin SLIP packet
  bndl.send(SLIPSerial);     // Send the bytes to the SLIP stream
  SLIPSerial.endPacket();    // Mark the end of the OSC packet
  bndl.empty();              // Empty the bundle to free room for a new one
}

void matrix_config(OSCMessage &msg) {
  //TODO
}

void bootBlink(const uint8_t pin, uint8_t flash) {
  for (uint8_t i = 0; i < flash; i++) {
    digitalWrite(pin, HIGH);
    delay(50);
    digitalWrite(pin, LOW);
    delay(50);
  }
}

