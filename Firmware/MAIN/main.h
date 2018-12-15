/*
  This file is part of the eTextile-matrix-sensor project - http://matrix.eTextile.org
  Copyright (c) 2014-2018 Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#ifndef __MAIN_H__
#define __MAIN_H__

#include "config.h"
#include "interp.h"
#include "llist.h"
#include "blob.h"

SPISettings settings(16000000, MSBFIRST, SPI_MODE0); // LSBFIRST

ADC *adc = new ADC();     // ADC object
ADC::Sync_result result;  // ADC_0 & ADC_1

SLIPEncodedUSBSerial SLIPSerial(thisBoardsSerialUSB);

unsigned long lastFarme = 0;
uint16_t fps = 0;

// Array to store all parameters used to configure the two 8:1 analog multiplexeurs
// Eatch byte |ENA|A|B|C|ENA|A|B|C|
byte setDualRows[ROWS] = {
  0x55, 0x77, 0x66, 0x44, 0x22, 0x11, 0x00, 0x33
};

char      serialConf[4] = {0};              // Array to store boot serial config
uint8_t   minVals[ROW_FRAME] = {0};         // Array to store smallest values

uint8_t   frameValues[ROW_FRAME] = {0};     // Array to store ofseted input values
image_t   rawFrame;                         // Instance of struct image_t

float     coef_A[SCALE_X * SCALE_Y] = {0};
float     coef_B[SCALE_X * SCALE_Y] = {0};
float     coef_C[SCALE_X * SCALE_Y] = {0};
float     coef_D[SCALE_X * SCALE_Y] = {0};

interp_t  interp;                           // Instance of struct interp_t

uint8_t   bilinIntOutput[NEW_FRAME] = {0};  // Bilinear interpolation output buffer
image_t   interpolatedFrame;                // Instance of struct image_t

char      bitmap[NEW_FRAME] = {0};          // 64 x 64
blob_t    blobArray[MAX_NODES] = {0};       // 40 nodes

llist_t   freeBlobs;
llist_t   blobs;
llist_t   outputBlobs;

void matrix_scan(void);

void matrix_calibration(OSCMessage &msg);
void matrix_threshold(OSCMessage &msg);
void matrix_raw_data(OSCMessage &msg);
void matrix_blobs(OSCMessage &msg);

//void bootBlink(const uint8_t pin, uint8_t flash);

#endif /*__MAIN_H__*/
