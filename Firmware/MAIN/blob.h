/*
  FORKED FROM https://github.com/openmv/openmv/tree/master/src/omv/img
    - This file is part of the OpenMV project.
    - Copyright (c) 2013-2019 Ibrahim Abdelkader <iabdalkader@openmv.io> & Kwabena W. Agyeman <kwagyeman@openmv.io>
    - This work is licensed under the MIT license, see the file LICENSE for details.

  Added custom blob détection algorithm to keep track of the blobs ID's
    - This patch is part of the eTextile-matrix-sensor project - http://matrix.eTextile.org
    - Copyright (c) 2014-2019 Maurin Donneaud <maurin@etextile.org>
    - This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#ifndef __BLOB_H__
#define __BLOB_H__

#include "config.h"
#include "llist.h"
#include "lifo.h"

typedef struct llist llist_t; // Forward declaration
typedef struct lifo lifo_t;   // Forward declaration
typedef struct xylr xylr_t;   // Forward declaration

#define IM_LOG2_2(x)    (((x) &                0x2ULL) ? ( 2                        ) :             1) // NO ({ ... }) !
#define IM_LOG2_4(x)    (((x) &                0xCULL) ? ( 2 +  IM_LOG2_2((x) >>  2)) :  IM_LOG2_2(x)) // NO ({ ... }) !
#define IM_LOG2_8(x)    (((x) &               0xF0ULL) ? ( 4 +  IM_LOG2_4((x) >>  4)) :  IM_LOG2_4(x)) // NO ({ ... }) !
#define IM_LOG2_16(x)   (((x) &             0xFF00ULL) ? ( 8 +  IM_LOG2_8((x) >>  8)) :  IM_LOG2_8(x)) // NO ({ ... }) !
#define IM_LOG2_32(x)   (((x) &         0xFFFF0000ULL) ? (16 + IM_LOG2_16((x) >> 16)) : IM_LOG2_16(x)) // NO ({ ... }) !
#define IM_LOG2(x)      (((x) & 0xFFFFFFFF00000000ULL) ? (32 + IM_LOG2_32((x) >> 32)) : IM_LOG2_32(x)) // NO ({ ... }) !

#define UINT8_T_BITS    (sizeof(uint8_t) * 8)
#define UINT8_T_MASK    (UINT8_T_BITS - 1)
#define UINT8_T_SHIFT   IM_LOG2(UINT8_T_MASK)

#define COMPUTE_IMAGE_ROW_PTR(pImage, y) \
  ({ \
    __typeof__ (pImage) _pImage = (pImage); \
    __typeof__ (y) _y = (y); \
    ((uint8_t*)_pImage->pData) + (_pImage->numCols * _y); \
  })

#define COMPUTE_BINARY_IMAGE_ROW_PTR(pBitmap, y) \
  ({ \
    __typeof__ (pBitmap) _pBitmap = (pBitmap); \
    __typeof__ (y) _y = (y); \
    ((uint8_t*)_pBitmap->pData) + (((_pBitmap->numCols + UINT8_T_MASK) >> UINT8_T_SHIFT) * _y); \
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

//static int sum_m_to_n(int m, int n);

////////////// Image stuff //////////////

typedef struct image {
  uint8_t numCols;
  uint8_t numRows;
  uint8_t* pData;
} image_t;

void bitmap_clear(image_t* bitmap_ptr);

typedef struct {
  uint8_t X;
  uint8_t Y;
} point_t;

typedef struct {
  uint8_t W; // Width
  uint8_t H; // Height
  uint8_t D; // Depth
} bbox_t;

// Blob states
typedef enum {
  FREE,
  TO_ADD,
  TO_UPDATE,
  TO_REMOVE
} state_t;

// What about the TUIO 1.1 Protocol Specification
// http://www.tuio.org/?specification
typedef struct blob {
  uint16_t pixels;
  uint8_t UID;
  uint8_t alive;
  state_t state;
  point_t centroid;
  bbox_t box;
  struct blob* next_ptr;
} blob_t;

void blob_raz(blob_t* node);
void blob_copy(blob_t* dst, blob_t* src);

void BLOB_SETUP(
  image_t* inputFrame_ptr,
  image_t* bitmap_ptr,
  uint8_t* bitmapArray_ptr,
  lifo_t*  lifo_ptr,
  lifo_t*  lifo_stack_ptr,
  xylr_t*  lifoArray_ptr,
  llist_t* blobs_ptr,
  llist_t* blobs_stack_ptr,
  blob_t*  blobArray_ptr,
  llist_t* outputBlobs_ptr
);

void find_blobs(
  uint8_t   Threshold,
  image_t*  inputFrame_ptr,
  image_t*  bitmap_ptr,
  lifo_t*   lifo_stack_ptr,
  lifo_t*   lifo_ptr,
  llist_t*  blobs_stack_ptr,
  llist_t*  inputBlobs_ptr,
  llist_t*  outputBlobs_ptr
);

float distance(blob_t* blobA, blob_t* blobB);

#endif /*__BLOB_H__*/
