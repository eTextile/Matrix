/*
  FORKED FROM https://github.com/openmv/openmv/tree/master/src/omv/img
  Added custom blob détection algorithm to keep track of the blobs ID's
    This patch is part of the eTextile-matrix-sensor project - http://matrix.eTextile.org
    Copyright (c) 2014-2018 Maurin Donneaud <maurin@etextile.org>
    This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#ifndef __BLOB_H__
#define __BLOB_H__

#include "config.h"
#include "llist.h"

#define IM_LOG2_2(x)    (((x) &                0x2ULL) ? ( 2                        ) :             1) // NO ({ ... }) !
#define IM_LOG2_4(x)    (((x) &                0xCULL) ? ( 2 +  IM_LOG2_2((x) >>  2)) :  IM_LOG2_2(x)) // NO ({ ... }) !
#define IM_LOG2_8(x)    (((x) &               0xF0ULL) ? ( 4 +  IM_LOG2_4((x) >>  4)) :  IM_LOG2_4(x)) // NO ({ ... }) !
#define IM_LOG2_16(x)   (((x) &             0xFF00ULL) ? ( 8 +  IM_LOG2_8((x) >>  8)) :  IM_LOG2_8(x)) // NO ({ ... }) !
#define IM_LOG2_32(x)   (((x) &         0xFFFF0000ULL) ? (16 + IM_LOG2_16((x) >> 16)) : IM_LOG2_16(x)) // NO ({ ... }) !
#define IM_LOG2(x)      (((x) & 0xFFFFFFFF00000000ULL) ? (32 + IM_LOG2_32((x) >> 32)) : IM_LOG2_32(x)) // NO ({ ... }) !

#define CHAR_BITS (sizeof(char) * 8)
#define CHAR_MASK (CHAR_BITS - 1)
#define CHAR_SHIFT IM_LOG2(CHAR_MASK)

#define ROW_PTR(pImage, y) \
  ({ \
    __typeof__ (pImage) _pImage = (pImage); \
    __typeof__ (y) _y = (y); \
    ((uint8_t*)_pImage->pData) + (_pImage->numCols * _y); \
  })

#define GET_PIXEL(pRow, x) \
  ({ \
    __typeof__ (pRow) _pRow = (pRow); \
    __typeof__ (x) _x = (x); \
    _pRow[_x]; \
  })

#define ROW_INDEX(pImage, y) \
  ({ \
    __typeof__ (pImage) _pImage = (pImage); \
    __typeof__ (y) _y = (y); \
    _pImage->numCols * _y; \
  })

#define BITMAP_INDEX(rowIndex, x) \
  ({ \
    __typeof__ (rowIndex) _rowIndex = (rowIndex); \
    __typeof__ (x) _x = (x); \
    _rowIndex + _x; \
  })

#define PIXEL_THRESHOLD(pixel, pThreshold) \
  ({ \
    __typeof__ (pixel) _pixel = (pixel); \
    __typeof__ (pThreshold) _pThreshold = (pThreshold); \
    _pixel >= _pThreshold; \
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

////////////// Image stuff //////////////

typedef struct image {
  uint8_t numCols;
  uint8_t numRows;
  uint8_t* pData;
} image_t;

void bitmap_bit_set(char* bitmap_ptr, uint16_t index);
char bitmap_bit_get(char* bitmap_ptr, uint16_t index);
void bitmap_clear(char* bitmap_ptr, const uint16_t Size);

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

// TODO add an ALIVE message
// What about the TUIO 1.1 Protocol Specification
// http://www.tuio.org/?specification
typedef struct blob {
  uint8_t UID;
  uint8_t alive;
  state_t state;
  point_t centroid;
  bbox_t box;
  uint16_t pixels;
  struct blob* next_ptr;
} blob_t;

inline void blob_raz(blob_t* node);
inline void blob_copy(blob_t* dst, blob_t* src);

typedef struct llist llist_t; // forward declaration

void find_blobs(
  image_t* interpolatedFrame_ptr,
  char* bitmap_ptr,
  uint8_t E256_threshold,
  const unsigned int minBlobPix,
  const unsigned int maxBlobPix,
  llist_t* freeBlobs_ptr,
  llist_t* inputBlobs_ptr,
  llist_t* outputBlobs_ptr
);

#endif /*__BLOB_H__*/
