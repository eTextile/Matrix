/*
  This file is part of the E256 - eTextile matrix sensor project - http://matrix.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#ifndef __TRANSMIT_MIDI_H__
#define __TRANSMIT_MIDI_H__

#include "config.h"
#include "presets.h"
#include "llist.h"
#include "blob.h"

typedef struct preset preset_t; // Forward declaration
typedef struct llist llist_t;   // Forward declaration
typedef struct blob blob_t;     // Forward declaration

#if USB_MIDI || HARDWARE_MIDI
#include <MIDI.h>               // http://www.pjrc.com/teensy/td_midi.html
#endif

#if HARDWARE_MIDI
extern MIDI_NAMESPACE::SerialMIDI<HardwareSerial>serialMIDI;
extern MIDI_NAMESPACE::MidiInterface<MIDI_NAMESPACE::SerialMIDI<HardwareSerial>> MIDI;
#endif

/*
  #define BI  0  // Blob UID
  #define BS  1  // Blob State
  #define BL, 2  // Blob Last State
  #define BX  3  // Blob Centroid PosX
  #define BY  4  // Blob Centroid PosY
  #define BW  5  // Blob width
  #define BH  6  // Blob Height
  #define BD  7  // Blob Depth
*/

typedef enum params {
  BI,  // Blob UID
  BS,  // Blob State
  BL,  // Blob Last State
  BX,  // Blob Centroid PosX
  BY,  // Blob Centroid PosY
  BW,  // Blob width
  BH,  // Blob Height
  BD   // Blob Depth
} params_t;

typedef struct midiNode midiNode_t;
struct midiNode {
  lnode_t node;
  uint8_t pithch;
  uint8_t velocity;
  uint8_t channel;
};

typedef struct ccPesets ccPesets_t;
struct ccPesets {
  uint8_t blobID;
  uint8_t mappVal;
  int8_t cChange;
  int8_t midiChannel;
  int8_t val;
};

#if USB_MIDI
void USB_MIDI_SETUP(void);
void usb_midi_learn(llist_t* llist_ptr, preset_t* preset_ptr);
void usb_midi_play(llist_t* llist_ptr);
#endif

#if HARDWARE_MIDI
void HARDWARE_MIDI_SETUP(void);
void midi_llist_init(llist_t* midiNodes_ptr, midiNode_t* nodeArray_ptr);
boolean handleMidiInput(llist_t* llist_ptr);
void controlChange(llist_t* llist_ptr, ccPesets_t* ccPesets_ptr);
#endif

#endif /*__TRANSMIT_MIDI_H__*/
