/*
  This file is part of the eTextile-matrix-sensor project - http://matrix.eTextile.org
  Copyright (c) 2014-2018 Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#ifndef __INTERP_H__
#define __INTERP_H__

#include "config.h"

//float coef_A[scale_X * scale_Y] = {0}; // FIXME
//float coef_B[scale_X * scale_Y] = {0}; // FIXME
//float coef_C[scale_X * scale_Y] = {0}; // FIXME
//float coef_D[scale_X * scale_Y] = {0}; // FIXME

float coef_A[16] = {0};
float coef_B[16] = {0};
float coef_C[16] = {0};
float coef_D[16] = {0};

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
