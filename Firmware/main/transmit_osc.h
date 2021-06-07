/*
  This file is part of the E256 - eTextile matrix sensor project - http://matrix.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#ifndef __TRANSMIT_OSC_H__
#define __TRANSMIT_OSC_H__

#include "config.h"
#include "presets.h"
#include "llist.h"
#include "blob.h"

#include <OSCBoards.h>              // https://github.com/CNMAT/OSC
#include <OSCMessage.h>             // https://github.com/CNMAT/OSC
#include <OSCBundle.h>              // https://github.com/CNMAT/OSC
#include <SLIPEncodedUSBSerial.h>   // https://github.com/CNMAT/OSC

typedef struct preset preset_t;     // Forward declaration
typedef struct llist llist_t;       // Forward declaration
typedef struct blob blob_t;         // Forward declaration

extern uint8_t currentMode;
extern uint8_t lastMode;

void USB_SLIP_OSC_SETUP(void);
void usb_slipOsc(preset_t* presets_ptr, image_t* rawFrame_ptr, image_t*interpFrame_ptr, llist_t* blobs_ptr);
void set_calibration(preset_t* presets_ptr);
void set_threshold(preset_t* presets_ptr);
void get_raw(image_t*interpFrame_ptr);
void get_interp(image_t*interpFrame_ptr);
void get_blobs(llist_t* blobs_ptr);

#endif /*__TRANSMIT_OSC_H__*/
