// https://github.com/ARM-software/CMSIS/blob/master/CMSIS/Include/arm_math.h
// https://www.keil.com/pack/doc/CMSIS/DSP/html/arm_linear_interp_example_f32_8c-example.html

#include <arm_math.h>

#define  COLS                 16
#define  ROWS                 16
#define  SCALE                4
#define  ROW_FRAME            COLS*ROWS
#define  NEW_FRAME            COLS*ROWS*SCALE

int16_t inputVals[ROW_FRAME] = {0};                           // Array to store input values
float32_t bilinIntOutput[ROW_FRAME] = {0};                    // Output buffer calculated from bilinear interpolation
extern float32_t arm_bilinear_interep_table[NEW_FRAME] = {0}; // External table used for bilinear interpolation

arm_bilinear_interp_instance_f32 S;

void setup() {
  S.numRows = ROWS * SCALE;
  S.numCols = COLS * SCALE;
  S.pData = &arm_bilinear_interep_table[0];
}

void loop() {

  uint32_t posX, posY;

  for (posX = 0; posX < ROWS; posX++) {
    for (posY = 0; posY < COLS; posY++) {
      bilinIntOutput[posX + posY*COLS] = arm_bilinear_interp_f32(&S, posX, posY );
    }
  }
}
