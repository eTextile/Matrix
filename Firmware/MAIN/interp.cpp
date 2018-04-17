/*
  This file is part of the eTextile-matrix-sensor project - http://matrix.eTextile.org
  Copyright (c) 2014-2018 Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "config.h"

/*
    Bilinear interpolation
    Pre-compute the four coefficient values for all interpolated output matrix positions
*/

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

  for (uint8_t i = 0; i < sFactor; i++) {
    Serial.printf(F("Ax%f\tBx%f\tCx%f\tDx%f\n"), interp->pCoefA[i], interp->pCoefB[i], interp->pCoefC[i], interp->pCoefD[i]);
  }
}

/*
    Bilinear interpolation

    param[IN]      inputFrame   // Points to an instance of an image_t structure
    param[OUT]     outputFrame  // Points to an instance of an image_t structure
*/

inline void bilinear_interp(const image_t* outputFrame, const interp_t* interp, const image_t* inputFrame) {

  for (uint8_t rowPos = 0; rowPos < inputFrame->numRows; rowPos++) {
    for (uint8_t colPos = 0; colPos < inputFrame->numCols - 1; colPos++) {

      uint8_t inIndexA = rowPos * inputFrame->numCols + colPos;
      uint8_t inIndexB = inIndexA + 1;
      uint8_t inIndexC = inIndexA + inputFrame->numCols;
      uint8_t inIndexD = inIndexC + 1;

      Serial.printf(F("\nA=%d\tB=%d\tC=%d\tD=%d\n"), inIndexA, inIndexB, inIndexC, inIndexD);

      for (uint8_t row = 0; row < interp->numRows; row++) {
        for (uint8_t col = 0; col < interp->numCols; col++) {

          uint8_t coefIndex = row * interp->numCols + col;
          uint16_t outIndex = inIndexA * interp->numCols + row * outputFrame->numCols + col;

          Serial.printf(F("%d\t"), outIndex);

          outputFrame->pData[outIndex] =
            (uint8_t) round(
              inputFrame->pData[inIndexA] * interp->pCoefA[coefIndex] +
              inputFrame->pData[inIndexB] * interp->pCoefB[coefIndex] +
              inputFrame->pData[inIndexC] * interp->pCoefC[coefIndex] +
              inputFrame->pData[inIndexD] * interp->pCoefD[coefIndex]
            );
        }
        Serial.println();
      }
      Serial.println();
    }
  }
#ifdef DEBUG_INTERP
  for (uint16_t i = 0; i < NEW_FRAME; i++) {
    if ((i % outputFrame->numCols) == (outputFrame->numCols - 1)) Serial.println();
    Serial.printf(F(" %d"), bilinIntOutput[i]);
    delay(1);
  }
  Serial.println();
  delay(500);
#endif /*__DEBUG_INTERP__*/
}
