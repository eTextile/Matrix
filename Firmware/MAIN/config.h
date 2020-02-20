
/*
  This file is part of the eTextile-matrix-sensor project - http://matrix.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>thun
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <Arduino.h>

/* __E256 BOARD CONTROL__*/

//#define E256_X0 -1        // TODO: select X-axis origine [-1:1]
//#define E256_Y0 -1        // TODO: select Y-axis origine [-1:1]

#define E256_RUN

//#define DEBUG_ADC
//#define DEBUG_INTERP
//#define DEBUG_BITMAP
//#define DEBUG_BLOBS_ID
//#define DEBUG_BLOBS_OSC
//#define DEBUG_FPS

//#define LED_BUILTIN         13        // FIXME - SPI (hardware) conflict
//#define BUTTON_PIN          32        // FIXME - NO BUTTON_PIN on the E256

//#define BAUD_RATE           115200
#define BAUD_RATE             230400
#define RAW_COLS              16
#define RAW_ROWS              16
#define DUAL_ROWS             (RAW_ROWS / 2)
#define SCALE_X               4
#define SCALE_Y               4
#define RAW_FRAME             (RAW_COLS * RAW_ROWS)
#define NEW_COLS              (RAW_COLS * SCALE_X)
#define NEW_ROWS              (RAW_ROWS * SCALE_Y)
#define NEW_FRAME             (NEW_COLS * NEW_ROWS)
#define MAX_NODES             40       // Set the maximum nodes number
#define LIFO_MAX_NODES        127      // Set the maximum nodes number
#define X_STRIDE              4
#define Y_STRIDE              1
#define MIN_BLOB_PIX          16       // Set the minimum blob pixels
#define MAX_BLOB_PIX          4095     // Set the maximum blob pixels
#define BLOB_PACKET_SIZE      7        // Blob data packet (bytes)

#define E256_SS_PIN           10       // SPI:SS    E2B56:RCK  // D10 - Hardware SPI no need to specify it
#define E256_SCK_PIN          13       // SPI:SCK   E2B56:SCK  // D13 - Hardware SPI no need to specify it
#define E256_MOSI_PIN         11       // SPI:MOSI  E2B56:DS   // D11 - Hardware SPI no need to specify it

#define ADC0_PIN              A9       // The output of multiplexerA (SIG pin) is connected to Analog pin 9
#define ADC1_PIN              A3       // The output of multiplexerB (SIG pin) is connected to Analog pin 3

#endif /*__CONFIG_H__*/
