/* This file is part of the OpenMV project.
   Copyright (c) 2013-2017 Ibrahim Abdelkader <iabdalkader@openmv.io> & Kwabena W. Agyeman <kwagyeman@openmv.io>
   This work is licensed under the MIT license, see the file LICENSE for details.
*/

#include "blob.h"

#define CHAR_BITS (sizeof(char) * 8)
#define CHAR_MASK (CHAR_BITS - 1)
#define CHAR_SHIFT IM_LOG2(CHAR_MASK)

////////////// Bitmap //////////////

void bitmap_alloc(bitmap_t *ptr, size_t size) {
  ptr->size = size;
  ptr->data = (char *) fb_alloc0(((size + CHAR_MASK) >> CHAR_SHIFT) * sizeof(char)); // Need explanation!
}

void bitmap_free(bitmap_t *ptr) {
  if (ptr->data) {
    fb_free();
  }
}

void bitmap_bit_set(bitmap_t *ptr, size_t index) {
  ptr->data[index >> CHAR_SHIFT] |= 1 << (index & CHAR_MASK);
}

bool bitmap_bit_get(bitmap_t *ptr, size_t index) {
  return (ptr->data[index >> CHAR_SHIFT] >> (index & CHAR_MASK)) & 1;
}

////////////// Lifo //////////////

void lifo_alloc_all(lifo_t *ptr, size_t *size, size_t data_len) {
  uint32_t tmp_size;
  ptr->data = (char *) fb_alloc_all(&tmp_size);
  ptr->data_len = data_len;
  ptr->size = tmp_size / data_len;
  ptr->len = 0;
  *size = ptr->size;
}

void lifo_free(lifo_t *ptr) {
  if (ptr->data) {
    fb_free();
  }
}

size_t lifo_size(lifo_t *ptr) {
  return ptr->len;
}

void lifo_enqueue(lifo_t *ptr, void *data) {
  memcpy(ptr->data + (ptr->len * ptr->data_len), data, ptr->data_len);
  ptr->len += 1;
}

void lifo_dequeue(lifo_t *ptr, void *data) {
  if (data) {
    memcpy(data, ptr->data + ((ptr->len - 1) * ptr->data_len), ptr->data_len);
  }
  ptr->len -= 1;
}

////////////// List //////////////

void list_init(list_t *ptr, size_t data_len) {
  ptr->head_ptr = NULL;
  ptr->tail_ptr = NULL;
  ptr->size = 0;
  ptr->data_len = data_len;
}

void list_copy(list_t *dst, list_t *src) {
  memcpy(dst, src, sizeof(list_t));
}

size_t list_size(list_t *ptr) {
  return ptr->size;
}

/////////// list_push_back() ///////////
// Adds a new element at the end of the list container.
// The content of *data is added to the new element.
// This increases the container size by one.
void list_push_back(heap_t *heap, list_t *ptr, void *data) {

  list_lnk_t *tmp = (list_lnk_t *) xalloc(heap, sizeof(list_lnk_t) + ptr->data_len);

  memcpy(tmp->data, data, ptr->data_len);

  if (ptr->size++) {
    tmp->next_ptr = NULL;
    tmp->prev_ptr = ptr->tail_ptr;
    ptr->tail_ptr->next_ptr = tmp;
    ptr->tail_ptr = tmp;
  } else {
    tmp->next_ptr = NULL;
    tmp->prev_ptr = NULL;
    ptr->head_ptr = tmp;
    ptr->tail_ptr = tmp;
  }
}

void list_pop_front(heap_t *heap, list_t *ptr, void *data) {

  list_lnk_t *tmp = ptr->head_ptr;

  if (data) {
    memcpy(data, tmp->data, ptr->data_len);
  }
  tmp->next_ptr->prev_ptr = NULL;
  ptr->head_ptr = tmp->next_ptr;
  ptr->size -= 1;
  xfree(heap, tmp);
}

////////////// Iterators //////////////

list_lnk_t *iterator_start_from_head(list_t *ptr) {
  return ptr->head_ptr;
}

list_lnk_t *iterator_next(list_lnk_t *lnk) {
  return lnk->next_ptr;
}

void iterator_get(list_t *ptr, list_lnk_t *lnk, void *data) {
  memcpy(data, lnk->data, ptr->data_len);
}

void iterator_set(list_t *ptr, list_lnk_t *lnk, void *data) {
  memcpy(lnk->data, data, ptr->data_len);
}

