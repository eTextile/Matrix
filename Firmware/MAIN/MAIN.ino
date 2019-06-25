/*
  ** E256 Firmware v1.1 ** (DO NOT WORK!)
  This file is part of the eTextile-matrix-sensor project - http://matrix.eTextile.org
  Copyright (c) 2014-2019 Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

// FPS with CPU speed to 120 MHz (Overclock)
// Optimize Fastest with LTO (Link Time Optimizations)
//   523 FPS : ADC_INPUT
//    .. FPS : ADC_INPUT / BLOB_TRACKING
//    .. FPS : ADC_INPUT / BILINEAR_INTERPOLATION
//    39 FPS : ADC_INPUT / BILINEAR_INTERPOLATION / BLOB_TRACKING

#include <SPI.h>                    // https://www.pjrc.com/teensy/td_libs_SPI.html
#include <ADC.h>                    // https://github.com/pedvide/ADC
#include <OSCBoards.h>              // https://github.com/CNMAT/OSC
#include <OSCMessage.h>             // https://github.com/CNMAT/OSC
#include <OSCBundle.h>              // https://github.com/CNMAT/OSC
#include <SLIPEncodedUSBSerial.h>   // https://github.com/CNMAT/OSC

#include "config.h"
#include "interp.h"
#include "blob.h"

SPISettings settings(16000000, MSBFIRST, SPI_MODE0);

ADC *adc = new ADC();     // ADC object
ADC::Sync_result result;  // ADC_0 & ADC_1

// Array to store all parameters used to configure the two 8:1 analog multiplexeurs
// Eatch byte |ENA|A|B|C|ENA|A|B|C|
const byte setDualRows[RAW_ROWS] = {
  0x55, 0x77, 0x66, 0x44, 0x22, 0x11, 0x00, 0x33
};

uint8_t minValsArray[RAW_FRAME] = {0};            // 1D Array to store smallest values
uint8_t frameArray[RAW_FRAME] = {0};              // 1D Array to store ofseted analog input values

uint8_t bitmapArray[NEW_FRAME] = {0};             // 1D Array to store binary values (16*16) array containing (64*64) values
uint8_t interpFrameArray[NEW_FRAME] = {0};        // 1D Array to store bilinear interpolated values

xylr_t lifoArray[LIFO_MAX_NODES] = {0};           // 1D Array to store lifo nodes
blob_t blobArray[MAX_NODES] = {0};                // 1D Array to store blobs

uint8_t blobPacket[BLOB_PACKET_SIZE] = {0};       // 1D Array to store blobs in OSC format

image_t  inputFrame;         // Input frame values
interp_t interp;             //
image_t  interpolatedFrame;  //
image_t  bitmap;             // Used by Scanline Flood Fill algorithm / SFF
lifo_t   lifo_stack;         // Lifo free nodes stack
lifo_t   lifo;               // Lifo stack
llist_t  blobs_stack;        // Blobs free nodes linked list
llist_t  blobs;              // Intermediate blobs linked list
llist_t  outputBlobs;        // Output blobs linked list

SLIPEncodedUSBSerial SLIPSerial(thisBoardsSerialUSB);

uint8_t threshold = 25;
uint8_t calibration_cycles = 20;
boolean calibrate = true;
boolean scanning = true;

void setup() {

  //pinMode(LED_BUILTIN, OUTPUT);   // FIXME - BUILTIN_LED is used for SPI hardware
  //digitalWrite(LED_BUILTIN, LOW); // FIXME - BUILTIN_LED is used for SPI hardware

#ifdef DEBUG_ADC
#ifdef DEBUG_INTERP
#ifdef DEBUG_BLOBS_OSC
#ifdef DEBUG_SFF_BITMAP
#ifdef DEBUG_FPS
  Serial.begin(BAUD_RATE);       // Arduino serial library ** 230400 **
  while (!Serial.dtr());         // Wait for user to start the serial monitor
  //Serial.println("SERIAL_SETUP");
  //delay(100);
#else
  SLIPSerial.begin(BAUD_RATE);   // Arduino serial library ** 230400 ** extended with SLIP encoding
#endif /*__DEBUG_ADC__*/
#endif /*__DEBUG_INTERP__*/
#endif /*__DEBUG_BLOBS_OSC__*/
#endif /*__DEBUG_SFF_BITMAP__*/
#endif /*__DEBUG_FPS__*/

  //pinMode(BUTTON_PIN, INPUT_PULLUP);          // Set button pin as input and activate the input pullup resistor // FIXME - NO BUTTON_PIN ON the E256!
  //attachInterrupt(BUTTON_PIN, calib, RISING); // Attach interrrupt on button PIN // FIXME - NO BUTTON_PIN ON the E256

  SPI_SETUP();
  //Serial.println("SPI_SETUP");
  //delay(100);
  ADC_SETUP();
  //Serial.println("ADC_SETUP");
  //delay(100);
  INTERP_SETUP(
    &inputFrame,          // image_t*
    &frameArray[0],       // uint8_t*
    &interpolatedFrame,   // image_t*
    &interpFrameArray[0], // uint8_t*
    &interp               // interp_t*
  );
  //Serial.println("INTERP_SETUP");
  //delay(100);
  BLOB_SETUP(
    &inputFrame,          // image_t*
    &bitmap,              // image_t*
    &bitmapArray[0],      // uint8_t*
    &lifo,                // lifo_t*
    &lifo_stack,          // lifo_t*
    &lifoArray[0],        // xylr_t*
    &blobs,               // list_t*
    &blobs_stack,         // list_t*
    &blobArray[0],        // blob_t*
    &outputBlobs          // list_t*
  );
  //Serial.println("BLOB_SETUP");
  //delay(100);
}

