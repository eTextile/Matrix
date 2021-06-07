/*
  This file is part of the E256 - eTextile matrix sensor project - http://matrix.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#ifndef __PRESETS_H__
#define __PRESETS_H__

#include "config.h"
#include "interp.h"

#include <Bounce2.h>                   // https://github.com/thomasfredericks/Bounce2
#define ENCODER_OPTIMIZE_INTERRUPTS
#include <Encoder.h>                   // https://github.com/PaulStoffregen/Encoder
//#include <EEPROM.h>                  // https://www.arduino.cc/en/Reference/EEPROM (TODO)

typedef struct interp interp_t;        // forward declaration

#define LINE_OUT    0
#define SIG_IN      1
#define SIG_OUT     2
#define THRESHOLD   3
#define MIDI_LEARN  4
#define CALIBRATE   5
#define SAVE        6

extern uint8_t interpThreshold;
extern uint8_t currentMode;
extern uint8_t lastMode;

typedef struct preset preset_t;
struct preset {
  uint8_t minVal;
  uint8_t maxVal;
  uint8_t val;
  uint8_t ledVal;
  boolean setLed;
  boolean updateLed;
  boolean update;
  boolean D1;
  boolean D2;
};

void LEDS_SETUP(void);
void SWITCHES_SETUP(void);
void update_buttons(preset_t* presets_ptr);
void update_presets(preset_t* presets_ptr);
void update_leds(preset_t* preset_ptr);
boolean setLevel(preset_t* preset_ptr);

void preset_load(preset_t* preset_ptr, boolean* state_ptr); // TODO
void preset_save(preset_t* preset_ptr, boolean* state_ptr); // TODO

#endif /*__PRESETS_H__*/
