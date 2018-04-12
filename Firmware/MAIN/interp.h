/*
  This file is part of the eTextile-matrix-sensor project - http://matrix.eTextile.org
  Copyright (c) 2014-2018 Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

// Bilinear interpolation

#ifndef __INTERP_H__
#define __INTERP_H__

#include "config.h"

#define C0  (float)1
#define C1  (float)2/3
#define C2  (float)1/2
#define C3  (float)1/3
#define C4  (float)1/6
#define C5  (float)0

const float coefficient_A[4][4] = {
  {C0, C1, C3, C5},
  {C1, C3, C2, C5},
  {C3, C2, C4, C5},
  {C5, C5, C5, C5}
};

const float coefficient_B[4][4] = {
  {C5, C3, C1, C0},
  {C5, C2, C3, C1},
  {C5, C4, C2, C3},
  {C5, C5, C5, C5}
};

const float coefficient_C[4][4] = {
  {C5, C5, C5, C5},
  {C3, C2, C4, C5},
  {C1, C3, C2, C5},
  {C0, C1, C3, C5}
};

const float coefficient_D[4][4] = {
  {C5, C5, C5, C5},
  {C5, C4, C2, C3},
  {C5, C2, C3, C1},
  {C5, C3, C1, C0}
};

inline void bilinear_interp(const image_t* outputFrame, const image_t* inputFrame);

#endif /*__INTERP_H__*/
