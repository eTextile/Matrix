/*
  This file is part of the eTextile-matrix-sensor project - http://matrix.eTextile.org
  Copyright (c) 2014-2019 Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "interp.h"

/*
    Bilinear interpolation
    Pre-compute the four coefficient values for all interpolated output matrix positions
*/

void bilinear_interp_init(const interp_t* interp) {

  float sFactor = interp->scale_X * interp->scale_Y;

  for (uint8_t row = 0; row < interp->scale_Y; row++) {
    for (uint8_t col = 0; col < interp->scale_X; col++) {
      int index = row * interp->scale_X + col;
      interp->pCoefA[ index ] = (interp->scale_X - col) * (interp->scale_Y - row) / sFactor;
      interp->pCoefB[ index ] = col * (interp->scale_Y - row) / sFactor;
      interp->pCoefC[ index ] = (interp->scale_X - col) *  row / sFactor;
      interp->pCoefD[ index ] = row * col / sFactor;
    }
  }
}

/*
    Bilinear interpolation
    param[IN]      inputFrame   // Points to an instance of an image_t structure
    param[OUT]     outputFrame  // Points to an instance of an image_t structure
*/

void bilinear_interp(const image_t *outputFrame, const image_t* inputFrame, const interp_t* interp) {

  for (uint8_t rowPos = 0; rowPos < inputFrame->numRows; rowPos++) {
    for (uint8_t colPos = 0; colPos < inputFrame->numCols - 1; colPos++) {

      uint8_t inIndexA = rowPos * inputFrame->numCols + colPos;
      uint8_t inIndexB = inIndexA + 1;
      uint8_t inIndexC = inIndexA + inputFrame->numCols;
      uint8_t inIndexD = inIndexC + 1;

      // TODO: windowing implementation

      for (uint8_t row = 0; row < interp->scale_Y; row++) {
        for (uint8_t col = 0; col < interp->scale_X; col++) {

          uint8_t coefIndex = row * interp->scale_X + col;
          uint16_t outIndex = rowPos * interp->outputStride_Y + colPos * interp->scale_X + row * outputFrame->numCols + col;

          outputFrame->pData[outIndex] =
            (uint8_t) round(
              inputFrame->pData[inIndexA] * interp->pCoefA[coefIndex] +
              inputFrame->pData[inIndexB] * interp->pCoefB[coefIndex] +
              inputFrame->pData[inIndexC] * interp->pCoefC[coefIndex] +
              inputFrame->pData[inIndexD] * interp->pCoefD[coefIndex]
            );
        }
      }
    }
  }
}
