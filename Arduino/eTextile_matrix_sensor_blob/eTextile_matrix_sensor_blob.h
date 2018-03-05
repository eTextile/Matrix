#ifndef __ETEXTILE_MATRIX_SENSOR_BLOB_H__
#define __ETEXTILE_MATRIX_SENSOR_BLOB_H__

#include <Arduino.h>
#include <arm_math.h>
#include "config.h"

unsigned long lastFarme = 0;
uint16_t fps = 0;
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

uint8_t minVals[ROW_FRAME] = {0};            // Array to store smallest values
float32_t frameValues[ROW_FRAME] = {0};      // Array to store ofset input values
uint8_t bilinIntOutput[NEW_FRAME] = {0};     // Bilinear interpolation Output buffer

#ifdef CORE_TEENSY
// See : https://os.mbed.com/teams/Renesas/code/mbed-dsp-neon/docs/a912b042151f/group__BilinearInterpolate.html
arm_bilinear_interp_instance_f32 interpolate;
// arm_bilinear_interp_q7 TODO?
#endif // __CORE_TEENSY__

image_t   frame;
char      bitmap[NEW_FRAME] = {0};

xylf_t    cclArray[MAX_NODES] = {0}; // NEW_FRAME
lifo_t    freeNodes;
lifo_t    lifo;

blob_t    blobArray[MAX_NODES];
llist_t   freeBlobs;
llist_t   blobs;
llist_t   blobsToUpdate;
llist_t   blobsToAdd;
llist_t   outputBlobs;

// uint8_t myPacket[ROW_FRAME] = {0};           // Array to store values to transmit

// void onPacket(const uint8_t* buffer, size_t size);
void calibrate(uint8_t* sumArray, const uint8_t cycles);
void bootBlink(uint8_t flash);
void pushButton();

#endif /*__ETEXTILE_MATRIX_SENSOR_BLOB_H__*/
