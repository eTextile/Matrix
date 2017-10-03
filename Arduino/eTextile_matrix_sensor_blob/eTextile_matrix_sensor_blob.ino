// eTextile matrix sensor shield V2.0 (E-256)

#include <PacketSerial.h> // https://github.com/bakercp/PacketSerial
#include "eTextile_matrix_sensor_blob.h"
#include "blob.h"

boolean DEBUG_INTERP = false;
boolean DEBUG_BLOB = false;

PacketSerial serial;

heap_t *heap;

void setup() {

  // serial.setPacketHandler(&onPacket); // We must specify a packet handler method so that
  // serial.begin(BAUD_RATE);            // Start the serial module
  Serial.begin(BAUD_RATE);               // Arduino serial standard library

  analogReadRes(10);                     // Set the ADC converteur resolution to 10 bit
  pinMode(BUILTIN_LED, OUTPUT);          // Set BUILTIN_LED pin as output
  pinMode(BUTTON_PIN, INPUT_PULLUP);     // Set button pin as input and activate the input pullup resistor
  attachInterrupt(BUTTON_PIN, pushButton, RISING); // Attach interrrupt on button PIN

  for (int i = 0; i < ROWS; i++) {
    pinMode(rowPins[ROWS], INPUT);        // Set rows pins in high-impedance state
  }

  heap = (heap_t*)malloc(sizeof(heap_t));
  memset(heap, 0, sizeof(heap_t));

  void *region = malloc((byte)HEAP_INIT_SIZE);
  memset(region, 0, (byte)HEAP_INIT_SIZE); // Added (byte) cast

  for (int i = 0; i < BIN_COUNT; i++) {
    heap->bins[i] = (bin_t*)malloc(sizeof(bin_t));
    memset(heap->bins[i], 0, sizeof(bin_t));
  }
  init_heap(heap, (uint)region);

  S.numCols = COLS;
  S.numRows = ROWS;
  S.pData = frameValues; // https://en.wikipedia.org/wiki/Q_(number_format)

  frame.w = COLS * SCALE;
  frame.h = ROWS * SCALE;
  frame.data = bilinIntOutput;

  roi.x = 0;
  roi.y = 0;
  roi.w = NEW_COLS;
  roi.h = NEW_ROWS;

  bootBlink(9);
}

void loop() {

  uint16_t sensorID = 0;
  for (uint8_t row = 0; row < ROWS; row++) {
    pinMode(rowPins[row], OUTPUT);  // Set row pin as output
    digitalWrite(rowPins[row], HIGH);
    for (uint8_t col = 0; col < COLS; col++) {
      uint16_t rowValue = analogRead(columnPins[col]); // Read the sensor value
      int16_t value = rowValue - minVals[sensorID]; // Aplay the calibration ofset
      if (value > 0) {
        frameValues[sensorID] = (float32_t)value;
      } else {
        frameValues[sensorID] = 0;
      }
      sensorID++;
    }
    pinMode(rowPins[row], INPUT); // Set row pin in high-impedance state
    // digitalWrite(rowPins[row], LOW); // Set row pin to GND (TO TEST!)
  }

  sensorID = 0;
  for (uint8_t row = 0; row < NEW_ROWS; row++) {
    for (uint8_t col = 0; col < NEW_COLS; col++) {
      float32_t rowPos = (float32_t)row / SCALE;
      float32_t colPos = (float32_t)col / SCALE;
      bilinIntOutput[sensorID] = (uint8_t) arm_bilinear_interp_f32(&S, rowPos, colPos);
      if (DEBUG_INTERP) Serial.print(" "), Serial.print(bilinIntOutput[sensorID]);
      sensorID++;
    }
    if (DEBUG_INTERP) Serial.println();
  }
  if (DEBUG_INTERP) Serial.println();

  find_blobs(
    heap,           // heap_t
    &blobOut,       // list_t
    &frame,         // image_t
    &roi,           // rectangle_t
    THRESHOLD,      // unsigned int
    MIN_BLOB_SIZE,  // unsigned int
    MIN_BLOB_PIX,   // unsigned int
    true,           // boolean merge
    0               // int margin
  );

  for (list_lnk_t *it = iterator_start_from_head(&blobOut); it; it = iterator_next(it)) {
    find_blobs_list_lnk_data_t lnk_data;
    iterator_get(&blobOut, it, &lnk_data);
    if (DEBUG_BLOB) Serial.print(lnk_data.centroid.x);
    if (DEBUG_BLOB) Serial.print("_");
    if (DEBUG_BLOB) Serial.print(lnk_data.centroid.y);
    if (DEBUG_BLOB) Serial.println();
  }

  // The update() method attempts to read in
  // any incoming serial data and emits packets via
  // the user's onPacket(const uint8_t* buffer, size_t size)
  // method registered with the setPacketHandler() method.
  // The update() method should be called at the end of the loop().
  // serial.update();

}

/////////// Calibrate the 16x16 sensor matrix by doing average
void calibrate(volatile uint16_t *sumArray) {

  for (uint8_t i = 0; i < CALIBRATION_CYCLES; i++) {
    uint8_t pos = 0;
    for (uint8_t row = 0; row < ROWS; row++) {
      pinMode(rowPins[row], OUTPUT);  // Set row pin as output
      digitalWrite(rowPins[row], HIGH);
      for (uint8_t col = 0; col < COLS; col++) {
        uint16_t val = analogRead(columnPins[col]); // Read the sensor value
        sumArray[pos] += (uint16_t) val / CALIBRATION_CYCLES;
        pos++;
      }
      pinMode(rowPins[row], INPUT); // Set row pin in high-impedance state
      // digitalWrite(rowPins[row], LOW); // Set row pin to GND (TO TEST!)
    }
  }
}

/////////// Blink
void bootBlink(uint8_t flash) {
  for (uint8_t i = 0; i < flash; i++) {
    digitalWrite(BUILTIN_LED, HIGH);
    delay(50);
    digitalWrite(BUILTIN_LED, LOW);
    delay(50);
  }
}

/////////// Called with interrupt triggered with push button attached to I/O pin 32
void pushButton() {
  cli();
  calibrate(minVals);
  bootBlink(3);
  sei();
}

// This is our packet callback.
// The buffer is delivered already decoded.
void onPacket(const uint8_t *buffer, size_t size) {
  // The send() method will encode the buffer
  // as a packet, set packet markers, etc.
  // serial.send(myPacket, ROW_FRAME);
}
