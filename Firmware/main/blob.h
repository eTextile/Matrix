/*
  FORKED FROM https://github.com/openmv/openmv/blob/master/src/omv/imlib/blob.c
    - This file is part of the OpenMV project.
    - Copyright (c) 2013- Ibrahim Abdelkader <iabdalkader@openmv.io> & Kwabena W. Agyeman <kwagyeman@openmv.io>
    - This work is licensed under the MIT license, see the file LICENSE for details.

  Added custom blob détection algorithm to keep track of the blobs ID's
    - This patch is part of the eTextile-Synthesizer project - http://synth.eTextile.org
    - Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
    - This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#ifndef __BLOB_H__
#define __BLOB_H__

#include "config.h"
#include "llist.h"

typedef struct lnode lnode_t;          // Forward declaration
typedef struct llist llist_t;          // Forward declaration

#define IM_LOG2_2(x)    (((x) &                0x2ULL) ? ( 2                        ) :             1) // NO ({ ... }) !
#define IM_LOG2_4(x)    (((x) &                0xCULL) ? ( 2 +  IM_LOG2_2((x) >>  2)) :  IM_LOG2_2(x)) // NO ({ ... }) !
#define IM_LOG2_8(x)    (((x) &               0xF0ULL) ? ( 4 +  IM_LOG2_4((x) >>  4)) :  IM_LOG2_4(x)) // NO ({ ... }) !
#define IM_LOG2_16(x)   (((x) &             0xFF00ULL) ? ( 8 +  IM_LOG2_8((x) >>  8)) :  IM_LOG2_8(x)) // NO ({ ... }) !
#define IM_LOG2_32(x)   (((x) &         0xFFFF0000ULL) ? (16 + IM_LOG2_16((x) >> 16)) : IM_LOG2_16(x)) // NO ({ ... }) !
#define IM_LOG2(x)      (((x) & 0xFFFFFFFF00000000ULL) ? (32 + IM_LOG2_32((x) >> 32)) : IM_LOG2_32(x)) // NO ({ ... }) !

#define UINT8_T_BITS    (sizeof(uint8_t) * 8)
#define UINT8_T_MASK    (UINT8_T_BITS - 1)
#define UINT8_T_SHIFT   IM_LOG2(UINT8_T_MASK)

#define SIZEOF_FRAME    (NEW_FRAME * sizeof(uint8_t))

#define COMPUTE_IMAGE_ROW_PTR(pImage, y) \
  ({ \
    __typeof__ (pImage) _pImage = (pImage); \
    __typeof__ (y) _y = (y); \
    ((uint8_t*)_pImage->pData) + (_pImage->numCols * _y); \
  })

#define COMPUTE_BINARY_IMAGE_ROW_PTR(bitmap, y) \
  ({ \
    __typeof__ (bitmap) _bitmap = (bitmap); \
    __typeof__ (y) _y = (y); \
    ((uint8_t*)_bitmap) + (((NEW_COLS + UINT8_T_MASK) >> UINT8_T_SHIFT) * _y); \
  })

#define IMAGE_GET_PIXEL_FAST(row_ptr, x) \
  ({ \
    __typeof__ (row_ptr) _row_ptr = (row_ptr); \
    __typeof__ (x) _x = (x); \
    _row_ptr[_x]; \
  })

#define IMAGE_SET_BINARY_PIXEL_FAST(row_ptr, x) \
  ({ \
    __typeof__ (row_ptr) _row_ptr = (row_ptr); \
    __typeof__ (x) _x = (x); \
    _row_ptr[_x >> UINT8_T_SHIFT] |= 1 << (_x & UINT8_T_MASK); \
  })

#define IMAGE_GET_BINARY_PIXEL_FAST(row_ptr, x) \
  ({ \
    __typeof__ (row_ptr) _row_ptr = (row_ptr); \
    __typeof__ (x) _x = (x); \
    (_row_ptr[_x >> UINT8_T_SHIFT] >> (_x & UINT8_T_MASK)) & 1; \
  })

#define PIXEL_THRESHOLD(pixel, Threshold) \
  ({ \
    __typeof__ (pixel) _pixel = (pixel); \
    __typeof__ (Threshold) _Threshold = (Threshold); \
    _pixel > _Threshold; \
  })

#define MAX(a, b) \
  ({ \
    __typeof__ (a) _a = (a); \
    __typeof__ (b) _b = (b); \
    _a > _b ? _a : _b; \
  })

#define MIN(a, b) \
  ({ \
    __typeof__ (a) _a = (a); \
    __typeof__ (b) _b = (b); \
    _a < _b ? _a : _b; \
  })

typedef struct image image_t;
struct image {
  uint8_t* pData;
  uint8_t numCols;
  uint8_t numRows;
};

typedef struct xylr xylr_t;
struct xylr {
  lnode_t node;
  uint8_t x;
  uint8_t y;
  uint8_t l;
  uint8_t r;
  uint8_t t_l;
  uint8_t b_l;
};

typedef struct point point_t;
struct point {
  float X;
  float Y;
};

typedef struct box box_t;
struct box {
  uint8_t W; // TODO Make it as float
  uint8_t H; // TODO Make it as float
  uint8_t D; // TODO Make it as float
};

typedef enum status {
  FREE,
  NOT_FOUND,
  TO_REMOVE
} status_t;

typedef struct blob blob_t;
struct blob {
  lnode_t node;
  uint8_t UID;
  status_t status;
  uint32_t timeTag;
  uint16_t pixels;
  boolean state;
  boolean lastState;
  box_t box;
  point_t centroid;
};

void lifo_llist_init(llist_t *list, xylr_t* nodesArray);
void blob_llist_init(llist_t *list, blob_t* nodesArray);

void BLOB_SETUP(llist_t* outputBlobs_ptr);
void find_blobs(uint8_t zThreshold, image_t* inputFrame_ptr, llist_t* outputBlobs_ptr);

typedef struct velocity velocity_t;
struct velocity {
  point_t lastPos;
  float vxy;
  float vz;
  float lvz;
};

void getBlobsVelocity(llist_t* blobs_ptr);

typedef struct polar polar_t;
struct polar {
  float r;
  float phi;
};

void getPolarCoordinates(llist_t* blobs_ptr);

#endif /*__BLOB_H__*/
