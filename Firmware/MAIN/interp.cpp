/*
  This file is part of the eTextile-matrix-sensor project - http://matrix.eTextile.org
  Copyright (c) 2014-2018 Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "config.h"

// Pre-compute the four coefficient values for all interpolated matrix positions
void bilinear_interp_init(interp_t* interp) {

  float sFactor = interp->numCols * interp->numRows;

  for (uint8_t row = 0; row < interp->numRows; row++) {
    for (uint8_t col = 0; col < interp->numCols; col++) {
      interp->pCoefA[row * interp->numCols + col] = (interp->numCols - col) * (interp->numRows - row) / sFactor;
      interp->pCoefB[row * interp->numCols + col] = col * (interp->numRows - row) / sFactor;
      interp->pCoefC[row * interp->numCols + col] = (interp->numCols - col) *  row / sFactor;
      interp->pCoefD[row * interp->numCols + col] = row * col / sFactor;
    }
  }
}

/*
    Bilinear interpolation

    param[IN]      inputFrame   // Points to an instance of an image_t structure
    param[OUT]     outputFrame  // Points to an instance of an image_t structure
*/
inline void bilinear_interp(const image_t* outputFrame, const image_t* inputFrame) {

  for (uint8_t rowPos = 0; rowPos < ROWS; rowPos++) {
    for (uint8_t colPos = 0; colPos < COLS - 1; colPos++) {

      uint8_t indexA = rowPos * COLS + colPos;
      uint8_t indexB = indexA + 1;
      uint8_t indexC = indexA + COLS;
      uint8_t indexD = indexC + 1;

      for (int row = 0; row < X_SCALE; row++) {
        for (int col = 0; col < Y_SCALE; col++) {

          int outIndex = indexA * X_SCALE + row * NEW_COLS + col;

          outputFrame->pData[outIndex] =
            (uint8_t) round(
              inputFrame->pData[indexA] * interp.pCoefA[row * X_SCALE + col] +
              inputFrame->pData[indexB] * interp.pCoefB[row * X_SCALE + col] +
              inputFrame->pData[indexC] * interp.pCoefC[row * X_SCALE + col] +
              inputFrame->pData[indexD] * interp.pCoefD[row * X_SCALE + col]
            );
        }
      }
    }
  }
}

