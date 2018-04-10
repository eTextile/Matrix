/*
  This file is part of the eTextile-matrix-sensor project - http://matrix.eTextile.org
  Copyright (c) 2014-2018 Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "config.h"

/**
    Bilinear interpolation
    param[in,out] S  points to an instance of the interpolation structure
    param[in]     X  interpolation coordinate
    param[in]     Y  interpolation coordinate
    return out interpolated value
*/

inline uint8_t bilinear_interp(const image_t* inputFrame, int posX, int posY) {
  
  uint8_t  xIndex, yIndex, index;
  uint8_t  f00, f01, f10, f11;
  uint8_t* pData = inputFrame->pData;
  float    xdiff, ydiff;
  uint8_t  b1, b2, b3, b4;
  uint8_t  out;

  xIndex =  posX >> 2; // 0 0 0 0, 1 1 1 1, 2 2 2 2... WARNING - If SCALE != 4
  yIndex =  posY >> 2; // 0 0 0 0, 1 1 1 1, 2 2 2 2... WARNING - If SCALE != 4

  // Index computation for two nearest points in X-direction
  index = (xIndex - 1) + (yIndex - 1) * inputFrame->numCols;
  // Read two nearest points in X-direction
  f00 = pData[index];
  f01 = pData[index + 1];

  // Index computation for two nearest points in Y-direction
  index = (xIndex - 1) + (yIndex) * inputFrame->numCols;
  // Read two nearest points in Y-direction
  f10 = pData[index];
  f11 = pData[index + 1];

  // Calculation of intermediate values
  b1 = f00;
  b2 = f01 - f00;
  b3 = f10 - f00;
  b4 = f00 - f01 - f10 + f11;

  // Calculation of fractional part in X
  // xdiff = posX - xIndex;
  xdiff = (posX / SCALE) - xIndex;

  // Calculation of fractional part in Y
  // ydiff = posY - yIndex;
  ydiff = (posY / SCALE) - yIndex;

  // Calculation of bi-linear interpolated output
  out = b1 + b2 * xdiff + b3 * ydiff + b4 * xdiff * ydiff;

  // return to application
  return (out);
}
