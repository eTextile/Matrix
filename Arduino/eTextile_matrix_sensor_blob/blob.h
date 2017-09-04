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

////////////// Rectangle Stuff //////////////

typedef struct rectangle {
  int16_t x;
  int16_t y;
  int16_t w;
  int16_t h;
} rectangle_t;

bool rectangle_overlap(rectangle_t *ptr0, rectangle_t *ptr1);
void rectangle_united(rectangle_t *dst, rectangle_t *src);

////////////// Color Stuff //////////////

typedef struct color_thresholds_list_lnk_data {
  uint8_t LMin, LMax; // or grayscale
  int8_t AMin, AMax;
  int8_t BMin, BMax;
} color_thresholds_list_lnk_data_t;

#define COLOR_THRESHOLD_GRAYSCALE(pixel, threshold, invert) \
  ({ \
    __typeof__ (pixel) _pixel = (pixel); \
    __typeof__ (threshold) _threshold = (threshold); \
    __typeof__ (invert) _invert = (invert); \
    ((_threshold->LMin <= _pixel) && (_pixel <= _threshold->LMax)) ^ _invert; \
  })

#define COLOR_GRAYSCALE_MIN 0
#define COLOR_GRAYSCALE_MAX 255

////////////// Image Stuff //////////////

typedef struct image {
  int w;
  int h;
  // int bpp;
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

////////////// Fast Stuff //////////////

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

////////////// Blob Tracking //////////////

typedef struct find_blobs_list_lnk_data {
  rectangle_t rect;
  uint32_t pixels;
  point_t centroid;
  // float rotation;
  uint16_t code, count;
} find_blobs_list_lnk_data_t;

void find_blobs(
  list_t *out, image_t *ptr, rectangle_t *roi,
  // unsigned int x_stride, unsigned int y_stride,
  list_t *thresholds, bool invert, unsigned int area_threshold, unsigned int pixels_threshold,
  bool merge, int margin
  // bool (*threshold_cb)(void*, find_blobs_list_lnk_data_t*), void *threshold_cb_arg,
  // bool (*merge_cb)(void*, find_blobs_list_lnk_data_t*, find_blobs_list_lnk_data_t*), void *merge_cb_arg
);

#endif // __BLOB_H__
