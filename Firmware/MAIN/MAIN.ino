/*
  ** E256 Firmware v1.0 **
  This file is part of the eTextile-matrix-sensor project - http://matrix.eTextile.org
  Copyright (c) 2014-2018 Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "main.h"

// FPS with CPU speed to 120 MHz (Overclock)
// 523 FPS ADC input
//  24 FPS with BILINEAR_INTERPOLATION
//  23 FPS with interpolation & blob tracking

uint8_t E256_threshold = 36; // Set the threshold that determine toutch sensitivity (10 is low 30 is high)

//OSCMessage slip_osc_out("/b");
OSCBundle OSCbundle;
//////////////////////////////////////////////////// SETUP

void setup() {

  //pinMode(LED_BUILTIN, OUTPUT); // FIXME - BUILTIN_LED is used for SPI hardware
  //digitalWrite(LED_BUILTIN, LOW); // FIXME - BUILTIN_LED is used for SPI hardware

#ifdef E256_BLOBS_SLIP_OSC
  SLIPSerial.begin(BAUD_RATE);   // Arduino serial library ** 230400 ** extended with SLIP encoding
#else
  Serial.begin(BAUD_RATE);       // Arduino serial library ** 230400 **
  while (!Serial.dtr());         // Wait for user to start the serial monitor
#endif /*__E256_BLOBS_SLIP_OSC__*/

  //pinMode(BUTTON_PIN, INPUT_PULLUP);          // Set button pin as input and activate the input pullup resistor // FIXME - NO BUTTON_PIN ON the E256!
  //attachInterrupt(BUTTON_PIN, calib, RISING); // Attach interrrupt on button PIN // FIXME - NO BUTTON_PIN ON the E256

  //SPI.setSCK(E256_SS_PIN);        // D10 - Hardware SPI no need to specify it!
  //SPI.setSCK(E256_SCK_PIN);       // D13 - Hardware SPI no need to specify it!
  //SPI.setMOSI(E256_MOSI_PIN);     // D11 - Hardware SPI no need to specify it!

  pinMode(E256_SS_PIN, OUTPUT);
  pinMode(E256_SCK_PIN, OUTPUT);
  pinMode(E256_MOSI_PIN, OUTPUT);
  digitalWriteFast(E256_SS_PIN, LOW);    // Set latchPin LOW
  digitalWriteFast(E256_SS_PIN, HIGH);   // Set latchPin HIGH
  SPI.begin();                           // Start the SPI module
  SPI.beginTransaction(settings);        // (16000000, MSBFIRST, SPI_MODE0);

  pinMode(ADC0_PIN, INPUT);              // Teensy PIN A9
  pinMode(ADC1_PIN, INPUT);              // Teensy PIN A3

#ifdef E256_ADC_SYNCHRO
  adc->setAveraging(1, ADC_0);                                           // Set number of averages
  adc->setResolution(8, ADC_0);                                          // Set bits of resolution
  adc->setConversionSpeed(ADC_CONVERSION_SPEED::VERY_HIGH_SPEED, ADC_0); // Change the conversion speed
  //adc->setConversionSpeed(ADC_CONVERSION_SPEED::HIGH_SPEED, ADC_0);   // Change the conversion speed
  adc->setSamplingSpeed(ADC_SAMPLING_SPEED::VERY_HIGH_SPEED, ADC_0);     // Change the sampling speed
  //adc->setSamplingSpeed(ADC_SAMPLING_SPEED::HIGH_SPEED, ADC_0);       // Change the sampling speed
  //adc->enableCompare(1.0 / 3.3 * adc->getMaxValue(ADC_0), 0, ADC_0);  // Measurement will be ready if value < 1.0V

  adc->setAveraging(1, ADC_1);                                           // Set number of averages
  adc->setResolution(8, ADC_1);                                          // Set bits of resolution
  adc->setConversionSpeed(ADC_CONVERSION_SPEED::VERY_HIGH_SPEED, ADC_1); // Change the conversion speed
  // adc->setConversionSpeed(ADC_CONVERSION_SPEED::HIGH_SPEED, ADC_1);   // Change the conversion speed
  adc->setSamplingSpeed(ADC_SAMPLING_SPEED::VERY_HIGH_SPEED, ADC_1);     // Change the sampling speed
  //adc->setSamplingSpeed(ADC_SAMPLING_SPEED::HIGH_SPEED, ADC_1);       // Change the sampling speed
  //adc->enableCompare(1.0 / 3.3 * adc->getMaxValue(ADC_1), 0, ADC_1);  // Measurement will be ready if value < 1.0V
#else
  analogReadRes(8); // Set the ADC converteur resolution to 10 bit
#endif /*__E256_ADC_SYNCHRO__*/

  // Raw frame init
  rawFrame.numCols = COLS;
  rawFrame.numRows = ROWS;
  rawFrame.pData = frameValues; // 16 x 16 // uint8_t frameValues[ROW_FRAME];

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
  //bootBlink(LED_BUILTIN, 9); // FIXME - BUILTIN_LED is used for hardware SPI

  //timerFps = 0;
}

