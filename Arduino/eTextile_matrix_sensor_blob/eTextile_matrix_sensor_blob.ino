// eTextile matrix sensor shield V2.0 (E-256)

#include <PacketSerial.h> // https://github.com/bakercp/PacketSerial
#include "blob.h"
#include "eTextile_matrix_sensor_blob.h"

// PacketSerial serial;

void setup() {

  // serial.setPacketHandler(&onPacket); // We must specify a packet handler method so that
  // serial.begin(BAUD_RATE);            // Start the serial module
  Serial.begin(BAUD_RATE);               // Arduino serial standard library

  analogReadRes(10);                     // Set the ADC converteur resolution to 10 bit
  pinMode(BUILTIN_LED, OUTPUT);          // Set BUILTIN_LED pin as output
  pinMode(BUTTON_PIN, INPUT_PULLUP);     // Set button pin as input and activate the input pullup resistor
  attachInterrupt(BUTTON_PIN, pushButton, RISING); // Attach interrrupt on button PIN

  minValsPtr = &minVals[0]; // Initialize minVals array pointer

  interpolate.numCols = COLS;
  interpolate.numRows = ROWS;
  interpolate.pData = &frameValues[0];

  inputFramePtr = &inputFrame;

  inputFramePtr->w = NEW_COLS;
  inputFramePtr->h = NEW_ROWS;
  inputFramePtr->dataPtr = &bilinIntOutput[0];

  bitmapPtr = &bitmap[0]; // Initialize bitmap array pointer
  memset(bitmapPtr, 255, NEW_FRAME * sizeof(char)); // Set bitmap array datas to 0

  tmpNodePushPtr = &tmpNodePush; // Initialize tmpNode node_t pointer
  tmpNodePushPtr = (node_t*) malloc(sizeof(node_t) + NEW_FRAME * sizeof(char));
  memset(tmpNodePushPtr, NULL, sizeof(node_t) + NEW_FRAME * sizeof(char)); // Initialize tmpNode size

  outputBlobsPtr = &outputBlobs; // Initialize outputBlobs list_t pointer
  tmpNodePushPtr = (node_t*) malloc(sizeof(list_t) + MAX_BLOBS * sizeof(node_t));
  memset(outputBlobsPtr, NULL, sizeof(list_t) + MAX_BLOBS * sizeof(node_t)); // Init outputBlobs size

  calibrate(minValsPtr, CYCLES);
  bootBlink(9);
}

void loop() {
  /*
    if ((millis() - lastFarme) >= 1000) {
      Serial.printf("\nFPS: %d", fps);  // I see 16 FPS!
      lastFarme = millis();
      fps = 0;
    }
  */
  sensorID = 0;
  for (uint8_t row = 0; row < ROWS; row++) {
    pinMode(rowPins[row], OUTPUT);  // Set row pin as output
    digitalWrite(rowPins[row], HIGH);
    for (uint8_t col = 0; col < COLS; col++) {
      uint16_t rowValue = analogRead(columnPins[col]); // Read the sensor value
      int16_t value = rowValue - minVals[sensorID]; // Aplay the calibration ofset
      if (value > 0) {
        frameValues[sensorID] = value;
      } else {
        frameValues[sensorID] = 0;
      }
      sensorID++;
    }
    pinMode(rowPins[row], INPUT); // Set row pin in high-impedance state
    // digitalWrite(rowPins[row], LOW); // Set row pin to GND (TO TEST!)
  }
  sensorID = 0;

  for (float row = 0; row < NEW_ROWS; row++) {
    for (float col = 0; col < NEW_COLS; col++) {
      float rowPos = row / SCALE;
      float colPos = col / SCALE;
      bilinIntOutput[sensorID] = (uint8_t) arm_bilinear_interp_f32(&interpolate, rowPos, colPos);
      if (DEBUG_INTERP) Serial.printf(" %d", bilinIntOutput[sensorID]);
      sensorID++;
    }
    if (DEBUG_INTERP) Serial.println();
  }
  if (DEBUG_INTERP) Serial.println();

  find_blobs(
    inputFramePtr,    // image_t
    outputBlobsPtr,   // list_t
    tmpNodePushPtr,   // node_t
    bitmapPtr,        // Array of char
    NEW_ROWS,         // const int
    NEW_COLS,         // const int
    THRESHOLD,        // const int
    MIN_BLOB_SIZE,    // const int
    MIN_BLOB_PIX,     // const int
    true              // boolean merge
  );

  for (node_t* it = iterator_start_from_head(outputBlobsPtr); it; it = iterator_next(it)) {
    blob_t lnk_data;
    iterator_get(outputBlobsPtr, it, &lnk_data);
    if (DEBUG_OUTPUT) Serial.printf("\nID: %d posX: %d posY: %d\t", lnk_data.code, lnk_data.centroid.x, lnk_data.centroid.y);
  }

  // The update() method attempts to read in
  // any incoming serial data and emits packets via
  // the user's onPacket(const uint8_t* buffer, size_t size)
  // method registered with the setPacketHandler() method.
  // The update() method should be called at the end of the loop().
  // serial.update();

  // fps++;
}

/////////// Calibrate the 16x16 sensor matrix by doing average
void calibrate(uint16_t *sumArray, const uint8_t cycles) {

  uint8_t pos;
  memset(sumArray, 255, ROW_FRAME * sizeof(uint16_t)); // Set minVals array datas to 0

  for (uint8_t i = 0; i < cycles; i++) {
    pos = 0;
    for (uint8_t row = 0; row < ROWS; row++) {
      pinMode(rowPins[row], OUTPUT);  // Set row pin as output
      digitalWrite(rowPins[row], HIGH);
      for (uint8_t col = 0; col < COLS; col++) {
        uint16_t val = analogRead(columnPins[col]); // Read the sensor value
        if (val < sumArray[pos]){
          sumArray[pos] = val;
        }
        pos++;
      }
      pinMode(rowPins[row], INPUT); // Set row pin in high-impedance state
      // digitalWrite(rowPins[row], LOW); // Set row pin to GND (TO TEST!)
    }
  }
  bootBlink(3);
  if (DEBUG_OUTPUT) Serial.printf("\n>>>> Calibrated!");
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
  calibrate(minValsPtr, CYCLES);
  sei();
}

// This is our packet callback.
// The buffer is delivered already decoded.
void onPacket(const uint8_t* buffer, size_t size) {
  // The send() method will encode the buffer
  // as a packet, set packet markers, etc.
  // serial.send(myPacket, ROW_FRAME);
}

