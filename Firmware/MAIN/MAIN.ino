/*
  ** E256 Firmware v1.0 **
  This file is part of the eTextile-matrix-sensor project - http://matrix.eTextile.org
  Copyright (c) 2014-2019 Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "main.h"

uint8_t E256_threshold = 15;        // Threshold defaultused to adjust toutch sensitivity (10 is low 40 is high)

// FPS with CPU speed to 120 MHz (Overclock)
// Optimize Fastest with LTO (Link Time Optimizations)

// 523 FPS : ADC_INPUT
//  99 FPS : ADC_INPUT / BLOB_TRACKING
//  34 FPS : ADC_INPUT / BILINEAR_INTERPOLATION // FIXME: It have to be optimize!
//  32 FPS : ADC_INPUT / BILINEAR_INTERPOLATION / BLOB_TRACKING

//////////////////////////////////////////////////// SETUP

void setup() {

  //pinMode(LED_BUILTIN, OUTPUT); // FIXME - BUILTIN_LED is used for SPI hardware
  //digitalWrite(LED_BUILTIN, LOW); // FIXME - BUILTIN_LED is used for SPI hardware

#ifdef DEBUG_ADC
#ifdef DEBUG_INTERP
#ifdef DEBUG_BLOBS_OSC
#ifdef DEBUG_FPS
  Serial.begin(BAUD_RATE);       // Arduino serial library ** 230400 **
  while (!Serial.dtr());         // Wait for user to start the serial monitor
#else
  SLIPSerial.begin(BAUD_RATE);   // Arduino serial library ** 230400 ** extended with SLIP encoding
#endif /*__DEBUG_ADC__*/
#endif /*__DEBUG_INTERP__*/
#endif /*__DEBUG_BLOBS_OSC__*/
#endif /*__DEBUG_FPS__*/

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

  adc->setAveraging(1, ADC_0);                                           // Set number of averages
  adc->setResolution(8, ADC_0);                                          // Set bits of resolution
  adc->setConversionSpeed(ADC_CONVERSION_SPEED::VERY_HIGH_SPEED, ADC_0); // Change the conversion speed
  //adc->setConversionSpeed(ADC_CONVERSION_SPEED::HIGH_SPEED, ADC_0);    // Change the conversion speed
  adc->setSamplingSpeed(ADC_SAMPLING_SPEED::VERY_HIGH_SPEED, ADC_0);     // Change the sampling speed
  //adc->setSamplingSpeed(ADC_SAMPLING_SPEED::HIGH_SPEED, ADC_0);        // Change the sampling speed
  //adc->enableCompare(1.0 / 3.3 * adc->getMaxValue(ADC_0), 0, ADC_0);   // Measurement will be ready if value < 1.0V

  adc->setAveraging(1, ADC_1);                                           // Set number of averages
  adc->setResolution(8, ADC_1);                                          // Set bits of resolution
  adc->setConversionSpeed(ADC_CONVERSION_SPEED::VERY_HIGH_SPEED, ADC_1); // Change the conversion speed
  // adc->setConversionSpeed(ADC_CONVERSION_SPEED::HIGH_SPEED, ADC_1);   // Change the conversion speed
  adc->setSamplingSpeed(ADC_SAMPLING_SPEED::VERY_HIGH_SPEED, ADC_1);     // Change the sampling speed
  //adc->setSamplingSpeed(ADC_SAMPLING_SPEED::HIGH_SPEED, ADC_1);        // Change the sampling speed
  //adc->enableCompare(1.0 / 3.3 * adc->getMaxValue(ADC_1), 0, ADC_1);   // Measurement will be ready if value < 1.0V

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
    OSCmsg.dispatch("/c", matrix_calibration);
    OSCmsg.dispatch("/t", matrix_threshold);
    OSCmsg.dispatch("/r", matrix_raw_data);
    OSCmsg.dispatch("/i", matrix_interp_data);
    OSCmsg.dispatch("/b", matrix_blobs);
  }

}

//////////////////////////////////////////////////// FONCTIONS

