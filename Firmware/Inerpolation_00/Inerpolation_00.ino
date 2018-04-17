// #undef round

#define COLS           16
#define ROWS           16
#define ROW_FRAME      (COLS * ROWS)
#define X_SCALE        4
#define Y_SCALE        4
#define NEW_COLS       (COLS * X_SCALE)
#define NEW_ROWS       (ROWS * Y_SCALE)
#define NEW_FRAME      (NEW_COLS * NEW_ROWS)

uint8_t frameValues[ROW_FRAME] = {0};     // Array to store ofseted input values TODO

float coef_A[X_SCALE * Y_SCALE] = {0};
float coef_B[X_SCALE * Y_SCALE] = {0};
float coef_C[X_SCALE * Y_SCALE] = {0};
float coef_D[X_SCALE * Y_SCALE] = {0};

uint8_t bilinIntOutput[NEW_FRAME] = {0};  // Bilinear interpolation Output buffer

typedef struct {
  uint8_t numCols;
  uint8_t numRows;
  uint8_t* pData;
} image_t;

image_t   rawFrame;
image_t   interpolatedFrame;

typedef struct {
  uint8_t numCols;
  uint8_t numRows;
  float* pCoefA;
  float* pCoefB;
  float* pCoefC;
  float* pCoefD;
} interp_t;

interp_t  interp;

boolean DEBUG = false;

void setup() {

  Serial.begin(230400);

  // Raw frame init
  rawFrame.numCols = COLS;
  rawFrame.numRows = ROWS;
  rawFrame.pData = frameValues; // 16 x 16

  interp.numCols = X_SCALE;
  interp.numRows = Y_SCALE;
  interp.pCoefA = coef_A;
  interp.pCoefB = coef_B;
  interp.pCoefC = coef_C;
  interp.pCoefD = coef_D;

  // Interpolated frame init
  interpolatedFrame.numCols = NEW_COLS;
  interpolatedFrame.numRows = NEW_ROWS;
  interpolatedFrame.pData = bilinIntOutput; // 32 x 32

  while (!Serial.dtr());  // Wait for user to start the serial monitor

  bilinear_interp_init(&interp);

  for (int i = 0; i < ROW_FRAME; i++) {
    rawFrame.pData[i] = i;
    Serial.printf("%d ", rawFrame.pData[i]);
    if ((i % COLS) == (COLS - 1)) Serial.println();
    delay(1);
  }
  Serial.println("START");
}

void loop() {

  bilinear_interp(&interpolatedFrame, &rawFrame);

  for (int i = 0; i < NEW_FRAME; i++) {
    if ((i % NEW_COLS) == (NEW_COLS - 1)) Serial.println();
    Serial.printf(F(" %d"), interpolatedFrame.pData[i]);
  }
  Serial.println();

  while (1);
}

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

  for (uint8_t i = 0; i < sFactor; i++) {
    Serial.printf(F("Ax%f\tBx%f\tCx%f\tDx%f\n"), interp->pCoefA[i], interp->pCoefB[i], interp->pCoefC[i], interp->pCoefD[i]);
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

      Serial.printf(F("\nA=%d\tB=%d\tC=%d\tD=%d\n"), indexA, indexB, indexC, indexD);

      for (int row = 0; row < X_SCALE; row++) {
        for (int col = 0; col < Y_SCALE; col++) {

          int outIndex = indexA * X_SCALE + row * NEW_COLS + col;
          Serial.printf(F("%d\t"), outIndex);

          outputFrame->pData[outIndex] =
            (uint8_t) round(
              inputFrame->pData[indexA] * interp.pCoefA[row * X_SCALE + col] +
              inputFrame->pData[indexB] * interp.pCoefB[row * X_SCALE + col] +
              inputFrame->pData[indexC] * interp.pCoefC[row * X_SCALE + col] +
              inputFrame->pData[indexD] * interp.pCoefD[row * X_SCALE + col]
            );
        }
        Serial.println();
      }
      Serial.println();
    }
  }
}
