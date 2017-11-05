#ifndef __ETEXTILE_MATRIX_SENSOR_BLOB_H__
#define __ETEXTILE_MATRIX_SENSOR_BLOB_H__

#include <Arduino.h>
#include <arm_math.h>
#include "config.h"

long lastFarme = 0;
uint8_t fps = 0;
uint16_t sensorID = 0;

// Digital pins array
// See the attached home made PCB (Eagle file) to understand the Digital and Analog pin mapping
const int rowPins[ROWS] = {
  27, 26, 25, 24, 12, 11, 10, 9, 8, 7, 6, 5, 33, 2, 1, 0
};

// Analog pins array
const int columnPins[COLS] = {
  A17, A18, A19, A0, A20, A1, A2, A3, A4, A5, A6, A7, A11, A8, A10, A9
};

uint16_t minVals[ROW_FRAME] = {0};      // Array to store smallest values
uint16_t* minValsPtr;

float32_t frameValues[ROW_FRAME] = {0};      // Array to store ofset input values
float32_t* frameValuesPtr;

uint8_t bilinIntOutput[NEW_FRAME] = {0};     // Bilinear interpolation Output buffer
uint8_t* bilinIntOutputPtr;

// uint8_t myPacket[ROW_FRAME] = {0};           // Array to store values to transmit

#ifdef CORE_TEENSY
arm_bilinear_interp_instance_f32 interpolate;
#endif // __CORE_TEENSY__

char      bitmap[NEW_FRAME] = {0};
char*     bitmapPtr;

image_t   inputFrame;
image_t*  inputFramePtr;

list_t    freeNodeList;
list_t*   freeNodeListPtr;

list_t    nodes;
list_t*   nodesPtr;

list_t    oldNodesToUpdate;
list_t*   oldNodesToUpdatePtr;

list_t    nodesToUpdate;
list_t*   nodesToUpdatePtr;

list_t    nodesToAdd;
list_t*   nodesToAddPtr;

list_t    outputNodes;
list_t*   outputNodesPtr;

blob_t    tmpBlob;
blob_t*   tmpBlobPtr;

// void onPacket(const uint8_t* buffer, size_t size);
void calibrate(uint16_t* sumArray, const uint8_t cycles);
void bootBlink(uint8_t flash);
void pushButton();

#endif /*__ETEXTILE_MATRIX_SENSOR_BLOB_H__*/
