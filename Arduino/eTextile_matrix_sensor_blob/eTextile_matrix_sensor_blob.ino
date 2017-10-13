// eTextile matrix sensor shield V2.0 (E-256)

#include <PacketSerial.h> // https://github.com/bakercp/PacketSerial
#include "blob.h"
#include "eTextile_matrix_sensor_blob.h"

long lastFarme = 0;

boolean DEBUG_INTERP = false;
boolean DEBUG_BLOB = false;

PacketSerial serial;

void setup() {

  // serial.setPacketHandler(&onPacket); // We must specify a packet handler method so that
  // serial.begin(BAUD_RATE);            // Start the serial module
  Serial.begin(BAUD_RATE);               // Arduino serial standard library

  analogReadRes(10);                     // Set the ADC converteur resolution to 10 bit
  pinMode(BUILTIN_LED, OUTPUT);          // Set BUILTIN_LED pin as output
  pinMode(BUTTON_PIN, INPUT_PULLUP);     // Set button pin as input and activate the input pullup resistor
  attachInterrupt(BUTTON_PIN, pushButton, RISING); // Attach interrrupt on button PIN

  // bitmapPtr = &bitmap[0]; // Holds the address of the 1st element.

  memset(minVals, 0, ROW_FRAME * sizeof(uint16_t)); // init minVals[] array
  memset(bitmap, 0, NEW_FRAME * sizeof(char)); // Init bitmap[] array
  memset(tmpNode.data, 0, NEW_FRAME * sizeof(char)); // Init tmpNode.data[] array

  S.numCols = COLS;
  S.numRows = ROWS;
  S.pData = frameValues;

  inputFrame.w = NEW_COLS;
  inputFrame.h = NEW_ROWS;
  inputFrame.data = bilinIntOutput;

  calibrate(minVals);
  bootBlink(9);
}

void loop() {

  Serial.printf("NEW FRAME! %d \n", millis() - lastFarme );
  lastFarme = millis();

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
      if (DEBUG_INTERP) Serial.printf(" %d", bilinIntOutput[sensorID]);
      sensorID++;
    }
    if (DEBUG_INTERP) Serial.println();
  }
  if (DEBUG_INTERP) Serial.println();

  find_blobs(
    &inputFrame,    // image_t
    &outputBlobs,   // list_t
    &tmpNode,       // node_t
    &bitmap[0],     // Array of char
    NEW_ROWS,       // const int
    NEW_COLS,       // const int
    THRESHOLD,      // const int
    MIN_BLOB_SIZE,  // const int
    MIN_BLOB_PIX,   // const int
    true,           // boolean merge
    0               // int margin
  );

  for (node_t *it = iterator_start_from_head(&outputBlobs); it; it = iterator_next(it)) {
    blob_t lnk_data;
    iterator_get(&outputBlobs, it, &lnk_data);
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
void calibrate(uint16_t *sumArray) {

  // memset(sumArray, 0, sizeof(sumArray)); TODO: set array to 0

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
