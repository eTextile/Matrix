/* This file is part of the OpenMV project.
   Copyright (c) 2013-2017 Ibrahim Abdelkader <iabdalkader@openmv.io> & Kwabena W. Agyeman <kwagyeman@openmv.io>
   This work is licensed under the MIT license, see the file LICENSE for details.
*/

#ifndef __BLOB_H__
#define __BLOB_H__

#include <Arduino.h>
#include "collections.h"

#define IM_MAX(a,b) \
  ({ \
    __typeof__ (a) _a = (a); \
    __typeof__ (b) _b = (b); \
    _a > _b ? _a : _b; \
  })

#define IM_MIN(a,b) \
  ({ \
    __typeof__ (a) _a = (a); \
    __typeof__ (b) _b = (b); \
    _a < _b ? _a : _b; \
  })

typedef struct point {
  int16_t x;
  int16_t y;
  int16_t z;
  int16_t dist;
} point_t;

typedef struct xylf {
  int16_t x, y, l, r;
} xylf_t;

////////////// Threshold stuff //////////////

#define PIXEL_THRESHOLD(pixel, pThreshold) \
  ({ \
    __typeof__ (pixel) _pixel = (pixel); \
    __typeof__ (pThreshold) _pThreshold = (pThreshold); \
    _pThreshold <= _pixel; \
  })

////////////// Image stuff //////////////

typedef struct image {
  int w;
  int h;
  uint8_t* dataPtr;
} image_t;

////////////// Fast stuff //////////////

#define FRAME_ROW_PTR(imagePtr, y) \
  ({ \
    __typeof__ (imagePtr) _imagePtr = (imagePtr); \
    __typeof__ (y) _y = (y); \
    ((uint8_t*)_imagePtr->dataPtr) + (_imagePtr->w * _y); \
  })

#define GET_FRAME_PIXEL(rowPtr, x) \
  ({ \
    __typeof__ (rowPtr) _rowPtr = (rowPtr); \
    __typeof__ (x) _x = (x); \
    _rowPtr[_x]; \
  })

void print_frame_pixels(uint8_t* rowPtr);

////////////// Blob tracking //////////////

typedef struct blob {
  uint8_t UID;
  point_t centroid;
  uint32_t pixels;
  boolean isDead;
} blob_t;

void find_blobs(
  const image_t* input_ptr,
  char* bitmap_ptr,
  const int rows,
  const int cols,
  const int pixelThreshold,
  const unsigned int minBlobPix,
  const unsigned int  maxBlobPix,
  list_t* freeNodeList_ptr,
  list_t* nodes_ptr,
  list_t* oldNodesToUpdate_ptr,
  list_t* nodesToUpdate_ptr,
  list_t* nodesToAdd_ptr,
  list_t* outputNodes_ptr
);

#endif /*__BLOB_H__*/
