/*
  This file is part of the E256 - eTextile matrix sensor project - http://matrix.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#ifndef __MAPPING_H__
#define __MAPPING_H__

#include "config.h"
#include "blob.h"
#include "llist.h"
#include "transmit_midi.h"
#include "notes.h"

typedef struct blob blob_t;         // Forward declaration
typedef struct llist llist_t;       // Forward declaration
typedef struct midiNode midiNode_t; // Forward declaration

#undef round
#define round(x) lround(x)

#define C_SLIDERS  3

typedef struct squareKey squareKey_t;
struct squareKey {
  int8_t val;
  uint8_t Xmin;
  uint8_t Xmax;
  uint8_t Ymin;
  uint8_t Ymax;
};

typedef struct tSwitch tSwitch_t;
struct tSwitch {
  uint8_t posX;
  uint8_t posY;
  uint8_t rSize; // width/2 and height/2
  uint32_t timeStamp;
  boolean state;
};

typedef struct vSlider vSlider_t;
struct vSlider {
  uint8_t posX;
  uint8_t Ymin;
  uint8_t Ymax;
  uint8_t width;
  uint8_t val;
};

typedef struct hSlider hSlider_t;
struct hSlider {
  uint8_t posY;
  uint8_t Xmin;
  uint8_t Xmax;
  uint8_t height;
  uint8_t val;
};

typedef struct cSlider cSlider_t;
struct cSlider {
  float r;
  uint8_t width;
  float phiOffset;
  float phiMin;
  float phiMax;
  uint8_t val;
};

#define GRID_COMPUTE_ROW_PTR(grid_ptr, y) (((squareKey_t *)(grid_ptr)) + ((GRID_COLS * sizeof(squareKey_t)) * y));
#define GRID_GET_KEY_PTR(col_ptr, x)(col_ptr + x);

void GRID_LAYOUT_SETUP(void);

void gridPopulate(llist_t* llist_ptr);
void gridPlay(llist_t* llist_ptr);

boolean trigger(llist_t* llist_ptr, tSwitch_t* switch_ptr);
boolean toggle(llist_t* llist_ptr, tSwitch_t* switch_ptr);
void hSlider(llist_t* llist_ptr, hSlider_t* slider_ptr);
void vSlider(llist_t* llist_ptr, vSlider_t* slider_ptr);
void cSlider(llist_t* llist_ptr, polar_t* polar_ptr, cSlider_t* slider_ptr);

#endif /*__MAPPING_H__*/