//////////////////// LOOP
void loop() {

#ifdef DEBUG_ADC
  matrix_scan(&frameArray[0]);

  for (uint8_t col = 0; col < RAW_COLS; col++) {
    for (uint8_t row = 0; row < RAW_ROWS; row++) {
      uint8_t index = col * RAW_COLS + row;          // Compute 1D array index
      Serial.printf("\t%d", frameArray[index]);
    }
    Serial.println();
  }
  Serial.println();
  delay(50);
#endif /*__DEBUG_ADC__*/

#ifdef DEBUG_INTERP
  if (calibrate) matrix_calibrate(&minValsArray[0]);
  matrix_scan(&frameArray[0]);
  matrix_interp(&interpolatedFrame, &inputFrame, &interp);

  for (uint8_t col = 0; col < NEW_COLS; col++) {
    for (uint8_t row = 0; row < NEW_ROWS; row++) {
      uint16_t index = col * NEW_COLS + row;          // Compute 1D array index
      Serial.printf("-%d", interpFrameArray[index]);
    }
    Serial.println();
  }
  Serial.println();
  delay(50);
#endif /*__DEBUG_INTERP__*/

#ifdef DEBUG_BITMAP
  if (calibrate) matrix_calibrate(&minValsArray[0]);
  matrix_scan(&frameArray[0]);
  matrix_interp(&interpolatedFrame, &inputFrame, &interp);
  matrix_scan_blobs();

  for (uint8_t posY = 0; posY < NEW_COLS; posY++) {
    uint8_t* bmp_row = COMPUTE_BINARY_IMAGE_ROW_PTR (&bitmap, posY);
    for (uint8_t posX = 0; posX < NEW_ROWS; posX++) {
      Serial.print(IMAGE_GET_BINARY_PIXEL_FAST(bmp_row, posX));
    }
    Serial.println();
  }
  Serial.println();
  delay(50);
#endif /*__DEBUG_BITMAP__*/

#ifdef DEBUG_BLOBS_OSC
  if (calibrate) matrix_calibrate(&minValsArray[0]);
  matrix_scan(&frameArray[0]);
  matrix_interp(&interpolatedFrame, &inputFrame, &interp);
  matrix_scan_blobs();

  /*
    for (blob_t* blob = ITERATOR_START_FROM_HEAD(&outputBlobs); blob != NULL; blob = ITERATOR_NEXT(blob)) {
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
  */
#endif /*__DEBUG_BLOBS_OSC__*/

#ifdef BLOBS_OSC

  OSCMessage OSCmsg;
  int size;
  while (!SLIPSerial.endofPacket()) {
    if ((size = SLIPSerial.available()) > 0) {
      while (size--)
        OSCmsg.fill(SLIPSerial.read());
    }
  }
  if (!OSCmsg.hasError()  && !scanning) {
    OSCmsg.dispatch("/c", matrix_calibration_set);
    OSCmsg.dispatch("/t", matrix_threshold_set);
    OSCmsg.dispatch("/r", matrix_raw_data_get);
    OSCmsg.dispatch("/i", matrix_interp_data_get);
    OSCmsg.dispatch("/x", matrix_interp_data_bin_get);
    OSCmsg.dispatch("/b", matrix_blobs_get);
  }

  if (calibrate) matrix_calibrate(&minValsArray[0]);
  if (scanning) {
    matrix_scan(&frameArray[0]);
    matrix_interp(&interpolatedFrame, &inputFrame, &interp);
    matrix_scan_blobs();
    scanning = false;
  }

#endif /*__BLOBS_OSC__*/
}

//////////////////////////////////////////////////// FONCTIONS

void SPI_SETUP(void) {

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
}

void ADC_SETUP(void) {

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
}