void matrix_scan(void) {

  // Columns are digital OUTPUT PINS - We supply one column at a time
  // Rows are analog INPUT PINS - We sens two rows at a time
  uint32_t setCols = 0x10000;

  for (uint8_t col = 0; col < COLS; col++) {        // 0 to 15
    setCols = setCols >> 1;
    for (uint8_t row = 0; row < DUAL_ROWS; row++) { // 0 to 7

      digitalWriteFast(E256_SS_PIN, LOW);   // Set latchPin LOW
      SPI.transfer(setCols & 0xFF);         // Shift out the LSB byte to set up the OUTPUT shift register
      SPI.transfer((setCols >> 8) & 0xFF);  // Shift out the MSB byte to set up the OUTPUT shift register
      SPI.transfer(setDualRows[row]);       // Shift out one byte that setup the two 8:1 analog multiplexers
      digitalWriteFast(E256_SS_PIN, HIGH);  // Set latchPin HIGH
      //delayMicroseconds(5);                 // TODO: See switching time of the 74HC4051BQ multiplexeur

      uint8_t rowIndexA = row * COLS + col;    // Row IndexA computation
      uint8_t rowIndexB = rowIndexA + 128;     // Row IndexB computation (ROW_FRAME/2 == 128)

      result = adc->analogSynchronizedRead(ADC0_PIN, ADC1_PIN);

      int valA = result.result_adc0 - minVals[rowIndexA];
      valA > 0 ? frameValues[rowIndexA] = (uint8_t)valA : frameValues[rowIndexA] = 0;
      int valB = result.result_adc1 - minVals[rowIndexB];
      valB > 0 ? frameValues[rowIndexB] = (uint8_t)valB : frameValues[rowIndexB] = 0;
    }
  }
}

void matrix_calibration(OSCMessage & msg) {

  uint8_t calibration_cycles = msg.getInt(0) & 0xFF;   // Get the first uint8_t in an int32_t

  for (uint8_t i = 0; i < calibration_cycles; i++) {
    // Columns are digital OUTPUT PINS - We supply one column at a time
    // Rows are analog INPUT PINS - We sens two rows at a time
    uint32_t setCols = 0x10000;

    for (uint8_t col = 0; col < COLS; col++) {
      setCols = setCols >> 1;
      for (uint8_t row = 0; row < DUAL_ROWS; row++) {

        digitalWriteFast(E256_SS_PIN, LOW);   // Set latchPin LOW
        SPI.transfer(setCols & 0xFF);         // Shift out the LSB byte to set up the OUTPUT shift register
        SPI.transfer((setCols >> 8) & 0xFF);  // Shift out the MSB byte to set up the OUTPUT shift register
        SPI.transfer(setDualRows[row]);       // Shift out one byte that setup the two 8:1 analog multiplexers
        digitalWriteFast(E256_SS_PIN, HIGH);  // Set latchPin HIGH
        //delayMicroseconds(5);                 // TODO: See switching time of the 74HC4051BQ multiplexeur

        result = adc->analogSynchronizedRead(ADC0_PIN, ADC1_PIN);
        uint8_t ADC0_val = result.result_adc0;
        uint8_t ADC1_val = result.result_adc1;

        uint8_t rowIndexA = row * COLS + col;    // Row IndexA computation
        uint8_t rowIndexB = rowIndexA + 128;     // Row IndexB computation (ROW_FRAME/2 == 128)

        if (ADC0_val > minVals[rowIndexA]) minVals[rowIndexA] = ADC0_val;
        if (ADC1_val > minVals[rowIndexB]) minVals[rowIndexB] = ADC1_val;
      }
    }
  }
}

// Set E256 threshold from an incoming OSC message
void matrix_threshold(OSCMessage & msg) {

  // Teensy is Little-endian!
  // The sequence addresses/sends/stores the least significant byte first (lowest address)
  // and the most significant byte last (highest address).
  E256_threshold = msg.getInt(0) & 0xFF; // Get the first uint8_t of the int32_t
}

// Send raw frame values in SLIP-OSC formmat
void matrix_raw_data(OSCMessage & msg) {

  matrix_scan();
  OSCMessage m("/r");
  m.add(frameValues, ROW_FRAME);
  SLIPSerial.beginPacket();
  m.send(SLIPSerial);        // Send the bytes to the SLIP stream
  SLIPSerial.endPacket();    // Mark the end of the OSC Packet
}

// Send bitmap frame values in SLIP-OSC formmat
void matrix_bitmap_data(OSCMessage & msg) {

  matrix_scan();
  OSCMessage m("/x");
  //m.add(&bitmap, NEW_FRAME); // bitmap must be uint8_t // FIXME
  SLIPSerial.beginPacket();
  m.send(SLIPSerial);          // Send the bytes to the SLIP stream
  SLIPSerial.endPacket();      // Mark the end of the OSC Packet
}

