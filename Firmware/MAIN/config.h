/*
  This file is part of the eTextile-matrix-sensor project - http://matrix.eTextile.org
  Copyright (c) 2014-2018 Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <ADC.h>                    // https://github.com/pedvide/ADC
#include <SPI.h>                    // https://www.pjrc.com/teensy/td_libs_SPI.html

#include <OSCBoards.h>              // https://github.com/CNMAT/OSC
#include <OSCMessage.h>             // https://github.com/CNMAT/OSC
#include <OSCBundle.h>              // https://github.com/CNMAT/OSC

#include <SLIPEncodedUSBSerial.h>   // https://github.com/CNMAT/OSC
//#include <PacketSerial.h>         // https://github.com/bakercp/PacketSerial (Send bundle is not implemented!)

//#define BUILTIN_LED         13        // FIXME - Hardware SPI conflict
//#define BUTTON_PIN          32        // FIXME - NO BUTTON_PIN on the E256
#define BAUD_RATE             230400
#define COLS                  16
#define ROWS                  16
#define DUAL_ROWS             (ROWS / 2)
#define SCALE_X               4
#define SCALE_Y               4
#define ROW_FRAME             (COLS * ROWS)
#define NEW_COLS              (COLS * SCALE_X)
#define NEW_ROWS              (ROWS * SCALE_Y)
#define NEW_FRAME             (NEW_COLS * NEW_ROWS)
#define CYCLES                10    // Set the calibration cycles
#define THRESHOLD             35    // Set the threshold that determine toutch sensitivity (10 is low 30 is high)
#define MAX_NODES             40    // Set the maximum nodes number
#define MIN_BLOB_PIX          4     // Set the minimum blob pixels
#define MAX_BLOB_PIX          1024  // Set the maximum blob pixels

#define E256_SS_PIN           10    // SPI:SS    E2B56:RCK  // D10 - Hardware SPI no need to specify it
#define E256_SCK_PIN          13    // SPI:SCK   E2B56:SCK  // D13 - Hardware SPI no need to specify it
#define E256_MOSI_PIN         11    // SPI:MOSI  E2B56:DS   // D11 - Hardware SPI no need to specify it

#define ADC0_PIN              A9    // The output of multiplexerA (SIG pin) is connected to Analog pin 9
#define ADC1_PIN              A3    // The output of multiplexerB (SIG pin) is connected to Analog pin 3

// Here is the board control
#define E256_FPS              0

#define E256_ADC_SYNCHRO      1
#define E256_BLOBS_SLIP_OSC   1

#define DEBUG_ADC             0
#define DEBUG_INTERP          0
#define DEBUG_BLOBS_OSC       0

#define DEBUG_BITMAP          0
#define DEBUG_CCL             0
#define DEBUG_CENTER          0
#define DEBUG_LIST            0
#define DEBUG_BLOBS           0

#endif /*__CONFIG_H__*/
