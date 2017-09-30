// eTextile matrix sensor shield V2.0 (E-256)

#include <PacketSerial.h> // https://github.com/bakercp/PacketSerial
#include "eTextile_matrix_sensor_blob.h"
#include "blob.h"

PacketSerial serial;

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
  S.pData = frameValues; // https://en.wikipedia.org/wiki/Q_(number_format)

  frame.w = COLS * SCALE;
  frame.h = ROWS * SCALE;
  frame.data = bilinIntOutput;

  Roi.x = 0;
  Roi.y = 0;
  Roi.w = COLS * SCALE;
  Roi.h = ROWS * SCALE;

  bootBlink(9);

}

void loop() {

  Serial.println("Start scanning!");

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

  Serial.println("Starting interpolation!");
  sensorID = 0;
  for (uint8_t row = 0; row < NEW_ROWS; row++) {
    for (uint8_t col = 0; col < NEW_COLS; col++) {
      float32_t rowPos = (float32_t)row / SCALE;
      float32_t colPos = (float32_t)col / SCALE;
      bilinIntOutput[sensorID] = (uint16_t) arm_bilinear_interp_f32(&S, rowPos, colPos);
      // Serial.print(" "), Serial.print((float)bilinIntOutput[sensorID]);
      sensorID++;
    }
    // Serial.println();
  }
  // Serial.println();

  // Serial.println("Starting blob!");
  find_blobs(
    &BlobOut,       // list_t
    &frame,         // image_t
    &Roi,           // rectangle_t
    THRESHOLD,      // unsigned int
    MIN_BLOB_SIZE,  // unsigned int
    MIN_BLOB_PIX,   // unsigned int
    true,           // boolean merge
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

/////////// Calibrate the 16x16 sensor matrix by doing average
void _calibrate(volatile uint16_t *sumArray) {

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
  _calibrate(minVals);
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