// Send interpolated frame values in SLIP-OSC formmat
void matrix_interp_data(OSCMessage & msg) {

  matrix_scan();
  bilinear_interp(&interpolatedFrame, &rawFrame, &interp);
  OSCMessage m("/i");
  m.add(bilinIntOutput, NEW_FRAME);
  SLIPSerial.beginPacket();
  m.send(SLIPSerial);        // Send the bytes to the SLIP stream
  SLIPSerial.endPacket();    // Mark the end of the OSC Packet
}


// Send all blobs values in SLIP-OSC formmat
void matrix_blobs(OSCMessage & msg) {

  matrix_scan();

  bilinear_interp(&interpolatedFrame, &rawFrame, &interp);

  find_blobs(
    //&rawFrame,           // image_t uint8_t [RAW_FRAME] - 1D array
    &interpolatedFrame,    // image_t uint8_t [NEW_FRAME] - 1D array
    bitmap,                // char array [NEW_FRAME] - 1D array // NOT &bitmap !?
    &freeBlobs,            // list_t
    &blobs,                // list_t
    &outputBlobs           // list_t
  );

#ifdef BLOBS_OSC
  OSCBundle OSCbundle;

  // Send all blobs in OCS bundle
  for (blob_t* blob = ITERATOR_START_FROM_HEAD(&outputBlobs); blob != NULL; blob = ITERATOR_NEXT(blob)) {
    blobPacket[0] = blob->UID;        // uint8_t unique session ID
    blobPacket[1] = blob->alive;      // uint8_t
    blobPacket[2] = blob->centroid.X; // uint8_t
    blobPacket[3] = blob->centroid.Y; // uint8_t
    blobPacket[4] = blob->box.W;      // uint8_t
    blobPacket[5] = blob->box.H;      // uint8_t
    blobPacket[6] = blob->box.D;      // uint8_t

    OSCMessage msg("/b");
    msg.add(blobPacket, BLOB_PACKET_SIZE);
    OSCbundle.add(msg);
  }
  SLIPSerial.beginPacket();     //
  OSCbundle.send(SLIPSerial);   // Send the bytes to the SLIP stream
  SLIPSerial.endPacket();       // Mark the end of the OSC Packet
#endif /*__BLOBS_OSC__*/

#ifdef DEBUG_BLOBS_OSC
  for (blob_t* blob = iterator_start_from_head(&outputBlobs); blob != NULL; blob = iterator_next(blob)) {
    Serial.print (blob->UID);        // uint8_t unique session ID
    Serial.print(" ");
    Serial.print (blob->alive);      // uint8_t
    Serial.print(" ");
    Serial.print (blob->centroid.X); // uint8_t
    Serial.print(" ");
    Serial.print (blob->centroid.Y); // uint8_t
    Serial.print(" ");
    Serial.print (blob->box.W);      // uint8_t
    Serial.print(" ");
    Serial.print (blob->box.H);      // uint8_t
    Serial.print(" ");
    Serial.print (blob->box.D);      // uint8_t
    Serial.println();
  }
#endif /*__DEBUG_BLOBS_OSC__*/

#ifdef DEBUG_ADC
  for (uint8_t col = 0; col < COLS; col++) {
    for (uint8_t row = 0; row < ROWS; row++) {
      uint8_t index = col * COLS + row;          // Compute 1D array index
      Serial.print("\t");
      Serial.print(frameValues[index]);
    }
    Serial.println();
  }
  Serial.println();
#endif /*__DEBUG_ADC__*/

#ifdef DEBUG_INTERP
  for (uint8_t col = 0; col < NEW_COLS; col++) {
    for (uint8_t row = 0; row < NEW_ROWS; row++) {
      uint16_t index = col * NEW_COLS + row;          // Compute 1D array index
      Serial.print(" ");
      Serial.print(bilinIntOutput[index]);
    }
    Serial.println();
  }
  Serial.println();
#endif /*__DEBUG_INTERP__*/

#ifdef DEBUG_FPS
  if (FPS_timer >= 1000) {
    Serial.printf(F("\nFPS: %d"), fps);
    FPS_timer = 0;
    fps = 0;
  }
  fps++;
#endif /*__E256_FPS__*/
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
