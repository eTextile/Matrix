/*
  This file is part of the E256 - eTextile matrix sensor project - http://matrix.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#ifndef __INTERP_H__
#define __INTERP_H__

#include "config.h"
#include "blob.h"

typedef struct image image_t; // forward declaration

#undef round
#define round(x) lround(x)

extern uint8_t interpThreshold;

typedef struct interp interp_t;
struct interp {
  uint8_t   scaleX;
  uint8_t   scaleY;
  uint16_t  outputStrideY;
  float*    pCoefA;
  float*    pCoefB;
  float*    pCoefC;
  float*    pCoefD;
};

void INTERP_SETUP(image_t* outputFrame);
void interp_matrix(image_t* inputFrame_ptr);

#endif /*__INTERP_H__*/
