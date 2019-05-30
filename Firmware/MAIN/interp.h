/*
  This file is part of the eTextile-matrix-sensor project - http://matrix.eTextile.org
  Copyright (c) 2014-2019 Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#ifndef __INTERP_H__
#define __INTERP_H__

#include "blob.h"

#undef round
#define round(x) lround(x)

typedef struct {
  uint8_t scale_X;
  uint8_t scale_Y;
  uint16_t outputStride_Y;
  float* pCoefA;
  float* pCoefB;
  float* pCoefC;
  float* pCoefD;
} interp_t;

typedef struct image image_t; // forward declaration

void bilinear_interp_init(const interp_t* interp);
void bilinear_interp(const image_t* outputFrame, const image_t* inputFrame, const interp_t* interp);

#endif /*__INTERP_H__*/
