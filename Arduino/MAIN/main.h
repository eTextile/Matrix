/*
  This file is part of the eTextile-matrix-sensor project - http://matrix.eTextile.org
  Copyright (c) 2014-2018 Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#ifndef __MAIN_H__
#define __MAIN_H__

#include "config.h"

SPISettings settings(16000000, MSBFIRST, SPI_MODE0);

ADC *adc = new ADC();     // ADC object
ADC::Sync_result result;  // ADC_0 & ADC_1

#ifdef E256_OSC
SLIPEncodedUSBSerial SLIPSerial(thisBoardsSerialUSB); // Extended serial library object
#endif

char debugMode = 0;

unsigned long lastFarme = 0;
uint16_t fps = 0;

// Array to store all parameters used to configure the two analog multiplexeurs
const uint8_t setDualRows[ROWS] = {
  0x55, 0x77, 0x33, 0x11, 0x22, 0x44, 0x00, 0x66
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

char      bitmap[NEW_FRAME] = {0};    // 64 x 64
blob_t    blobArray[MAX_NODES] = {0}; // 20 nodes

llist_t   freeBlobs;
llist_t   blobs;
llist_t   outputBlobs;

void bootBlink(const uint8_t pin, uint8_t flash);
void calib(void);

#endif /*__MAIN_H__*/
