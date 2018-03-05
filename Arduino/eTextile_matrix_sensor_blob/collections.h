/*
   This file is part of the OpenMV project - https://github.com/openmv/openmv
   Copyright (c) 2013-2017 Ibrahim Abdelkader <iabdalkader@openmv.io> & Kwabena W. Agyeman <kwagyeman@openmv.io>
   This work is licensed under the MIT license, see the file LICENSE for details.

   This file has been modified to fit the eTextile matrix sensor needs
   eTextile matrix sensor - http://matrix.eTextile.org
*/

#ifndef __COLLECTIONS_H__
#define __COLLECTIONS_H__

#define IM_LOG2_2(x)    (((x) &                0x2ULL) ? ( 2                        ) :             1) // NO ({ ... }) !
#define IM_LOG2_4(x)    (((x) &                0xCULL) ? ( 2 +  IM_LOG2_2((x) >>  2)) :  IM_LOG2_2(x)) // NO ({ ... }) !
#define IM_LOG2_8(x)    (((x) &               0xF0ULL) ? ( 4 +  IM_LOG2_4((x) >>  4)) :  IM_LOG2_4(x)) // NO ({ ... }) !
#define IM_LOG2_16(x)   (((x) &             0xFF00ULL) ? ( 8 +  IM_LOG2_8((x) >>  8)) :  IM_LOG2_8(x)) // NO ({ ... }) !
#define IM_LOG2_32(x)   (((x) &         0xFFFF0000ULL) ? (16 + IM_LOG2_16((x) >> 16)) : IM_LOG2_16(x)) // NO ({ ... }) !
#define IM_LOG2(x)      (((x) & 0xFFFFFFFF00000000ULL) ? (32 + IM_LOG2_32((x) >> 32)) : IM_LOG2_32(x)) // NO ({ ... }) !

#define CHAR_BITS (sizeof(char) * 8)
#define CHAR_MASK (CHAR_BITS - 1)
#define CHAR_SHIFT IM_LOG2(CHAR_MASK)


#define ROW_PTR(imagePtr, y) \
  ({ \
    __typeof__ (imagePtr) _imagePtr = (imagePtr); \
    __typeof__ (y) _y = (y); \
    ((uint8_t*)_imagePtr->dataPtr) + (_imagePtr->w * _y); \
  })

#define GET_PIXEL(rowPtr, x) \
  ({ \
    __typeof__ (rowPtr) _rowPtr = (rowPtr); \
    __typeof__ (x) _x = (x); \
    _rowPtr[_x]; \
  })

#define ROW_INDEX(imagePtr, y) \
  ({ \
    __typeof__ (imagePtr) _imagePtr = (imagePtr); \
    __typeof__ (y) _y = (y); \
    _imagePtr->w * _y; \
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
    _pThreshold <= _pixel; \
  })

#define IM_MAX(a, b) \
  ({ \
    __typeof__ (a) _a = (a); \
    __typeof__ (b) _b = (b); \
    _a > _b ? _a : _b; \
  })

#define IM_MIN(a, b) \
  ({ \
    __typeof__ (a) _a = (a); \
    __typeof__ (b) _b = (b); \
    _a < _b ? _a : _b; \
  })

////////////// Lifo //////////////

typedef struct xylf {
  uint16_t x, y, l, r;
  struct xylf* prev_ptr;
} xylf_t;

typedef struct lifo {
  xylf_t* head_ptr;
  uint16_t buffer_size;
  int16_t index; // If no element index is -1
} lifo_t;

void lifo_raz(lifo_t* ptr);
void lifo_init(lifo_t* dst, xylf_t* node, uint16_t max_nodes); // uint8_t node_size
void lifo_enqueue(lifo_t* dst, xylf_t* node);
xylf_t* lifo_dequeue(lifo_t* src);
int16_t lifo_size(lifo_t *ptr);

////////////// Image stuff //////////////

typedef struct image {
  uint8_t w;
  uint8_t h;
  uint8_t* dataPtr;
} image_t;

void bitmap_bit_set(char* array_ptr, int index);
char bitmap_bit_get(char* array_ptr, int index);
void bitmap_clear(char* array_ptr);

////////////// Blob //////////////

typedef struct point {
  uint8_t x;
  uint8_t y;
  uint8_t z;
} point_t;

typedef struct blob {
  int8_t UID; // If no ID, UID is -1
  point_t centroid;
  uint16_t pixels;
  boolean isDead;
  struct blob* next_ptr;
} blob_t;

typedef struct {
  blob_t* head_ptr;
  blob_t* tail_ptr;
  uint8_t buffer_size;
  int8_t index; // If no element index is -1
} llist_t;

////////////// Linked list - Fonction prototypes //////////////

////////////// Iterators //////////////
int8_t list_size(llist_t* ptr);
blob_t* iterator_start_from_head(llist_t* src);
blob_t* iterator_next(blob_t* src);

void llist_raz(llist_t* ptr);
void llist_init(llist_t* dst, blob_t* node, uint8_t buffer_size); // uint8_t buffer_size
blob_t* llist_pop_front(llist_t* src);
void llist_push_back(llist_t* dst, blob_t* blob);
void llist_save_blobs(llist_t* dst, llist_t* src);
void llist_copy_blob(blob_t* dst, blob_t* src);
void llist_remove_blob(llist_t* src, blob_t* blob);
int16_t llist_size(llist_t *ptr);

void find_blobs(
  image_t* input_ptr,
  char* bitmap_ptr,
  const int rows,
  const int cols,
  const int pixelThreshold,
  const unsigned int minBlobPix,
  const unsigned int maxBlobPix,
  lifo_t*  freeNodes_ptr,
  lifo_t*  lifo_ptr,
  llist_t* freeBlob_ptr,
  llist_t* blobs_ptr,
  llist_t* blobsToUpdate_ptr,
  llist_t* blobsToAdd_ptr,
  llist_t* outputBlobs_ptr
);

#endif /*__COLLECTIONS_H__*/
