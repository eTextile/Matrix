// E-256 eTextile matrix sensor shield V2.0

#include "E256.h"

image_t image;
list_t blobOut;
rectangle_t roi;

////////////////////////////////////// SETUP
void setup() {
  analogReadRes(10);                     // Set the ADC converteur resolution to 10 bit
  pinMode(LED_PIN, OUTPUT);              // Set rows pins in high-impedance state
  pinMode(BUTTON_PIN, INPUT_PULLUP);     // Set button pins as input and activate the input pullup resistor
  attachInterrupt(BUTTON_PIN, pushButton, RISING); // interrrupt 1 is data ready

  serial.setPacketHandler(&onPacket); // We must specify a packet handler method so that
  serial.begin(BAUD_RATE);  // Start the serial module

  for (int i = 0; i < ROWS; i++) {
    pinMode(rowPins[ROWS], INPUT);        // Set rows pins in high-impedance state
  }

  S.numCols = COLS;
  S.numRows = ROWS;
  S.pData = &inputVals[0];

  image.w = COLS * SCALE;
  image.h = ROWS * SCALE;
  image.pixels = &bilinIntOutput[0];
  bootBlink(9);

  // while (!Serial.dtr());                   // wait for user to start the serial monitor
  bootBlink(6);
  delay(500);
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
          myPacket[sensorID] = value;
        }
      }
      // Set row pin in high-impedance state
      pinMode(rowPins[row], INPUT);
    }
    scan = false;
  }
  bilinearInterpolation(1 / SCALE);

  // list_t *out, image_t *ptr, rectangle_t *roi, unsigned int x_stride, unsigned int y_stride,
  // list_t *thresholds, bool invert, unsigned int area_threshold, unsigned int pixels_threshold,
  // bool merge, int margin,
  // bool (*threshold_cb)(void*, find_blobs_list_lnk_data_t*), void *threshold_cb_arg,
  // bool (*merge_cb)(void*, find_blobs_list_lnk_data_t*, find_blobs_list_lnk_data_t*), void *merge_cb_arg
  imlib_find_blobs(
    &blobOut, &image, &roi, 1, 1,
    //thresholds, 0, 6, 4,
    //0, 0,
    //...
  );

  // The update() method attempts to read in
  // any incoming serial data and emits packets via
  // the user's onPacket(const uint8_t* buffer, size_t size)
  // method registered with the setPacketHandler() method.
  // The update() method should be called at the end of the loop().
  serial.update();
}

void Calibrate( uint8_t id, int val, int frame[] ) {
  static int calibrationCounter = 0;

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

void bilinearInterpolation(float inc) {
  float32_t posX, posY;
  int pos = 0;

  for (posX = 0; posX < ROWS; posX += inc) {
    for (posY = 0; posY < COLS; posY += inc) {
      bilinIntOutput[pos++] = arm_bilinear_interp_q7(&S, posX, posY);
    }
  }
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
    digitalWrite(LED_PIN, HIGH);
    delay(50);
    digitalWrite(LED_PIN, LOW);
    delay(50);
  }
}
