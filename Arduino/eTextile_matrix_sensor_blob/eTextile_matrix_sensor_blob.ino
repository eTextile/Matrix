// eTextile matrix sensor shield V2.0 (E-256)

#include "eTextile_matrix_sensor_blob.h"

////////////////////////////////////// SETUP

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

  S.numCols = COLS;
  S.numRows = ROWS;
  S.pData = frameValues;

  frame.w = COLS * SCALE;
  frame.h = ROWS * SCALE;
  frame.data = bilinIntOutput;

  Roi.x = 0;
  Roi.y = 0;
  Roi.w = COLS * SCALE;
  Roi.h = ROWS * SCALE;

  bootBlink(9);

  heap_t *heap = malloc(sizeof(heap_t));
  memset(heap, 0, sizeof(heap_t));

  void *region = malloc(HEAP_INIT_SIZE);
  memset(region, 0, HEAP_INIT_SIZE);

  for (int i = 0; i < BIN_COUNT; i++) {
    heap->bins[i] = malloc(sizeof(bin_t));
    memset(heap->bins[i], 0, sizeof(bin_t));
  }

  init_heap(heap, (uint) region);
  
}

/////////////////////////////////// LOOP

void loop() {

  for (uint8_t row = 0; row < ROWS; row++) {
    pinMode(rowPins[row], OUTPUT);  // Set row pin as output
    digitalWrite(rowPins[row], HIGH);
    for (uint8_t column = 0; column < COLS; column++) {
      uint16_t rowValue = analogRead(columnPins[column]); // Read the sensor value
      uint8_t sensorID = row * ROWS + column; // Calculate the index of the unidimensional array
      int16_t value = rowValue - minVals[sensorID]; // Aplay the calibration ofset
      if (value > 0) {
        frameValues[sensorID] = value;
      } else {
        frameValues[sensorID] = 0;
      }
    }
    pinMode(rowPins[row], INPUT); // Set row pin in high-impedance state
    // digitalWrite(rowPins[row], LOW); // Set row pin to GND (TO TEST!)
  }

  int pos = 0;
  for (q31_t posY = 0; posY < COLS; posY += INC) {
    for (q31_t posX = 0; posX < ROWS; posX += INC) {
      #ifdef CORE_TEENSY
      bilinIntOutput[pos] = arm_bilinear_interp_q7(&S, posX, posY);
      #endif // __CORE_TEENSY__
      pos++;
    }
  }

  find_blobs(
    &BlobOut,       // list_t
    &frame,         // image_t
    &Roi,           // rectangle_t
    THRESHOLD,      // unsigned int
    MIN_BLOB_SIZE,  // unsigned int
    MIN_BLOB_PIX,   // unsigned int
    true,           // bool merge
    0               // int margin
  );

  Serial.println("Frame compleat!");

  for (list_lnk_t *it = iterator_start_from_head(&BlobOut); it; it = iterator_next(it)) {
    find_blobs_list_lnk_data_t lnk_data;
    iterator_get(&BlobOut, it, &lnk_data);
    Serial.print(lnk_data.centroid.x);
    Serial.print("_");
    Serial.print(lnk_data.centroid.y);
    Serial.print("\t");
  }

  // The update() method attempts to read in
  // any incoming serial data and emits packets via
  // the user's onPacket(const uint8_t* buffer, size_t size)
  // method registered with the setPacketHandler() method.
  // The update() method should be called at the end of the loop().
  // serial.update();

}

// Calibrate the sensor matrix by doing average
void _calibrate(uint16_t *sumArray, uint8_t frames) {

  for (uint8_t i = 0; i < frames; i++) {
    for (uint8_t row = 0; row < ROWS; row++) {
      pinMode(rowPins[row], OUTPUT);  // Set row pin as output
      digitalWrite(rowPins[row], HIGH);
      for (uint8_t column = 0; column < COLS; column++) {
        uint16_t rowValue = analogRead(columnPins[column]); // Read the sensor value
        uint8_t sensorID = row * ROWS + column; // Calculate the index of the unidimensional array
        sumArray[sensorID] += rowValue;
      }
      pinMode(rowPins[row], INPUT); // Set row pin in high-impedance state
      // digitalWrite(rowPins[row], LOW); // Set row pin to GND (TO TEST!)
    }
  }
  for (uint8_t i = 0; i < ROW_FRAME; i++) {
    sumArray[i] = sumArray[i] / frames;
  }
}

// This is our packet callback.
// The buffer is delivered already decoded.
void onPacket(const uint8_t *buffer, size_t size) {
  // The send() method will encode the buffer
  // as a packet, set packet markers, etc.
  // serial.send(myPacket, ROW_FRAME);
}

/////////// Called with interrupt triggered with push button attached to I/O pin 32
void pushButton() {
  cli();
  _calibrate(&minVals, CALIBRATION_CYCLES);
  bootBlink(3);
  sei();
}

/////////// Blink
void bootBlink(int flash) {
  for (int i = 0; i < flash; i++) {
    digitalWrite(BUILTIN_LED, HIGH);
    delay(50);
    digitalWrite(BUILTIN_LED, LOW);
    delay(50);
  }
}
