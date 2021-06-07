/*
  median filter (c) 2019 by RainerU
  This code is under the MIT License
  https://github.com/RainerU/medianfilter

  Added multy blob filtering capability
  Copyright (c) 2021 Maurin Donneaud <maurin@etextile.org>
*/

#ifndef __MEDIAN_H__
#define __MEDIAN_H__

#include "config.h"
#include "llist.h"
#include "blob.h"

typedef struct llist llist_t;  // Forward declaration
typedef struct blob blob_t;    // Forward declaration

#define MEDIAN_WINDOW 5                  // Allowed filter window size : 3, 5, 7...
#define MEDIAN_POS ((MEDIAN_WINDOW-1)/2) // position of median in ordered list

typedef struct median median_t;
struct median {
  float zVal[MEDIAN_WINDOW];  // Input values Array
  float zSort[MEDIAN_WINDOW]; // Sorted values Array
  uint8_t index;              // Ring storage current index
};

void median(llist_t* blobs_ptr);

#endif /*__MEDIAN_H__*/
