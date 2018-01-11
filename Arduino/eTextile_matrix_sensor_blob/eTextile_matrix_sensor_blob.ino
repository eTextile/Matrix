// eTextile matrix sensor
// shield V2.0 (E-256) solution that enable
// Embedded blob detection
// See http://eTextile.org

#include <PacketSerial.h> // https://github.com/bakercp/PacketSerial
#include "collections.h"
#include "eTextile_matrix_sensor_blob.h"

// PacketSerial serial;

void setup() {

  // serial.setPacketHandler(&onPacket); // We must specify a packet handler method so that
  // serial.begin(BAUD_RATE);            // Start the serial module
  Serial.begin(BAUD_RATE);               // Arduino serial standard library
  while (!Serial.dtr()) ;                // wait for user to start the serial monitor

  analogReadRes(10);                     // Set the ADC converteur resolution to 10 bit
  pinMode(BUILTIN_LED, OUTPUT);          // Set BUILTIN_LED pin as output
  pinMode(BUTTON_PIN, INPUT_PULLUP);     // Set button pin as input and activate the input pullup resistor
  attachInterrupt(BUTTON_PIN, pushButton, RISING); // Attach interrrupt on button PIN

  interpolate.numCols = COLS;
  interpolate.numRows = ROWS;
  interpolate.pData = &frameValues[0];

  inputFrame.w = NEW_COLS;
  inputFrame.h = NEW_ROWS;
  inputFrame.dataPtr = &bilinIntOutput[0];

  memset(&bitmap[0], 0, NEW_FRAME * sizeof(char)); // Set all values to 0

  lifo_alloc_all(&lifo, &cclArray[0], sizeof(xylf_t));

  list_init(&freeBlobs);
  list_alloc_all(&freeBlobs, &blobsArray[0]);

  list_init(&blobs);
  list_init(&blobsToUpdate);
  list_init(&blobsToAdd);
  list_init(&outputBlobs);
  if (DEBUG_BLOB) Serial.printf(F("\n>>>>>>>> outputBlobs head_ptr: %p"), outputBlobs.head_ptr);

  calibrate(&minVals[0], CYCLES);
  bootBlink(9);

}

void loop() {

  if ((millis() - lastFarme) >= 1000) {
    Serial.printf(F("\nFPS: %d"), fps);  // 22 FPS - with CPU speed to 120 MHz (overclock) - 45ms by frame
    lastFarme = millis();
    fps = 0;
  }

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
    // pinMode(rowPins[row], INPUT); // Set row pin in high-impedance state
    digitalWrite(rowPins[row], LOW); // Set row pin to GND (TO TEST!)
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
    if (DEBUG_INTERP) Serial.printf("\n\n");
  }
  sensorID = 0;

  find_blobs(
    &inputFrame,        // image_t
    &bitmap[0],         // char Array
    NEW_ROWS,           // const int
    NEW_COLS,           // const int
    THRESHOLD,          // const int
    MIN_BLOB_PIX,       // const int
    MAX_BLOB_PIX,       // const int
    &lifo,              // lifo_t
    &freeBlobs,         // list_t
    &blobs,             // list_t
    &blobsToUpdate,     // list_t
    &blobsToAdd,        // list_t
    &outputBlobs        // list_t
  );

  // The update() method attempts to read in
  // any incoming serial data and emits packets via
  // the user's onPacket(const uint8_t* buffer, size_t size)
  // method registered with the setPacketHandler() method.
  // The update() method should be called at the end of the loop().
  // serial.update();

  fps++;
}

/////////// Calibrate the 16x16 sensor matrix by doing average
void calibrate(uint16_t *arrayMax, const uint8_t cycles) {

  uint8_t pos;
  memset(arrayMax, 0, ROW_FRAME * sizeof(uint16_t)); // Set minVals array datas to 0

  for (uint8_t i = 0; i < cycles; i++) {
    pos = 0;
    for (uint8_t row = 0; row < ROWS; row++) {
      pinMode(rowPins[row], OUTPUT);  // Set row pin as output
      digitalWrite(rowPins[row], HIGH);
      for (uint8_t col = 0; col < COLS; col++) {
        uint16_t val = analogRead(columnPins[col]); // Read the sensor value
        if (val > arrayMax[pos]) {
          arrayMax[pos] = val;
        } else {
          // NA
        }
        pos++;
      }
      pinMode(rowPins[row], INPUT); // Set row pin in high-impedance state
      // digitalWrite(rowPins[row], LOW); // Set row pin to GND (TO TEST!)
    }
  }
  bootBlink(3);
  Serial.printf("\n>>>> Calibrated!");
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
  calibrate(&minVals[0], CYCLES);
  sei();
}

// This is our packet callback.
// The buffer is delivered already decoded.
void onPacket(const uint8_t* buffer, size_t size) {
  // The send() method will encode the buffer
  // as a packet, set packet markers, etc.
  // serial.send(myPacket, ROW_FRAME);
}

