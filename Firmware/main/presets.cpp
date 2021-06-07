/*
  This file is part of the E256 - eTextile matrix sensor project - http://matrix.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "presets.h"

// DO NOT CHANGE
#define LED_PIN_D1              5
#define LED_PIN_D2              4
#define BUTTON_PIN_L            2
#define BUTTON_PIN_R            3
#define ENCODER_PIN_A           22
#define ENCODER_PIN_B           9

Encoder encoder(ENCODER_PIN_A, ENCODER_PIN_B);

//Button BUTTON_L = Button();
Bounce2::Button BUTTON_L = Bounce2::Button();
//Button BUTTON_R = Button();
Bounce2::Button BUTTON_R = Bounce2::Button();

#define LONG_HOLD               1500
#define MIDI_LEARN_LED_TIMEON   600
#define MIDI_LEARN_LED_TIMEOFF  600
#define CALIBRATE_LED_TIMEON    35
#define CALIBRATE_LED_TIMEOFF   100
#define CALIBRATE_LED_ITER      4
#define SAVE_LED_TIMEON         20
#define SAVE_LED_TIMEOFF        50
#define SAVE_LED_ITER           10

void LEDS_SETUP(void) {
  pinMode(LED_PIN_D1, OUTPUT);
  pinMode(LED_PIN_D2, OUTPUT);
}

// Hear it should not compile if you didn't install the library (Manually!)
// [Bounce2](https://github.com/thomasfredericks/Bounce2)
// in your /Applications/Arduino/library
void SWITCHES_SETUP(void) {
  BUTTON_L.attach(BUTTON_PIN_L, INPUT_PULLUP);  // Attach the debouncer to a pin with INPUT_PULLUP mode
  BUTTON_R.attach(BUTTON_PIN_R, INPUT_PULLUP);  // Attach the debouncer to a pin with INPUT_PULLUP mode
  BUTTON_L.interval(25);                        // Debounce interval of 25 millis
  BUTTON_R.interval(25);                        // Debounce interval of 25 millis
}

void update_buttons(preset_t* presets_ptr) {
  BUTTON_L.update();
  BUTTON_R.update();
  // ACTION : BUTTON_L short press
  // FONCTION : CALIBRATE THE SENSOR MATRIX
  if (BUTTON_L.rose() && BUTTON_L.previousDuration() < LONG_HOLD) {
    lastMode = currentMode;
    currentMode = CALIBRATE;
    presets_ptr[CALIBRATE].setLed = true;
    presets_ptr[CALIBRATE].updateLed = true;
#if DEBUG_BUTTONS
    Serial.printf("\nBUTTON_L : CALIBRATE : %d", currentMode);
#endif
  }
  // ACTION : BUTTON_L long press
  // FONCTION : SAVE ALL PARAMETERS TO THE EEPROM MEMORY
  if (BUTTON_L.rose() && BUTTON_L.previousDuration() > LONG_HOLD) {
    lastMode = currentMode;
    currentMode = SAVE;
    presets_ptr[SAVE].setLed = true;
#if DEBUG_BUTTONS
    Serial.printf("\nBUTTON_L : SAVE : %d", currentMode);
#endif
  }
  // ACTION : BUTTON_R short press
  // FONCTION : SELECT_MODE
  if (BUTTON_R.rose() && BUTTON_R.previousDuration() < LONG_HOLD) {
    lastMode = currentMode;                // Save last Mode
    currentMode = (currentMode + 1) % 4;   // Loop into the modes
    encoder.write(presets_ptr[currentMode].val << 2);
    presets_ptr[currentMode].setLed = true;
#if DEBUG_BUTTONS
    Serial.printf("\nBUTTON_R : SELECT_MODE : %d", currentMode);
#endif
  }
  // ACTION : BUTTON_R long press
  // FONCTION : MIDI_LEARN (send blob values separately for Max4Live MIDI_LEARN)
  // LEDs : alternates durring all the learning process
  if (BUTTON_R.rose() && BUTTON_R.previousDuration() > LONG_HOLD) {
    lastMode = currentMode;
    currentMode = MIDI_LEARN;
    encoder.write(0x1);
    presets_ptr[MIDI_LEARN].setLed = true;
#if DEBUG_BUTTONS
    Serial.printf("\nMIDI_LEARN : %d", currentMode);
#endif
  }
}

void update_presets(preset_t* presets_ptr) {
  switch (currentMode) {
    case LINE_OUT:
      if (setLevel(&presets_ptr[LINE_OUT])) {
        presets_ptr[LINE_OUT].ledVal = map(presets_ptr[LINE_OUT].val, presets_ptr[LINE_OUT].minVal, presets_ptr[LINE_OUT].maxVal, 0, 255);
        presets_ptr[LINE_OUT].updateLed = true;
        presets_ptr[LINE_OUT].update = true;
      }
      break;
    case SIG_IN:
      if (setLevel(&presets_ptr[SIG_IN])) {
        presets_ptr[SIG_IN].ledVal = map(presets_ptr[SIG_IN].val, presets_ptr[SIG_IN].minVal, presets_ptr[SIG_IN].maxVal, 0, 255);
        presets_ptr[SIG_IN].updateLed = true;
        presets_ptr[SIG_IN].update = true;
      }
      break;
    case SIG_OUT:
      if (setLevel(&presets_ptr[SIG_OUT])) {
        presets_ptr[SIG_OUT].ledVal = map(presets_ptr[SIG_OUT].val, presets_ptr[SIG_OUT].minVal, presets_ptr[SIG_OUT].maxVal, 0, 255);
        presets_ptr[SIG_OUT].updateLed = true;
        presets_ptr[SIG_OUT].update = true;
      }
      break;
    case THRESHOLD:
      if (setLevel(&presets_ptr[THRESHOLD])) {
        presets_ptr[THRESHOLD].ledVal = map(presets_ptr[THRESHOLD].val, presets_ptr[THRESHOLD].minVal, presets_ptr[THRESHOLD].maxVal, 0, 255);
        interpThreshold = constrain(presets_ptr[THRESHOLD].val - 5, presets_ptr[THRESHOLD].minVal, presets_ptr[THRESHOLD].maxVal);
        presets_ptr[THRESHOLD].updateLed = true;
        presets_ptr[THRESHOLD].update = true;
      }
      break;
    case MIDI_LEARN:
      if (setLevel(&presets_ptr[MIDI_LEARN])) {
        presets_ptr[MIDI_LEARN].updateLed = true;
        presets_ptr[MIDI_LEARN].update = true;
      }
      break;
    default:
      break;
  }
}

// Update LEDs according to the mode and rotary encoder values
void update_leds(preset_t* presets_ptr) {
  static uint32_t timeStamp = 0;
  static uint8_t iter = 0;

  switch (currentMode) {
    case LINE_OUT:
      if (presets_ptr[LINE_OUT].setLed) {
        presets_ptr[LINE_OUT].setLed = false;
        pinMode(LED_PIN_D1, OUTPUT);
        pinMode(LED_PIN_D2, OUTPUT);
        digitalWrite(LED_PIN_D1, presets_ptr[LINE_OUT].D1);
        digitalWrite(LED_PIN_D2, presets_ptr[LINE_OUT].D2);
      }
      if (presets_ptr[LINE_OUT].updateLed) {
        presets_ptr[LINE_OUT].updateLed = false;
        analogWrite(LED_PIN_D1, presets_ptr[LINE_OUT].ledVal);
        analogWrite(LED_PIN_D2, abs(presets_ptr[LINE_OUT].ledVal - 255));
      }
      break;
    case SIG_IN:
      if (presets_ptr[SIG_IN].setLed) {
        presets_ptr[SIG_IN].setLed = false;
        pinMode(LED_PIN_D1, OUTPUT);
        pinMode(LED_PIN_D2, OUTPUT);
        digitalWrite(LED_PIN_D1, presets_ptr[SIG_IN].D1);
        digitalWrite(LED_PIN_D2, presets_ptr[SIG_IN].D2);
      }
      if (presets_ptr[SIG_IN].updateLed) {
        presets_ptr[SIG_IN].updateLed = false;
        analogWrite(LED_PIN_D1, presets_ptr[SIG_IN].ledVal);
        analogWrite(LED_PIN_D2, abs(presets_ptr[SIG_IN].ledVal - 255));
      }
      break;
    case SIG_OUT:
      if (presets_ptr[SIG_OUT].setLed) {
        presets_ptr[SIG_OUT].setLed = false;
        pinMode(LED_PIN_D1, OUTPUT);
        pinMode(LED_PIN_D2, OUTPUT);
        digitalWrite(LED_PIN_D1, presets_ptr[SIG_OUT].D1);
        digitalWrite(LED_PIN_D2, presets_ptr[SIG_OUT].D2);
      }
      if (presets_ptr[SIG_OUT].updateLed) {
        presets_ptr[SIG_OUT].updateLed = false;
        analogWrite(LED_PIN_D1, presets_ptr[SIG_OUT].ledVal);
        analogWrite(LED_PIN_D2, abs(presets_ptr[SIG_OUT].ledVal - 255));
      }
      break;
    case THRESHOLD:
      if (presets_ptr[THRESHOLD].setLed) {
        presets_ptr[THRESHOLD].setLed = false;
        pinMode(LED_PIN_D1, OUTPUT);
        pinMode(LED_PIN_D2, OUTPUT);
        digitalWrite(LED_PIN_D1, presets_ptr[THRESHOLD].D1);
        digitalWrite(LED_PIN_D2, presets_ptr[THRESHOLD].D2);
      }
      if (presets_ptr[THRESHOLD].updateLed) {
        presets_ptr[THRESHOLD].updateLed = false;
        analogWrite(LED_PIN_D1, presets_ptr[THRESHOLD].ledVal);
        analogWrite(LED_PIN_D2, abs(presets_ptr[THRESHOLD].ledVal - 255));
      }
      break;
    case MIDI_LEARN: // LEDs : alternating blink
      if (presets_ptr[MIDI_LEARN].setLed) {
        presets_ptr[MIDI_LEARN].setLed = false;
        pinMode(LED_PIN_D1, OUTPUT);
        pinMode(LED_PIN_D2, OUTPUT);
        timeStamp = millis();
      }
      if (millis() - timeStamp < MIDI_LEARN_LED_TIMEON && presets_ptr[MIDI_LEARN].updateLed == true) {
        presets_ptr[MIDI_LEARN].updateLed = false;
        digitalWrite(LED_PIN_D1, HIGH);
        digitalWrite(LED_PIN_D2, LOW);
      }
      else if (millis() - timeStamp > MIDI_LEARN_LED_TIMEON && presets_ptr[MIDI_LEARN].updateLed == false) {
        presets_ptr[MIDI_LEARN].updateLed = true;
        digitalWrite(LED_PIN_D1, LOW);
        digitalWrite(LED_PIN_D2, HIGH);
      }
      else if (millis() - timeStamp > MIDI_LEARN_LED_TIMEON + MIDI_LEARN_LED_TIMEOFF) {
        timeStamp = millis();
      }
      break;
    case CALIBRATE: // LEDs : both LED are blinking three time
      if (presets_ptr[CALIBRATE].setLed) {
        presets_ptr[CALIBRATE].setLed = false;
        pinMode(LED_PIN_D1, OUTPUT);
        pinMode(LED_PIN_D2, OUTPUT);
        timeStamp = millis();
        iter = 0;
      }
      if (iter < CALIBRATE_LED_ITER) {
        if (millis() - timeStamp < CALIBRATE_LED_TIMEON && presets_ptr[CALIBRATE].updateLed == true) {
          presets_ptr[CALIBRATE].updateLed = false;
          digitalWrite(LED_PIN_D1, HIGH);
          digitalWrite(LED_PIN_D2, HIGH);
        }
        else if (millis() - timeStamp > CALIBRATE_LED_TIMEON && presets_ptr[CALIBRATE].updateLed == false) {
          presets_ptr[CALIBRATE].updateLed = true;
          digitalWrite(LED_PIN_D1, LOW);
          digitalWrite(LED_PIN_D2, LOW);
        }
        else if (millis() - timeStamp > CALIBRATE_LED_TIMEON + CALIBRATE_LED_TIMEOFF) {
          if (iter <= CALIBRATE_LED_ITER) {
            timeStamp = millis();
            iter++;
          }
          else {
            presets_ptr[CALIBRATE].update = true;
          }
        }
      }
      break;
    case SAVE: // LEDs : Both LED are blinking weary fast
      if (presets_ptr[SAVE].setLed) {
        presets_ptr[SAVE].setLed = false;
        pinMode(LED_PIN_D1, OUTPUT);
        pinMode(LED_PIN_D2, OUTPUT);
        timeStamp = millis();
        iter = 0;
      }
      if (iter < SAVE_LED_ITER) {
        if (millis() - timeStamp < SAVE_LED_TIMEON && presets_ptr[SAVE].updateLed == true) {
          presets_ptr[SAVE].updateLed = false;
          digitalWrite(LED_PIN_D1, HIGH);
          digitalWrite(LED_PIN_D2, HIGH);
        }
        else if (millis() - timeStamp > SAVE_LED_TIMEON && presets_ptr[SAVE].updateLed == false) {
          presets_ptr[SAVE].updateLed = true;
          digitalWrite(LED_PIN_D1, LOW);
          digitalWrite(LED_PIN_D2, LOW);
        }
        else if (millis() - timeStamp > SAVE_LED_TIMEON + SAVE_LED_TIMEOFF) {
          if (iter < SAVE_LED_ITER) {
            timeStamp = millis();
            iter++;
          }
        }
      }
      break;

    default:
      break;
  }
}

// Values adjustment using rotary encoder
boolean setLevel(preset_t* preset_ptr) {
  uint8_t val = encoder.read() >> 2;
  if (val != preset_ptr->val) {
    if (val > preset_ptr->maxVal) {
      encoder.write(preset_ptr->maxVal << 2);
      return false;
    }
    else if (val < preset_ptr->minVal) {
      encoder.write(preset_ptr->minVal << 2);
      return false;
    }
    else {
      preset_ptr->val = val;
      return true;
    }
  }
  else {
    return false;
  }
}

// TODO
void preset_load(preset_t* preset_ptr, boolean* state_ptr) {

  for (int i = 0; i < 4; i++) {
    // EEPROM.read(i, preset_ptr[i].val); // uint8_t
  }
  state_ptr = false;
}

// TODO
void preset_save(preset_t* preset_ptr, boolean* state_ptr) {

  for (int i = 0; i < 4; i++) {
    //EEPROM.write(i, preset_ptr[i].val); // uint8_t
  }
  state_ptr = false;
}
