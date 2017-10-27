/* This file is part of the OpenMV project.
   Copyright (c) 2013-2017 Ibrahim Abdelkader <iabdalkader@openmv.io> & Kwabena W. Agyeman <kwagyeman@openmv.io>
   This work is licensed under the MIT license, see the file LICENSE for details.
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

////////////// Bitmap //////////////

typedef struct {
  size_t siZe;
  char* data;
} bitmap_t;

void bitmap_bit_set(char* arrayPtr, int index);
char bitmap_bit_get(char* arrayPtr, int index);
void bitmap_clear(char* arrayPtr);
void bitmap_print(char* arrayPtr);

#define BITMAP_ROW_INDEX(imagePtr, y) \
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

////////////// Lifo //////////////

typedef struct lifo {
  size_t len;      // Number of elements
  size_t data_len; // Size of an element
  size_t index;    //
  char* data;
} lifo_t;

void lifo_alloc_all(lifo_t* ptr, size_t* siZe, size_t data_len);
void lifo_free(lifo_t* ptr);

size_t lifo_size(lifo_t *ptr);

void lifo_enqueue(lifo_t *ptr, void* data);
void lifo_dequeue(lifo_t *ptr, void* data);

////////////// list //////////////

typedef struct node {
  struct node* next_ptr;
  char data[];
} node_t;

typedef struct list {
  node_t* head_ptr;
  size_t data_len;
  size_t index;
} list_t;

void list_init(list_t* ptr, size_t data_len);
void list_alloc_all(list_t* ptr, size_t data_len);
void list_list_memcpy(list_t* dst, list_t* src);
size_t list_size(list_t* ptr);

node_t* list_get_freeNode(list_t* ptr);
void list_save_node(list_t* ptr, node_t* node);
void list_copy(list_t* dst, list_t* src);

void list_push_back(list_t* ptr, void* data, node_t* freeNode);
void list_pop_front(list_t* ptr, void* data, node_t* freeNode);

////////////// Iterators //////////////

node_t* iterator_start_from_head(list_t* ptr);
node_t* iterator_next(node_t* lnk);

void iterator_get(node_t* lnk, void* data, list_t* ptr);

#endif /*__COLLECTIONS_H__*/
