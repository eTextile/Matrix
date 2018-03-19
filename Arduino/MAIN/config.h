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
#include <SPI.h>                  // Include the new SPI library
#include <OSCMessage.h>           // https://github.com/CNMAT/OSC
#include <OSCBundle.h>            // https://github.com/CNMAT/OSC
#include <OSCBoards.h>            // https://github.com/CNMAT/OSC
#include <SLIPEncodedUSBSerial.h> // https://github.com/CNMAT/OSC


#include "blob.h"
#include "llist.h"

#define BUILTIN_LED       13
#define BUTTON_PIN        32          // Button on the eTextile Teensy shield V1.0
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

#define  E256_MOSI_PIN    11  // SPI
#define  E256_MISO_PIN    10  // SPI
#define  E256_SCK_PIN     12  // SPI
#define  A0_PIN           A9  // The output of multiplexerA (SIG pin) is connected to Arduino Analog pin 0
#define  A1_PIN           A3  // The output of multiplexerB (SIG pin) is connected to Arduino Analog pin 1


#define E256_SERIAL
// #define DEBUG_ADC_INPUT
// #define E256_OSC
// #define DEBUG_OSC

#define PERSISTANT_ID     true
#define DEBUG_INTERP      false
#define DEBUG_BITMAP      false
#define DEBUG_CCL         false
#define DEBUG_CENTER      false
#define DEBUG_LIST        false
#define DEBUG_BLOB        false


#endif /*__CONFIG_H__*/
