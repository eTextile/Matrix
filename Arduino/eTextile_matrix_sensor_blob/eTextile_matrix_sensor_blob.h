#ifndef __ETEXTILE_MATRIX_SENSOR_BLOB_H__
#define __ETEXTILE_MATRIX_SENSOR_BLOB_H__

#include <Arduino.h>
#include <arm_math.h>
#include "config.h"

unsigned long lastFarme = 0;
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
float32_t frameValues[ROW_FRAME] = {0};      // Array to store ofset input values
uint16_t bilinIntOutput[NEW_FRAME] = {0};     // Bilinear interpolation Output buffer

#ifdef CORE_TEENSY
arm_bilinear_interp_instance_f32 interpolate;
#endif // __CORE_TEENSY__

image_t   inputFrame;
char      bitmap[NEW_FRAME] = {0};
xylf_t    cclArray[NEW_FRAME] = {0};
lifo_t    lifo;

blob_t    blobsArray[MAX_NODES];
list_t    freeBlobs;

list_t    blobs;
list_t    blobsToUpdate;
list_t    blobsToAdd;
list_t    outputBlobs;

// uint8_t myPacket[ROW_FRAME] = {0};           // Array to store values to transmit

// void onPacket(const uint8_t* buffer, size_t size);
void calibrate(uint16_t* sumArray, const uint8_t cycles);
void bootBlink(uint8_t flash);
void pushButton();

#endif /*__ETEXTILE_MATRIX_SENSOR_BLOB_H__*/
