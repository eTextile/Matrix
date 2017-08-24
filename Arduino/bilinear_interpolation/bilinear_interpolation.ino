// https://www.keil.com/pack/doc/CMSIS/DSP/html/group__BilinearInterpolate.html

#include <arm_math.h>

#define  COLS                 16
#define  ROWS                 16
#define  SCALE                4
#define  ROW_FRAME            COLS*ROWS
#define  NEW_FRAME            COLS*ROWS*SCALE

float32_t inputVals[ROW_FRAME] = {0};      // Array to store row input values
float32_t bilinIntOutput[NEW_FRAME] = {0}; // Output buffer calculated from bilinear interpolation

arm_bilinear_interp_instance_f32 S;

void setup() {
  S.numRows = ROWS;
  S.numCols = COLS;
  S.pData = &inputVals[0];
}

void loop() {

  float32_t posX, posY;
  int pos = 0;

  for (posX = 0; posX < ROWS; posX += .25) {
    for (posY = 0; posY < COLS; posY += .25) {
      bilinIntOutput[pos++] = arm_bilinear_interp_f32(&S, posX, posY);
    }
  }
  // pos = 0;
}

