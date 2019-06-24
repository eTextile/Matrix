/*
  This file is part of the eTextile-matrix-sensor project - http://matrix.eTextile.org
  Copyright (c) 2014-2019 Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#ifndef __INTERP_H__
#define __INTERP_H__

#include "config.h"
#include "blob.h"

typedef struct image image_t; // forward declaration

#undef round
#define round(x) lround(x)

typedef struct interp {
  uint8_t   scale_X;
  uint8_t   scale_Y;
  uint16_t  outputStride_Y;
  float*    pCoefA;
  float*    pCoefB;
  float*    pCoefC;
  float*    pCoefD;
} interp_t;

void INTERP_SETUP(
  image_t* inputFrame_ptr,
  uint8_t* inputArray_ptr,
  image_t* outputFrame,
  uint8_t* outputArray,
  interp_t* interp
);

void matrix_interp(
  image_t* outputFrame_ptr,
  image_t* inputFrame_ptr,
  const interp_t* interp
);


#endif /*__INTERP_H__*/
