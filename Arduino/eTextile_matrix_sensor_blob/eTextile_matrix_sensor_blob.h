#ifndef __ETEXTILE_MATRIX_SENSOR_BLOB_H__
#define __ETEXTILE_MATRIX_SENSOR_BLOB_H__

#include <Arduino.h>
#include <arm_math.h>
#include <ADC.h>            // https://github.com/pedvide/ADC
//#include <PacketSerial.h> // https://github.com/bakercp/PacketSerial

#include "collections.h"
#include "config.h"

ADC *adc = new ADC(); // adc object

unsigned long lastFarme = 0;
uint16_t fps = 0;

uint16_t sensorID = 0;

// Digital pins array
// See the attached home made PCB (Eagle file) to understand the Digital and Analog pin mapping
const uint8_t rowPins[ROWS] = {
  27, 26, 25, 24, 12, 11, 10, 9, 8, 7, 6, 5, 33, 2, 1, 0
};

// Analog pins array
const uint8_t columnPins[COLS] = {
  A17, A18, A19, A0, A20, A1, A2, A3, A4, A5, A6, A7, A11, A8, A10, A9
};

uint8_t minVals[ROW_FRAME] = {0};            // Array to store smallest values
float32_t frameValues[ROW_FRAME] = {0};      // Array to store ofseted input values
uint8_t bilinIntOutput[NEW_FRAME] = {0};     // Bilinear interpolation Output buffer
uint8_t maxVals[NEW_FRAME] = {0};

#ifdef CORE_TEENSY
// See : https://os.mbed.com/teams/Renesas/code/mbed-dsp-neon/docs/a912b042151f/group__BilinearInterpolate.html
arm_bilinear_interp_instance_f32 interpolate;
// image_t interpolate;
// arm_bilinear_interp_q7 TODO?
#endif // __CORE_TEENSY__

image_t   inputFrame;
char      bitmap[NEW_FRAME] = {0};

xylf_t    cclArray[NEW_FRAME] = {0}; // 4095 nodes
lifo_t    freeNodes;
lifo_t    lifo;

blob_t    blobArray[MAX_NODES]; // 10 nodes
llist_t   freeBlobs;
llist_t   blobs;
llist_t   blobsToUpdate;
llist_t   blobsToAdd;
llist_t   outputBlobs;

// PacketSerial serial;
// uint8_t myPacket[ROW_FRAME] = {0};           // Array to store frameValuess to transmit

void onPacket(const uint8_t* buffer, size_t size);
void calibrate(uint8_t *frameArray, const uint8_t cycles);
void bootBlink(const uint8_t pin, const uint8_t flash);
void pushButton();

#endif /*__ETEXTILE_MATRIX_SENSOR_BLOB_H__*/
