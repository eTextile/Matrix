/* This file is part of the OpenMV project.
   Copyright (c) 2013-2017 Ibrahim Abdelkader <iabdalkader@openmv.io> & Kwabena W. Agyeman <kwagyeman@openmv.io>
   This work is licensed under the MIT license, see the file LICENSE for details.
*/

#ifndef __BLOB_H__
#define __BLOB_H__

#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <arm_math.h>

#include "fb_alloc.h"
#include "xalloc.h"
#include "fmath.h"
#include "collections.h"

#define IM_MAX(a,b) ({ __typeof__ (a) _a = (a); __typeof__ (b) _b = (b); _a > _b ? _a : _b; })
#define IM_MIN(a,b) ({ __typeof__ (a) _a = (a); __typeof__ (b) _b = (b); _a < _b ? _a : _b; })

typedef struct point {
  int16_t x;
  int16_t y;
} point_t;

typedef struct xylf {
  int16_t x, y, l, r;
} xylf_t;

////////////// Rectangle stuff //////////////

typedef struct rectangle {
  int16_t x;
  int16_t y;
  int16_t w;
  int16_t h;
} rectangle_t;

bool rectangle_overlap(rectangle_t *ptr0, rectangle_t *ptr1);
void rectangle_united(rectangle_t *dst, rectangle_t *src);

////////////// Threshold stuff //////////////

#define GRAYSCALE_THRESHOLD(pixel, pThreshold) \
  ({ \
    __typeof__(pixel) _pixel = (pixel); \
    __typeof__(pThreshold) _pThreshold = (pThreshold); \
    (_pThreshold <= _pixel); \
  })

////////////// Image stuff //////////////

typedef struct image {
  uint8_t w;
  uint8_t h;
  uint16_t *data;
} image_t;

////////////// Fast stuff //////////////

#define IMAGE_COMPUTE_ROW_PTR(image, y) \
  ({ \
    __typeof__ (image) _image = (image); \
    __typeof__ (y) _y = (y); \
    ((uint8_t *) _image->data) + (_image->w * _y); \
  })

#define IMAGE_GET_PIXEL_FAST(row_ptr, x) \
  ({ \
    __typeof__ (row_ptr) _row_ptr = (row_ptr); \
    __typeof__ (x) _x = (x); \
    _row_ptr[_x]; \
  })

////////////// Blob tracking //////////////

typedef struct find_blobs_list_lnk_data {
  rectangle_t rect;
  uint32_t pixels;
  point_t centroid;
  uint16_t code, count;
} find_blobs_list_lnk_data_t;

void find_blobs(
  list_t *out,
  image_t *ptr,
  rectangle_t *roi,
  unsigned int pixelThreshold,
  unsigned int minBlobSize,
  unsigned int minBlobPix,
  bool merge,
  int margin
);

#endif /* __BLOB_H__ */
