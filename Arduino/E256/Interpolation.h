
// Those arrays needs to be sorted in ascending order
// Need at least 2 points
double X [] = {0, 500, 1000, 2000, 3000, 4000, 5000, 7000};           //X axis Data for testing
double Y [] = {0, 10, 15, 20, 30, 35, 40, 45};                        //Y axis Data for testing

const int Xcount = sizeof(X) / sizeof(X[0]);
const int Ycount = sizeof(Y) / sizeof(Y[0]);

// points in 3D space (X[x], Y[y], Z[x][y])
double Z[Xcount][Ycount] = {                                        //Z Data for testing
  {70, 71, 72, 73, 74, 75, 76, 77},
  {60, 61, 62, 63, 64, 65, 66, 67},
  {50, 51, 52, 53, 54, 55, 56, 57},
  {40, 41, 42, 43, 44, 45, 46, 47},
  {30, 31, 32, 33, 34, 35, 36, 37},
  {20, 21, 22, 23, 24, 25, 26, 27},
  {10, 11, 12, 13, 14, 15, 16, 17},
  {00, 01, 02, 03, 04, 05, 06, 07}
};

double bilinearXY(int x, int y) {
  int xIndex, yIndex;

  if ((x < X[0]) || (x > X[Xcount - 1])) {
    Serial.println(F("x not in range"));
    return -1; // arbitrary...
  }

  if ((y < Y[0]) || (y > Y[Ycount - 1])) {
    Serial.println(F("y not in range"));
    return -1; // arbitrary...
  }

  for (int i = Xcount - 2; i >= 1 ; --i)
    if (x >= X[i]) {
      xIndex = i;
      break;
    }

  for (int i = Ycount - 2; i >= 1 ; --i)
    if (y >= Y[i]) {
      yIndex = i;
      break;
    }

  Serial.print(F("X:")); Serial.print(x); Serial.print(F(" in [")); Serial.print(X[xIndex]); Serial.print(F(",")); Serial.print(X[xIndex + 1]);
  Serial.print(F("] and Y:")); Serial.print(y); Serial.print(F(" in [")); Serial.print(Y[yIndex]); Serial.print(F(",")); Serial.print(Y[yIndex + 1]); Serial.println(F("]"));

  // https://en.wikipedia.org/wiki/Bilinear_interpolation
  // Q11 = (x1, y1), Q12 = (x1, y2), Q21 = (x2, y1), and Q22 = (x2, y2)
  double x1, y1, x2, y2;
  double fQ11, fQ12, fQ21, fQ22;
  double fxy1, fxy2, fxy;

  x1 = X[xIndex];
  x2 = X[xIndex + 1];
  y1 = Y[yIndex];
  y2 = Y[yIndex + 1];

  fQ11 = Z[xIndex][yIndex];
  fQ12 = Z[xIndex][yIndex + 1];
  fQ21 = Z[xIndex + 1][yIndex];
  fQ22 = Z[xIndex + 1][yIndex + 1];

  fxy1 = ((x2 - x) / (x2 - x1)) * fQ11 + ((x - x1) / (x2 - x1)) * fQ21;
  fxy2 = ((x2 - x) / (x2 - x1)) * fQ12 + ((x - x1) / (x2 - x1)) * fQ22;

  fxy = ((y2 - y) / (y2 - y1)) * fxy1 + ((y - y1) / (y2 - y1)) * fxy2;

  return fxy;

}