// Columns are digital OUTPUT PINS - We supply them one by one sequentially
// Rows are analog INPUT PINS - We sens them two by two
void matrix_scan(uint8_t* outputFrame) {

  uint32_t setCols = 0x10000;

  for (uint8_t col = 0; col < RAW_COLS; col++) {        // 0 to 15
    setCols = setCols >> 1;
    for (uint8_t row = 0; row < DUAL_ROWS; row++) { // 0 to 7

      digitalWriteFast(E256_SS_PIN, LOW);   // Set latchPin LOW
      SPI.transfer(setCols & 0xFF);         // Shift out the LSB byte to set up the OUTPUT shift register
      SPI.transfer((setCols >> 8) & 0xFF);  // Shift out the MSB byte to set up the OUTPUT shift register
      SPI.transfer(setDualRows[row]);       // Shift out one byte that setup the two 8:1 analog multiplexers
      digitalWriteFast(E256_SS_PIN, HIGH);  // Set latchPin HIGH
      //delayMicroseconds(5);               // TODO: see switching time of the 74HC4051BQ multiplexeur

      uint8_t rowIndexA = row * RAW_COLS + col; // Row IndexA computation
      uint8_t rowIndexB = rowIndexA + 128;  // Row IndexB computation (ROW_FRAME/2 == 128)

      result = adc->analogSynchronizedRead(ADC0_PIN, ADC1_PIN);

      int valA = result.result_adc0 - minValsArray[rowIndexA];
      valA > 0 ? outputFrame[rowIndexA] = (uint8_t)valA : outputFrame[rowIndexA] = 0;
      int valB = result.result_adc1 - minValsArray[rowIndexB];
      valB > 0 ? outputFrame[rowIndexB] = (uint8_t)valB : outputFrame[rowIndexB] = 0;
    }
  }
}

void matrix_calibration_set(OSCMessage & msg) {
  calibration_cycles = msg.getInt(0) & 0xFF;   // Get the first uint8_t in an int32_t
  calibrate = true;
}

// Columns are digital OUTPUT PINS - We supply them one by one sequentially
// Rows are analog INPUT PINS - We sens them two by two
void matrix_calibrate(uint8_t* outputFrame) {

  for (uint8_t i = 0; i < calibration_cycles; i++) {

    uint32_t setCols = 0x10000;

    for (uint8_t col = 0; col < RAW_COLS; col++) {
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

        uint8_t rowIndexA = row * RAW_COLS + col;    // Row IndexA computation
        uint8_t rowIndexB = rowIndexA + 128;     // Row IndexB computation (ROW_FRAME/2 == 128)

        if (ADC0_val > outputFrame[rowIndexA]) outputFrame[rowIndexA] = ADC0_val;
        if (ADC1_val > outputFrame[rowIndexB]) outputFrame[rowIndexB] = ADC1_val;
      }
    }
  }
  calibrate = false;
}

void matrix_scan_blobs(void) {

  find_blobs(
    threshold,          // uint8_t
    &interpolatedFrame, // image_t (uint8_t array[NEW_FRAME] - 64*64 1D array)
    &bitmap,            // image_t (uint8_t array[NEW_FRAME] - 64*64 1D array)
    &lifo_stack,        // lifo_t
    &lifo,              // lifo_t
    &blobs_stack,       // list_t
    &blobs,             // list_t
    &outputBlobs        // list_t
  );
}

// Set the threshold
void matrix_threshold_set(OSCMessage & msg) {
  threshold = msg.getInt(0) & 0xFF; // Get the first uint8_t of the int32_t
}

// Send raw frame values in SLIP-OSC formmat
void matrix_raw_data_get(OSCMessage & msg) {

  matrix_scan(&frameArray[0]);
  OSCMessage m("/r");
  m.add(frameArray, RAW_FRAME);
  SLIPSerial.beginPacket();
  m.send(SLIPSerial);        // Send the bytes to the SLIP stream
  SLIPSerial.endPacket();    // Mark the end of the OSC Packet
}

// Send interpolated frame values in SLIP-OSC formmat
void matrix_interp_data_get(OSCMessage & msg) {

  matrix_scan(&frameArray[0]);
  matrix_interp(&interpolatedFrame, &inputFrame, &interp);
  OSCMessage m("/i");
  m.add(interpFrameArray, NEW_FRAME);
  SLIPSerial.beginPacket();
  m.send(SLIPSerial);        // Send the bytes to the SLIP stream
  SLIPSerial.endPacket();    // Mark the end of the OSC Packet
}

// Send interpolated frame values in SLIP-OSC formmat
void matrix_interp_data_bin_get(OSCMessage & msg) {

  matrix_scan(&frameArray[0]);
  matrix_interp(&interpolatedFrame, &inputFrame, &interp);
  OSCMessage m("/i");
  m.add(&bitmapArray[0], RAW_FRAME);
  SLIPSerial.beginPacket();
  m.send(SLIPSerial);        // Send the bytes to the SLIP stream
  SLIPSerial.endPacket();    // Mark the end of the OSC Packet
}

void matrix_blobs_get(OSCMessage & msg) {

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
  scanning = true;
}
