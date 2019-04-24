/*
  FORKED FROM https://github.com/openmv/openmv/tree/master/src/omv/img
  Added custom blob détection algorithm to keep track of the blobs ID's
    This patch is part of the eTextile-matrix-sensor project - http://matrix.eTextile.org
    Copyright (c) 2014-2018 Maurin Donneaud <maurin@etextile.org>
    This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#ifndef __BLOB_H__
#define __BLOB_H__

#include "llist.h"

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
  TO_UPDATE,
  TO_ADD,
  DEAD
} state_t;

typedef struct blob {
  uint8_t UID;
  state_t state;
  point_t centroid;
  bbox_t box;
  struct blob* next_ptr;
} blob_t;

inline void blob_raz(blob_t* node);
inline void blob_copy(blob_t* dst, blob_t* src);

typedef struct llist llist_t; // forward declaration

void find_blobs(
  image_t* interpolatedFrame_ptr,
  char* bitmap_ptr,
  const int rows,
  const int cols,
  uint8_t E256_threshold,
  const unsigned int minBlobPix,
  const unsigned int maxBlobPix,
  llist_t* freeBlobs_ptr,
  llist_t* inputBlobs_ptr,
  llist_t* outputBlobs_ptr
);

#endif /*__BLOB_H__*/
