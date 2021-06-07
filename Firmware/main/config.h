/*
  This file is part of the E256 - eTextile matrix sensor project - http://matrix.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <Arduino.h>

#define NAME                "E256"
#define VERSION             "1.0.5"

#define USB_MIDI            0  // [0:1] Set the eTextile-Synthesizer as USB MIDI divice **DO NOT FORGET: Arduino/Touls/USB_Type/MIDI**
#define USB_SLIP_OSC        1  // [0:1] Set the eTextile-Synthesizer as USB SLIP_OSC divice **DO NOT FORGET: Arduino/Touls/USB_Type/Serial**
#define HARDWARE_MIDI       0  // [0:1] Set the eTextile-Synthesizer as MIDI divice **DO NOT FORGET: Arduino/Touls/USB_Type/Serial**
#define MAPPING_LAYAOUT     0  // [0:1] ...

// Arduino serial monitor
#define DEBUG_FPS           0  // [0:1] Print Frames Per Second
#define DEBUG_ENCODER       0  // [0:1] Print encoder value 
#define DEBUG_BUTTONS       0  // [0:1] Print buttons states
#define DEBUG_ADC           0  // [0:1] Print 16x16 Analog raw values
#define DEBUG_INTERP        0  // [0:1] Print 64x64 interpolated values
#define DEBUG_BITMAP        0  // [0:1] Print 64x64 binary image based on threshold
#define DEBUG_FIND_BLOBS    0  // [0:1] Print lowlevel blobs values
#define DEBUG_BLOBS         0  // [0:1] Print blobs values
#define DEBUG_MAPPING       0  // [0:1] Print blobs values

#define BAUD_RATE           230400
#define RAW_COLS            16
#define RAW_ROWS            16
#define RAW_FRAME           (RAW_COLS * RAW_ROWS)
#define SCALE_X             4
#define SCALE_Y             4
#define NEW_COLS            (RAW_COLS * SCALE_X)
#define NEW_ROWS            (RAW_ROWS * SCALE_Y)
#define NEW_FRAME           (NEW_COLS * NEW_ROWS)
#define X_MAX               58 // Blobs centroid X max value
#define Y_MAX               58 // Blobs centroid Y max value
#define MAX_SYNTH           8  // [1:8] How many synthesizers can be played at the same time

#define PI                  3.1415926535897932384626433832795
#define PI2                 (PI+PI)

#endif /*__CONFIG_H__*/
