// E-256 eTextile matrix sensor shield V2.0

#include "eTextile_matrix_sensor_blob.h"

////////////////////////////////////// SETUP
void setup() {

  // serial.setPacketHandler(&onPacket); // We must specify a packet handler method so that
  // serial.begin(BAUD_RATE);  // Start the serial module
  Serial.begin(BAUD_RATE);
  Serial.println("START_0");

  analogReadRes(10);                     // Set the ADC converteur resolution to 10 bit
  pinMode(BUILTIN_LED, OUTPUT);          // Set rows pins in high-impedance state
  pinMode(BUTTON_PIN, INPUT_PULLUP);     // Set button pins as input and activate the input pullup resistor
  attachInterrupt(BUTTON_PIN, pushButton, RISING); // Attach interrrupt on button PIN
  Serial.println("STEP_1");

  for (int i = 0; i < ROWS; i++) {
    pinMode(rowPins[ROWS], INPUT);        // Set rows pins in high-impedance state
  }

  S.numCols = COLS;
  S.numRows = ROWS;
  S.pData = &frameValues[0];
  Serial.println("STEP_2");

  Image.w = COLS * SCALE;
  Image.h = ROWS * SCALE;
  Image.pixels = &bilinIntOutput[0];
  Image.data = &bilinIntOutput[0]; // Do we nead it?
  Serial.println("STEP_3");

  Roi.x = 0;
  Roi.y = 0;
  Roi.w = COLS * SCALE;
  Roi.h = ROWS * SCALE;
  Serial.println("STEP_4");

  Thresholds.LMin = 15;
  Thresholds.LMax = 255;
  Serial.println("STEP_5");

  bootBlink(9);
  Serial.println("STEP_6");
}

/////////////////////////////////// LOOP
void loop() {
  Serial.println("STEP_7");
  if (scan) {
    Serial.println("STEP_8");
    for (uint8_t row = 0; row < ROWS; row++) {
      // Set row pin as output + 3.3V
      pinMode(rowPins[row], OUTPUT);
      digitalWrite(rowPins[row], HIGH);
      for (uint8_t column = 0; column < COLS; column++) {
        uint16_t rowValue = analogRead(columnPins[column]); // Read the sensor value
        uint8_t sensorID = row * ROWS + column; // Calculate the index of the unidimensional array
        if (calibration) {
          calibrate(&minVals, sensorID, rowValue);
          Serial.println("STEP_9");
        } else {
          uint8_t value = map(rowValue, minVals[sensorID], 1023, 0, 255);
          frameValues[sensorID] = value;
        }
      }
      pinMode(rowPins[row], INPUT); // Set row pin in high-impedance state
      // digitalWrite(rowPins[row], LOW); // Set row pin to GND
    }
    // scan = false;
    int pos = 0;
  }

  for (float posY = 0; posY < COLS; posY += INC) {
    for (float posX = 0; posX < ROWS; posX += INC) {
      // bilinIntOutput[pos] = arm_bilinear_interp_q7(&S, posX, posY);
      pos++;
    }
  }

  Serial.println("STEP_10");
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
  Serial.println("Frame compleat");
  /*
    for (list_lnk_t *it = iterator_start_from_head(&BlobOut); it; it = iterator_next(it)) {
    find_blobs_list_lnk_data_t lnk_data;
    iterator_get(&BlobOut, it, &lnk_data);
    Serial.print(lnk_data.centroid.x);
    Serial.print("_");
    Serial.print(lnk_data.centroid.y);
    }
  */
  // The update() method attempts to read in
  // any incoming serial data and emits packets via
  // the user's onPacket(const uint8_t* buffer, size_t size)
  // method registered with the setPacketHandler() method.
  // The update() method should be called at the end of the loop().
  // serial.update();
}

void calibrate(uint16_t *sumArray, uint16_t id, uint16_t val) {
  static int counter = 0;

  sumArray[id] += val;

  if (counter >= CALIBRATION_CYCLES * ROW_FRAME) {
    for (int i = 0; i < ROW_FRAME; i++) {
      sumArray[i] = sumArray[i] / CALIBRATION_CYCLES;
    }
    counter = 0;
    calibration = false; // Global variable - FIXME!?
  }
  counter++;
}

// This is our packet callback.
// The buffer is delivered already decoded.
void onPacket(const uint8_t *buffer, size_t size) {
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
    digitalWrite(BUILTIN_LED, HIGH);
    delay(50);
    digitalWrite(BUILTIN_LED, LOW);
    delay(50);
  }
}
