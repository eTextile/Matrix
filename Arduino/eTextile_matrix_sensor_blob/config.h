#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <Arduino.h>
#include <arm_math.h>
#include <ADC.h>                  // https://github.com/pedvide/ADC
#include <OSCMessage.h>           // https://github.com/CNMAT/OSC
#include <OSCBundle.h>            // https://github.com/CNMAT/OSC
#include <OSCBoards.h>            // https://github.com/CNMAT/OSC
#include <SLIPEncodedUSBSerial.h> // https://github.com/CNMAT/OSC

#include "blob.h"

#define BUILTIN_LED       13
#define BUTTON_PIN        32    // Button on the eTextile Teensy shield V1.0
#define BAUD_RATE         230400
#define COLS              16
#define ROWS              16
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

// #define A0_PIN         A0    // The output of multiplexerA (SIG pin) is connected to Analog pin A0
// #define A1_PIN         A1    // The output of multiplexerB (SIG pin) is connected to Analog pin A1

#define PERSISTANT_ID     true

#define DEBUG_ADC_INPUT   false
#define DEBUG_INTERP      false
#define DEBUG_BITMAP      false
#define DEBUG_CCL         false
#define DEBUG_CENTER      false
#define DEBUG_LIST        false
#define DEBUG_BLOB        true
#define DEBUG_OSC         false

#endif /*__CONFIG_H__*/
