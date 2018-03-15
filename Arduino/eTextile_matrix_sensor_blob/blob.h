#ifndef __BLOB_H__
#define __BLOB_H__

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

////////////// Lifo //////////////

typedef struct xylf {
  uint8_t x, y, l, r;
  struct xylf* prev_ptr;
} xylf_t;

typedef struct {
  xylf_t* head_ptr;
  uint16_t max_nodes;
  int16_t index; // If no element index is -1
} lifo_t;

void lifo_raz(lifo_t* ptr);
void lifo_init(lifo_t* dst, xylf_t* nodesArray, const uint16_t max_nodes);
void lifo_enqueue(lifo_t* dst, xylf_t* node);
xylf_t* lifo_dequeue(lifo_t* src);
int16_t lifo_size(lifo_t *ptr);

////////////// Image stuff //////////////

typedef struct image {
  uint8_t numCols;
  uint8_t numRows;
  uint8_t* pData;
} image_t;

void bitmap_bit_set(char* bitmap_ptr, uint16_t index);
char bitmap_bit_get(char* bitmap_ptr, uint16_t index);
void bitmap_clear(char* bitmap_ptr, const uint16_t size);

////////////// Linked list - Fonction prototypes //////////////
typedef struct point {
  uint8_t X;
  uint8_t Y;
  uint8_t Z;
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
  uint8_t max_nodes;
  int8_t index; // If no element index is -1
} llist_t;

////////////// Iterators //////////////
int8_t list_size(llist_t* ptr);
blob_t* iterator_start_from_head(llist_t* src);
blob_t* iterator_next(blob_t* src);

void llist_raz(llist_t* ptr);
void llist_init(llist_t* dst, blob_t* nodesArray, const uint8_t max_nodes);
blob_t* llist_pop_front(llist_t* src);
void llist_push_back(llist_t* dst, blob_t* blob);
void llist_save_blobs(llist_t* dst, llist_t* src);
void llist_update_blob(blob_t* dst, blob_t* src);
void llist_remove_blob(llist_t* src, blob_t* blob);
int8_t llist_size(llist_t* ptr);

void raz_blob(blob_t* node);

void find_blobs(
  image_t* inFrame_ptr,
  char* bitmap_ptr,
  const int rows,
  const int cols,
  const int pixelThreshold,
  const unsigned int minBlobPix,
  const unsigned int maxBlobPix,
  llist_t* freeBlob_ptr,
  llist_t* blobs_ptr,
  llist_t* blobsToUpdate_ptr,
  llist_t* blobsToAdd_ptr,
  llist_t* outputBlobs_ptr
);

#endif /*__BLOB_H__*/
