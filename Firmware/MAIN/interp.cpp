/*
  This file is part of the eTextile-matrix-sensor project - http://matrix.eTextile.org
  Copyright (c) 2014-2018 Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "config.h"

/*
    Bilinear interpolation

    param[in]      inputFrame   // Points to an instance of an image_t structure
    param[OUT]     outputFrame  // Points to an instance of an image_t structure
*/

inline void bilinear_interp(const image_t* outputFrame, const image_t* inputFrame) {

  for (uint8_t rowPos = 0; rowPos < ROWS; rowPos = rowPos + 2) {
    for (uint8_t colPos = 0; colPos < COLS; colPos = colPos + 2) {

      uint8_t indexA = rowPos * ROWS + colPos;
      uint8_t indexB = indexA + 1;
      uint8_t indexC = indexA + COLS;
      uint8_t indexD = indexC + 1;

      Serial.printf(F("\n\tQ00=%d\tQ01%d\tQ10=%d\tQ11=%d"), indexA, indexB, indexC, indexD);

      for (uint8_t row = 0; row < 4; row++) {
        for (uint8_t col = 0; col < 4; col++) {

          uint16_t index = indexA + col + row * COLS; // FIXME
          Serial.printf(F("\n%d"), index);

          outputFrame->pData[index] =
            inputFrame->pData[indexA] * coefficient_A[row][col] +
            inputFrame->pData[indexB] * coefficient_B[row][col] +
            inputFrame->pData[indexC] * coefficient_C[row][col] +
            inputFrame->pData[indexD] * coefficient_D[row][col];
        }
      }
    }
  }

}


