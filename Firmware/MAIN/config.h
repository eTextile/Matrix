/*
  This file is part of the eTextile-matrix-sensor project - http://matrix.eTextile.org
  Copyright (c) 2014-2018 Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <Arduino.h>
#include <arm_math.h>
#include <ADC.h>                  // https://github.com/pedvide/ADC
#include <SPI.h>                  // https://www.pjrc.com/teensy/td_libs_SPI.html
#include <OSCMessage.h>           // https://github.com/CNMAT/OSC
#include <OSCBundle.h>            // https://github.com/CNMAT/OSC
#include <OSCBoards.h>            // https://github.com/CNMAT/OSC
#include <SLIPEncodedUSBSerial.h> // https://github.com/CNMAT/OSC

#include "blob.h"
#include "llist.h"

// #define BUILTIN_LED    13      // FIXME - Hardware SPI conflict
// #define BUTTON_PIN     32      // FIXME - NO BUTTON_PIN ON the E256
#define BAUD_RATE         230400
#define COLS              16
#define ROWS              16
#define DUAL_ROWS         ROWS / 2
#define SCALE             4

#define ROW_FRAME         (COLS * ROWS)
#define NEW_COLS          (COLS * SCALE)
#define NEW_ROWS          (ROWS * SCALE)
#define NEW_FRAME         (NEW_COLS * NEW_ROWS)
#define INC               (1.0 / SCALE)
#define CYCLES            10    // Set the calibration cycles
#define THRESHOLD         15    // Set the threshold that determine toutch sensitivity (10 is low 30 is high)
#define MAX_NODES         20    // Set the maximum nodes number
#define MIN_BLOB_PIX      4     // Set the minimum blob pixels
#define MAX_BLOB_PIX      1024  // Set the maximum blob pixels
#define E256_EOF         'A'    // End Of Frame is ascii value letter 'A'

#define E256_SS_PIN      10     // SPI:SS    E2B56:RCK
#define E256_SCK_PIN     13     // SPI:SCK   E2B56:SCK
#define E256_MOSI_PIN    11     // SPI:MOSI  E2B56:DS

#define ADC0_PIN         A9     // The output of multiplexerA (SIG pin) is connected to Arduino Analog pin 0
#define ADC1_PIN         A3     // The output of multiplexerB (SIG pin) is connected to Arduino Analog pin 1

// Here is the control board
//#define E256_FPS
#define E256_ADC
#define E256_SERIAL
//#define E256_INTERP
//#define E256_BLOB
//#define E256_BLOB_ID
//#define E256_OSC

#define DEBUG_ADC
//#define DEBUG_OSC
#define DEBUG_INTERP      false
#define DEBUG_BITMAP      false
#define DEBUG_CCL         false
#define DEBUG_CENTER      false
#define DEBUG_LIST        false
#define DEBUG_BLOB        false

#endif /*__CONFIG_H__*/
