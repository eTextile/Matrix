/*
  **E256 - eTextile matrix sensor**
  This file is part of the E256 - eTextile matrix sensor project - http://matrix.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "config.h"
#include "presets.h"
#include "scan.h"
#include "interp.h"
#include "blob.h"
#include "median.h"
#include "mapping.h"

#include <elapsedMillis.h>  // https://github.com/pfeerick/elapsedMillis

#if USB_MIDI
#include "transmit_midi.h"
#endif
#if USB_SLIP_OSC
#include "transmit_osc.h"
#endif

image_t  rawFrame;      // Input frame values
image_t  interpFrame;   // Interpolated frame values
llist_t  blobs;         // Output blobs linked list
llist_t  midiIn;        // MidiIn linked list

uint8_t currentMode = CALIBRATE;   // Init currentMode with CALIBRATE (DEFAULT_MODE)
uint8_t lastMode = LINE_OUT;       // Init lastMode with LINE_OUT (DEFAULT_MODE)

#if DEBUG_FPS
elapsedMillis curentMillisFps;
unsigned int fps = 0;
#endif

preset_t presets[7] = {
  {13, 31, 29, 0, false, false, false, LOW,  LOW },  // LINE_OUT   - ARGS[minVal, maxVal, val, ledVal, setLed, updateLed, update, D1, D2]
  { 1, 50, 12, 0, false, false, false, HIGH, LOW },  // SIG_IN     - ARGS[minVal, maxVal, val, ledVal, setLed, updateLed, update, D1, D2]
  { 1, 31, 17, 0, false, false, false, LOW,  HIGH},  // SIG_OUT    - ARGS[minVal, maxVal, val, ledVal, setLed, updateLed, update, D1, D2]
  { 5, 30, 10, 0, false, false, false, HIGH, HIGH},  // THRESHOLD  - ARGS[minVal, maxVal, val, ledVal, setLed, updateLed, update, D1, D2]
  { 1, 6,  1,  0, false, false, false, NULL, NULL},  // MIDI_LEARN - ARGS[minVal, maxVal, val, ledVal, setLed, updateLed, update, D1, D2]
  { 0, 0,  0,  0, true,  true,  false, NULL, NULL},  // CALIBRATE  - ARGS[minVal, maxVal, val, ledVal, setLed, updateLed, update, D1, D2]
  { 0, 0,  0,  0, false, false, false, NULL, NULL}   // SAVE       - ARGS[minVal, maxVal, val, ledVal, setLed, updateLed, update, D1, D2]
};

#if MAPPING_LAYAOUT
tSwitch_t trigParam = {10, 10, 5, 1000, false};     // ARGS[posX, posY, rSize, debounceTimer, state]
tSwitch_t toggParam = {40, 30, 5, 1000, false};     // ARGS[posX, posY, rSize, debounceTimer, state]
vSlider_t vSliderParam = {10, 15, 40, 5, 0};        // ARGS[posX, Ymin, Ymax, width, val]
hSlider_t hSliderParam = {30, 15, 40, 5, 0};        // ARGS[posY, Xmin, Xmax, width, val]
cSlider_t cSlidersParam[C_SLIDERS] = {
  {   6, 4,  3.8,  5, 0},                           // ARGS[r, width, phiOffset, phiMax, val]
  {13.5, 3,  3.8, 10, 0},                           // ARGS[r, width, phiOffset, phiMax, val]
  {  20, 4,  4.8,  5, 0}                            // ARGS[r, width, phiOffset, phiMax, val]
};
ccPesets_t ccParam = {NULL, BD, 44, 1, 0};          // ARGS[blobID, [BX,BY,BW,BH,BD], cChange, midiChannel, Val]
#endif

void setup() {
#if DEBUG_ADC || DEBUG_INTERP || DEBUG_BITMAP || DEBUG_BLOBS || DEBUG_FPS || DEBUG_ENCODER || DEBUG_BUTTONS || DEBUG_MAPPING
  Serial.begin(BAUD_RATE); // Start Serial communication using 230400 baud
  while (!Serial);
  Serial.printf("\n%s_%s", NAME, VERSION);
#endif
  LEDS_SETUP();
  SWITCHES_SETUP();
  SPI_SETUP();
  ADC_SETUP();

  SCAN_SETUP(&rawFrame);
  INTERP_SETUP(&interpFrame);
  BLOB_SETUP(&blobs);

#if USB_MIDI
  USB_MIDI_SETUP();
#endif
#if USB_SLIP_OSC
  USB_SLIP_OSC_SETUP();
#endif
#if HARDWARE_MIDI
  HARDWARE_MIDI_SETUP();
#endif

};

void loop() {

  update_buttons(&presets[0]);
  update_presets(&presets[0]);
  update_leds(&presets[0]);

  calibrate_matrix(&presets[0]);
  scan_matrix();
  interp_matrix(&rawFrame);
  find_blobs(presets[THRESHOLD].val, &interpFrame, &blobs);

  //median(&blobs);
  //getPolarCoordinates(&blobs);
  //getBlobsVelocity(&blobs);

#if USB_MIDI
  if (currentMode == MIDI_LEARN) {
    usb_midi_learn(&blobs, &presets[MIDI_LEARN]);
  }
  else {
    usb_midi_play(&blobs);
  };
#endif

#if USB_SLIP_OSC
  usb_slipOsc(&presets[0], &rawFrame, &interpFrame, &blobs);
#endif

#if HARDWARE_MIDI
  if (handleMidiInput(&midiIn)) {
    gridPopulate(&midiIn);
  };
#endif

#if MAPPING_LAYAOUT
  gridPlay(&blobs);
  //controlChange(&blobs, &ccParam);
  //boolean toggSwitch = toggle(&blobs, &toggParam);
  //boolean trigSwitch = trigger(&blobs, &trigParam);
  //hSlider(&blobs, &hSliderParam);
  //vSlider(&blobs, &vSliderParam);
  //cSlider(&blobs, &polarCoord[0], &cSlidersParam[0]);
#endif

#if DEBUG_FPS
  if (curentMillisFps >= 1000) {
    curentMillisFps = 0;
    Serial.printf("\nFPS:%d\tCPU:%f\tMEM:%f", fps, AudioProcessorUsageMax(), AudioMemoryUsageMax());
    AudioProcessorUsageMaxReset();
    AudioMemoryUsageMaxReset();
    fps = 0;
  };
  fps++;
#endif

};
