/* This file is part of the OpenMV project.
   Copyright (c) 2013-2017 Ibrahim Abdelkader <iabdalkader@openmv.io> & Kwabena W. Agyeman <kwagyeman@openmv.io>
   This work is licensed under the MIT license, see the file LICENSE for details.
*/
#ifndef __BLOB_H__
#define __BLOB_H__

#include "fmath.h"
#include "collections.h"

#define IM_MAX(a,b) ({ __typeof__ (a) _a = (a); __typeof__ (b) _b = (b); _a > _b ? _a : _b; })
#define IM_MIN(a,b) ({ __typeof__ (a) _a = (a); __typeof__ (b) _b = (b); _a < _b ? _a : _b; })

typedef struct point {
  int16_t x;
  int16_t y;
} point_t;

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

typedef struct thresholds {
  uint8_t LMin;
  uint8_t LMax;
} thresholds_t;


#define GRAYSCALE_THRESHOLD(pixel, thresholds, invert) \
  ({ \
    __typeof__(pixel) _pixel = (pixel); \
    __typeof__(*thresholds) _thresholds = (*thresholds); \
    __typeof__(invert) _invert = (invert); \
    ((_thresholds->LMin <= _pixel) && (_pixel <= _thresholds->LMax)) ^ _invert; \
  })

////////////// Image stuff //////////////

typedef struct image {
  uint8_t w;
  uint8_t h;
  union {
    uint8_t *pixels;
    uint8_t *data;
  };
} image_t;

#define IMAGE_GET_GRAYSCALE_PIXEL(image, x, y) \
  ({ \
    __typeof__ (image) _image = (image); \
    __typeof__ (x) _x = (x); \
    __typeof__ (y) _y = (y); \
    ((uint8_t *) _image->data)[(_image->w * _y) + _x]; \
  })

////////////// Fast stuff //////////////

#define IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(image, y) \
  ({ \
    __typeof__ (image) _image = (image); \
    __typeof__ (y) _y = (y); \
    ((uint8_t *) _image->data) + (_image->w * _y); \
  })

#define IMAGE_GET_GRAYSCALE_PIXEL_FAST(row_ptr, x) \
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
  // float rotation;
  uint16_t code, count;
} find_blobs_list_lnk_data_t;

void find_blobs(
  list_t *out,
  image_t *ptr,
  rectangle_t *roi,
  thresholds_t *thresholds,
  bool invert,
  unsigned int area_threshold,
  unsigned int pixels_threshold,
  bool merge,
  int margin
  // bool (*threshold_cb)(void*, find_blobs_list_lnk_data_t*), void *threshold_cb_arg,
  // bool (*merge_cb)(void*, find_blobs_list_lnk_data_t*, find_blobs_list_lnk_data_t*), void *merge_cb_arg
);

#endif // __BLOB_H__
