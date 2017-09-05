// E-256 eTextile matrix sensor shield V2.0

#include "eTextile_matrix_sensor_blob.h"

////////////////////////////////////// SETUP
void setup() {

  analogReadRes(10);                     // Set the ADC converteur resolution to 10 bit
  pinMode(LED_BUILTIN, OUTPUT);          // Set rows pins in high-impedance state
  pinMode(BUTTON_PIN, INPUT_PULLUP);     // Set button pins as input and activate the input pullup resistor
  attachInterrupt(BUTTON_PIN, pushButton, RISING); // Attach interrrupt on button PIN

  // serial.setPacketHandler(&onPacket); // We must specify a packet handler method so that
  // serial.begin(BAUD_RATE);  // Start the serial module
  Serial.begin(BAUD_RATE);
  Serial.println("Serial:start");
  for (int i = 0; i < ROWS; i++) {
    pinMode(rowPins[ROWS], INPUT);        // Set rows pins in high-impedance state
  }

  S.numCols = COLS;
  S.numRows = ROWS;
  S.pData = &frameValues[0];

  Image.w = COLS * SCALE;
  Image.h = ROWS * SCALE;
  Image.pixels = &bilinIntOutput[0];
  Image.data = &bilinIntOutput[0]; // Do we nead it?

  Roi.x = 0;
  Roi.y = 0;
  Roi.w = COLS * SCALE;
  Roi.h = ROWS * SCALE;

  Thresholds.LMin = 15;
  Thresholds.LMax = 255;

  bootBlink(9);
}

/////////////////////////////////// LOOP
void loop() {

  if (scan) {
    for (int row = 0; row < ROWS; row++) {
      // Set row pin as output + 3.3V
      pinMode(rowPins[row], OUTPUT);
      digitalWrite(rowPins[row], HIGH);

      for (int column = 0; column < COLS; column++) {
        int rowValue = analogRead(columnPins[column]); // Read the sensor value
        int sensorID = row * ROWS + column; // Calculate the index of the unidimensional array

        if (calibration) {
          Calibrate(sensorID, rowValue, minVals);
        } else {
          uint8_t value = map(rowValue, minVals[sensorID], 1024, 0, 255);
          frameValues[sensorID] = value;
        }
      }
      pinMode(rowPins[row], INPUT); // Set row pin in high-impedance state
      // digitalWrite(rowPins[row], LOW); // Set row pin to GND
    }
    scan = false;
  }
  Serial.println("FRAME");

  int pos = 0;

  for (float32_t posX = 0; posX < ROWS; posX += INC) {
    for (float32_t posY = 0; posY < COLS; posY += INC) {
      // bilinIntOutput[pos++] = arm_bilinear_interp_q7(&S, posX, posY);
      pos++;
    }
  }
  /*
    find_blobs(
      &BlobOut,      // list_t *out
      &Image,        // image_t *ptr
      &Roi,          // rectangle_t *roi
      &Thresholds,   // thresholds_t *thresholds
      false,         // bool invert
      6,             // unsigned int area_threshold
      4,             // unsigned int pixels_threshold
      true ,         // bool merge
      0              // int margin
      // ?,             // bool (*threshold_cb)(void*, find_blobs_list_lnk_data_t*)
      // ?,             // void *threshold_cb_arg
      // ?,             // bool (*merge_cb)(void*, find_blobs_list_lnk_data_t*, find_blobs_list_lnk_data_t*)
      // ?              // void *merge_cb_arg
    );
  */
  Serial.println("FRAME");
  for (list_lnk_t *it = iterator_start_from_head(&BlobOut); it; it = iterator_next(it)) {
    find_blobs_list_lnk_data_t lnk_data;
    iterator_get(&BlobOut, it, &lnk_data);
    Serial.print(lnk_data.centroid.x);
    Serial.print("_");
    Serial.print(lnk_data.centroid.y);
  }

  // The update() method attempts to read in
  // any incoming serial data and emits packets via
  // the user's onPacket(const uint8_t* buffer, size_t size)
  // method registered with the setPacketHandler() method.
  // The update() method should be called at the end of the loop().
  // serial.update();
}

void Calibrate( uint8_t id, int val, int frame[] ) {

  frame[id] += val;
  calibrationCounter++;
  if (calibrationCounter >= CALIBRATION_CYCLES * ROW_FRAME) {
    for (int i = 0; i < ROW_FRAME; i++) {
      frame[i] = frame[i] / CALIBRATION_CYCLES;
    }
    calibrationCounter = 0;
  }
  calibration = false;
}

// This is our packet callback.
// The buffer is delivered already decoded.
void onPacket(const uint8_t* buffer, size_t size) {
  // The send() method will encode the buffer
  // as a packet, set packet markers, etc.
  serial.send(myPacket, ROW_FRAME);
  scan = true;
}

/////////// Called with interrupt triggered with push button attached to I/O pin 32
void pushButton() {
  cli();
  calibrationCounter = 0;
  calibration = true; // Activate the calibration process
  bootBlink(3);
  sei();
}

/////////// Blink fonction
void bootBlink(int flash) {
  for (int i = 0; i < flash; i++) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(50);
    digitalWrite(LED_BUILTIN, LOW);
    delay(50);
  }
}
