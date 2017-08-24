/* This file is part of the OpenMV project.
   Copyright (c) 2013-2017 Ibrahim Abdelkader <iabdalkader@openmv.io> & Kwabena W. Agyeman <kwagyeman@openmv.io>
   This work is licensed under the MIT license, see the file LICENSE for details.
*/

#ifndef __COLLECTIONS_H__
#define __COLLECTIONS_H__

#include <stdbool.h>
#include <stddef.h>

////////////
// bitmap //
////////////

typedef struct bitmap {
  size_t size;
  char *data;
} bitmap_t;

void bitmap_alloc(bitmap_t *ptr, size_t size);
void bitmap_free(bitmap_t *ptr);
void bitmap_bit_set(bitmap_t *ptr, size_t index);
bool bitmap_bit_get(bitmap_t *ptr, size_t index);
#define BITMAP_COMPUTE_ROW_INDEX(image, y) (((image)->w)*(y))
#define BITMAP_COMPUTE_INDEX(row_index, x) ((row_index)+(x))

//////////
// lifo //
//////////

typedef struct lifo {
  size_t len, size, data_len;
  char *data;
} lifo_t;

void lifo_alloc_all(lifo_t *ptr, size_t *size, size_t data_len);
void lifo_free(lifo_t *ptr);
size_t lifo_size(lifo_t *ptr);
void lifo_enqueue(lifo_t *ptr, void *data);
void lifo_dequeue(lifo_t *ptr, void *data);

//////////
// list //
//////////

typedef struct list_lnk {
  struct list_lnk *next_ptr, *prev_ptr;
  char data[];
} list_lnk_t;

typedef struct list {
  list_lnk_t *head_ptr, *tail_ptr;
  size_t size, data_len;
} list_t;

void list_init(list_t *ptr, size_t data_len);
void list_copy(list_t *dst, list_t *src);
size_t list_size(list_t *ptr);
void list_push_back(list_t *ptr, void *data);
void list_pop_front(list_t *ptr, void *data);

//////////////
// iterator //
//////////////

list_lnk_t *iterator_start_from_head(list_t *ptr);
list_lnk_t *iterator_next(list_lnk_t *lnk);
void iterator_get(list_t *ptr, list_lnk_t *lnk, void *data);

#endif /* __COLLECTIONS_H__ */
