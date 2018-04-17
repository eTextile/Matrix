/*
  This file is part of the eTextile-matrix-sensor project - http://matrix.eTextile.org
  Copyright (c) 2014-2018 Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#ifndef __INTERP_H__
#define __INTERP_H__

#include "config.h"

float coef_A[X_SCALE * Y_SCALE] = {0};
float coef_B[X_SCALE * Y_SCALE] = {0};
float coef_C[X_SCALE * Y_SCALE] = {0};
float coef_D[X_SCALE * Y_SCALE] = {0};

typedef struct {
  uint8_t numCols;
  uint8_t numRows;
  float* pCoefA;
  float* pCoefB;
  float* pCoefC;
  float* pCoefD;
} interp_t;

interp_t  interp;

void bilinear_interp_init(interp_t* interp);
inline void bilinear_interp(const image_t* outputFrame, const image_t* inputFrame);

#endif /*__INTERP_H__*/
