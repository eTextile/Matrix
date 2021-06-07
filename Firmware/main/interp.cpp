/*
  This file is part of the E256 - eTextile matrix sensor project - http://matrix.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "interp.h"

uint8_t interpFrameArray[NEW_FRAME] = {0};  // 1D Array to store E256 bilinear interpolated values
interp_t interp;                            // Interpolation parameters structure

float coef_A[SCALE_X * SCALE_Y] = {0};
float coef_B[SCALE_X * SCALE_Y] = {0};
float coef_C[SCALE_X * SCALE_Y] = {0};
float coef_D[SCALE_X * SCALE_Y] = {0};

uint8_t interpThreshold = 5;

/*
    Bilinear interpolation
    Pre-compute the four coefficient values for all interpolated output matrix positions
*/

void INTERP_SETUP(image_t* outputFrame_ptr) {

  // image_t* outputFrame_ptr init config
  outputFrame_ptr->pData = &interpFrameArray[0];  // Setup -> uint8_t bilinInterpOutput[NEW_FRAME] (64x64)
  outputFrame_ptr->numCols = NEW_COLS;
  outputFrame_ptr->numRows = NEW_ROWS;

  // interp_t* interp init config
  interp.outputStrideY = SCALE_X * SCALE_Y * RAW_COLS;
  interp.pCoefA = &coef_A[0];
  interp.pCoefB = &coef_B[0];
  interp.pCoefC = &coef_C[0];
  interp.pCoefD = &coef_D[0];

  float sFactor = SCALE_X * SCALE_Y;

  for (uint8_t row = 0; row < SCALE_Y; row++) {
    for (uint8_t col = 0; col < SCALE_X; col++) {
      int index = row * SCALE_Y + col;
      interp.pCoefA[index] = (SCALE_X - col) * (SCALE_Y - row) / sFactor;
      interp.pCoefB[index] = col * (SCALE_Y - row) / sFactor;
      interp.pCoefC[index] = (SCALE_X - col) * row / sFactor;
      interp.pCoefD[index] = row * col / sFactor;
    };
  };
};

// Bilinear interpolation
void interp_matrix(image_t* inputFrame_ptr) {

  // Clear interpFrameArray
  memset((uint8_t*)interpFrameArray, 0, SIZEOF_FRAME);

  for (uint8_t rowPos = 0; rowPos < (RAW_ROWS - 1); rowPos++) {
    uint8_t* row_ptr = COMPUTE_IMAGE_ROW_PTR(inputFrame_ptr, rowPos);
    for (uint8_t colPos = 0; colPos < (RAW_COLS - 1); colPos++) {
      if (IMAGE_GET_PIXEL_FAST(row_ptr, colPos) > interpThreshold) { // 'Windowing' interpolation

        uint8_t inIndexA = rowPos * RAW_COLS + colPos;
        uint8_t inIndexB = inIndexA + 1;
        uint8_t inIndexC = inIndexA + RAW_COLS;
        uint8_t inIndexD = inIndexC + 1;

        for (uint8_t row = 0; row < SCALE_Y; row++) {
          for (uint8_t col = 0; col < SCALE_X; col++) {
            uint8_t coefIndex = row * SCALE_X + col;
            uint16_t outIndex = rowPos * interp.outputStrideY + colPos * SCALE_X + row * NEW_COLS + col;
            interpFrameArray[outIndex] =
              (uint8_t)round(
                inputFrame_ptr->pData[inIndexA] * interp.pCoefA[coefIndex] +
                inputFrame_ptr->pData[inIndexB] * interp.pCoefB[coefIndex] +
                inputFrame_ptr->pData[inIndexC] * interp.pCoefC[coefIndex] +
                inputFrame_ptr->pData[inIndexD] * interp.pCoefD[coefIndex]
              );
          };
        };
      };
    };
  };

#if DEBUG_INTERP
  for (uint8_t posY = 0; posY < NEW_ROWS; posY++) {
    uint8_t* row_ptr = COMPUTE_IMAGE_ROW_PTR(frame_ptr, posY);
    for (int posX = 0; posX < NEW_COLS; posX++) {
      Serial.printf("%d-", IMAGE_GET_PIXEL_FAST(row_ptr, posX));
    };
    Serial.printf("\n");
  };
  Serial.printf("\n\n");
#endif

};
