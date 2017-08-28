// E-256 eTextile matrix sensor shield V2.0

#include "E256.h"

list_t blobOut;

////////////////////////////////////// SETUP
void setup() {

  serial.setPacketHandler(&onPacket); // We must specify a packet handler method so that
  serial.begin(BAUD_RATE);  // Start the serial module

  SPI.begin();              // Start the SPI module
  pinMode(SS, OUTPUT);      // Set up slave mode
  SPI.transfer(0xFFFFFF);   // All OFF
  digitalWrite(SS, LOW);    // set latchPin LOW
  digitalWrite(SS, HIGH);   // set latchPin HIGH
  S.numRows = ROWS;
  S.numCols = COLS;
  S.pData = &inputVals[0];
}

////////////////////////////////////// LOOP
void loop() {

  if (scan) {

    for (byte row = 0; row < ROWS; row++) {
      for (byte col = 0; col < COLS; col++) {

        if (row < 8) {
          byteA = setRows[row];
          byteB = setCols[col];
          byteC = 0x0;
          SPI.transfer(byteA);    // shift out the first byte
          SPI.transfer(byteB);    // shift out the secound byte
          SPI.transfer(byteC);    // shift out the terird byte
          digitalWrite(SS, LOW);  // set latchPin LOW
          digitalWrite(SS, HIGH); // set latchPin HIGH
        } else {
          byteA = setRows[row];
          byteB = 0x0;
          byteC = setCols[col];
          SPI.transfer(byteA);    // shift out the first byte
          SPI.transfer(byteB);    // shift out the secound byte
          SPI.transfer(byteC);    // shift out the terird byte
          digitalWrite(SS, LOW);  // set latchPin LOW
          digitalWrite(SS, HIGH); // set latchPin HIGH
        }

        int rowValue = analogRead(A0_PIN);  // Reding use to store analog inputs values
        byte sensorID = col * COLS + row;   // Calculate the unidimensional array index

        if (calibration) {
          Calibrate(sensorID, rowValue, minVals);
        } else {
          uint8_t value = map(rowValue, minVals[sensorID], 1024, 0, 255);
          myPacket[sensorID] = value;
        }

      }
    }
    scan = false;
  }

  bilinearInterpolation(1 / SCALE);
  // imlib_find_blobs(&blobOut, );

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
      bilinIntOutput[pos++] = arm_bilinear_interp_f32(&S, posX, posY);
    }
  }
}
