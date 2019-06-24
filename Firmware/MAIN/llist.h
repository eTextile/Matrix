/*
  This file is part of the eTextile-matrix-sensor project - http://matrix.eTextile.org
  Copyright (c) 2014-2019 Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#ifndef __LLIST_H__
#define __LLIST_H__

#include "config.h"

#include "blob.h"
typedef struct blob blob_t; // Forward declaration

typedef struct llist {
  blob_t* head_ptr;
  blob_t* tail_ptr;
  uint8_t max_nodes;
  int8_t index; // If no element in the linked list, index is -1
} llist_t;

////////////// Linked list - Fonction prototypes //////////////

void llist_raz(llist_t* src);
void llist_init(llist_t* dst, blob_t* nodesArray, uint8_t max_nodes);

blob_t* llist_pop_front(llist_t* src);
void llist_push_back(llist_t* dst, blob_t* blob);

void llist_save_blobs(llist_t* dst, llist_t* src);
void llist_remove_blob(llist_t* src, blob_t* blob);
void llist_sort(llist_t* ptr);

////////////// Iterators //////////////

#define ITERATOR_START_FROM_HEAD(src) \
  ({ \
    __typeof__ (src) _src = (src); \
    (blob_t*)_src->head_ptr; \
  })

#define ITERATOR_NEXT(src) \
  ({ \
    __typeof__ (src) _src = (src); \
    (blob_t*)_src->next_ptr; \
  })

#endif /*__LLIST_H__*/
