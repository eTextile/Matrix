/*
  This file is part of the eTextile-matrix-sensor project - http://matrix.eTextile.org
  Copyright (c) 2014-2018 Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#ifndef __INTERP_H__
#define __INTERP_H__

#include <Arduino.h>
#include "config.h"
#include "blob.h"

#define round(x) lround(x)

float coef_A[SCALE_X * SCALE_Y] = {0};
float coef_B[SCALE_X * SCALE_Y] = {0};
float coef_C[SCALE_X * SCALE_Y] = {0};
float coef_D[SCALE_X * SCALE_Y] = {0};

typedef struct {
  uint8_t scale_X;
  uint8_t scale_Y;
  uint16_t outputStride_Y;
  float* pCoefA;
  float* pCoefB;
  float* pCoefC;
  float* pCoefD;
} interp_t;


void bilinear_interp_init(interp_t* interp);
void bilinear_interp(const image_t* outputFrame, const image_t* inputFrame, const interp_t* interp);

#endif /*__INTERP_H__*/