//////////////////////////////////////////////////// LOOP
void loop() {

  OSCMessage OSCmsg;
  int size;
  while (!SLIPSerial.endofPacket()) {
    if ((size = SLIPSerial.available()) > 0) {
      while (size--)
        OSCmsg.fill(SLIPSerial.read());
    }
  }
  if (!OSCmsg.hasError()) {
    OSCmsg.dispatch("/calibrate", matrix_calibration);
    OSCmsg.dispatch("/threshold", matrix_threshold);
    OSCmsg.dispatch("/rowData", matrix_raw_data); // FIXME
    OSCmsg.dispatch("/blobs", matrix_blobs);
  }

#ifdef E256_FPS
  if (timerFPS >= 1000) {
    timerFPS = 0;
    fps = 0;
    Serial.printf(F("\nFPS: %d"), fps);
  }
  fps++;
#endif /*__E256_FPS__*/
}

//////////////////////////////////////////////////// FONCTIONS

void matrix_scan(void) {

  // Columns are digital OUTPUT PINS - We supply one column at a time
  // Rows are analog INPUT PINS - We sens two rows at a time
  uint16_t setCols = 0x8000;

  for (uint8_t col = 0; col < COLS; col++) {        // 0 to 15
    for (uint8_t row = 0; row < DUAL_ROWS; row++) { // 0 to 7

      digitalWriteFast(E256_SS_PIN, LOW);      // Set latchPin LOW
      SPI.transfer(setCols & 0xff);            // Shift out the LSB byte to set up the OUTPUT shift register
      SPI.transfer(setCols >> 8);              // Shift out the MSB byte to set up the OUTPUT shift register
      SPI.transfer(setDualRows[row]);          // Shift out one byte that setup the two 8:1 analog multiplexers
      digitalWriteFast(E256_SS_PIN, HIGH);     // Set latchPin HIGH
      delayMicroseconds(5);                    // See switching time of the 74HC4051BQ multiplexeur

      uint8_t rowIndexA = row * COLS + col;    // Row IndexA computation
      uint8_t rowIndexB = rowIndexA + 128;     // Row IndexB computation (ROW_FRAME/2 == 128)

#ifdef E256_ADC_SYNCHRO
      result = adc->analogSynchronizedRead(ADC0_PIN, ADC1_PIN);
      //frameValues[rowIndexA] = constrain(result.result_adc0 - minVals[rowIndexA], 0, 255);
      //frameValues[rowIndexB] = constrain(result.result_adc1 - minVals[rowIndexB], 0, 255);

      int valA = result.result_adc0 - minVals[rowIndexA];
      valA >= 0 ? frameValues[rowIndexA] = (uint8_t)valA : frameValues[rowIndexA] = 0;
      int valB = result.result_adc1 - minVals[rowIndexB];
      valB >= 0 ? frameValues[rowIndexB] = (uint8_t)valB : frameValues[rowIndexB] = 0;

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

  //uint8_t cycles = (uint8_t)msg.getInt(0);
  uint8_t cycles = 20;

  for (uint8_t i = 0; i < cycles; i++) {
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
        delayMicroseconds(10);                // See switching time of the 74HC4051BQ multiplexeur

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

void matrix_threshold(OSCMessage &msg) { // TODO Add threshold_ptr
  //E256_threshold = (0xFF | msg.isInt(0));
  E256_threshold = (uint8_t)msg.isInt(0);
}

/// Send raw frame values in SLIP-OSC formmat // FIXME
void matrix_raw_data(OSCMessage &msg) {

  matrix_scan();
  OSCMessage m("/m");
  m.add(frameValues, ROW_FRAME);
  SLIPSerial.beginPacket();
  m.send(SLIPSerial);   // Send the bytes to the SLIP stream
  SLIPSerial.endPacket();    // Mark the end of the OSC Packet
}

/// Blobs detection
void matrix_blobs(OSCMessage &msg) {

  //int32_t debug = msg.getInt(0);

  matrix_scan();

  bilinear_interp(&interpolatedFrame, &rawFrame, &interp);

  find_blobs(
    &interpolatedFrame,    // image_t uint8_t [NEW_FRAME] - 1D array
    bitmap,                // char array [NEW_FRAME] - 1D array // NOT &bitmap !?
    NEW_ROWS,              // const int
    NEW_COLS,              // const int
    E256_threshold,        // uint8_t
    MIN_BLOB_PIX,          // const int
    MAX_BLOB_PIX,          // const int
    &freeBlobs,            // list_t
    &blobs,                // list_t
    &outputBlobs           // list_t
  );

  for (blob_t* blob = iterator_start_from_head(&outputBlobs); blob != NULL; blob = iterator_next(blob)) {
    blobPaket[0] = blob->UID;        // uint8_t
    blobPaket[1] = blob->centroid.X; // uint8_t
    blobPaket[2] = blob->centroid.Y; // uint8_t
    blobPaket[3] = blob->box.W;      // uint8_t
    blobPaket[4] = blob->box.H;      // uint8_t
    blobPaket[5] = blob->box.D;      // uint8_t

    OSCMessage msg("/b");
    msg.add(blobPaket, OSC_PAKET_SIZE);
    OSCbundle.add(msg);
  }

  SLIPSerial.beginPacket();     //
  OSCbundle.send(SLIPSerial);   // Send the bytes to the SLIP stream
  SLIPSerial.endPacket();       // Mark the end of the OSC Packet
  OSCbundle.empty();            // empty the OSCMessage ready to use for new messages
}

/*
  void bootBlink(const uint8_t pin, uint8_t flash) {
  for (uint8_t i = 0; i < flash; i++) {
    digitalWrite(pin, HIGH);
    delay(50);
    digitalWrite(pin, LOW);
    delay(50);
  }
  }
*/
