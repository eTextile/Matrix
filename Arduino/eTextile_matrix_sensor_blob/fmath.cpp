/*
   This file is part of the OpenMV project.
   Copyright (c) 2013/2014 Ibrahim Abdelkader <i.abdalkader@gmail.com>
   This work is licensed under the MIT license, see the file LICENSE for details.

   Fast approximate math functions.
*/

#include <stdint.h>

#include "fmath.h"

#define M_PI    3.14159265f
#define M_PI_2  1.57079632f
#define M_PI_4  0.78539816f

inline float fast_atanf(float xx) {
  float x, y, z;
  int sign;

  x = xx;

  /* make argument positive and save the sign */
  if ( xx < 0.0f ) {
    sign = -1;
    x = -xx;
  }
  else {
    sign = 1;
    x = xx;
  }
  /* range reduction */
  if ( x > 2.414213562373095f ) { /* tan 3pi/8 */
    y = M_PI_2;
    x = -( 1.0f / x );
  }
  else if ( x > 0.4142135623730950f ) { /* tan pi/8 */
    y = M_PI_4;
    x = (x - 1.0f) / (x + 1.0f);
  }
  else
    y = 0.0f;

  z = x * x;
  y += ((( 8.05374449538e-2f  * z - 1.38776856032E-1f) * z + 1.99777106478E-1f) * z - 3.33329491539E-1f) * z * x + x;

  if ( sign < 0 ) {
    y = -y;
  }

  return ( y );
}

float fast_atan2f(float y, float x) {
  if (x > 0 && y >= 0)
    return fast_atanf(y / x);

  if (x < 0 && y >= 0)
    return M_PI - fast_atanf(-y / x);

  if (x < 0 && y < 0)
    return M_PI + fast_atanf(y / x);

  if (x > 0 && y < 0)
    return 2 * M_PI - fast_atanf(-y / x);

  return 0;
}
