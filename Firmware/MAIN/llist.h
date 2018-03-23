/*
  This file is part of the eTextile-matrix-sensor project - http://matrix.eTextile.org
  Copyright (c) 2014-2018 Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "config.h"

#ifndef __LLIST_H__
#define __LLIST_H__

////////////// Linked list - Fonction prototypes //////////////

void llist_raz(llist_t* ptr);

void llist_init(llist_t* dst, blob_t* nodesArray, const uint8_t max_nodes);

blob_t* llist_pop_front(llist_t* src);

void llist_push_back(llist_t* dst, blob_t* blob);

void llist_save_blobs(llist_t* dst, llist_t* src);

void llist_remove_blob(llist_t* src, blob_t* blob);

int8_t llist_size(llist_t* ptr);

////////////// Iterators //////////////

int8_t list_size(llist_t* ptr);

blob_t* iterator_start_from_head(llist_t* src);

blob_t* iterator_next(blob_t* src);

#endif /*__LLIST_H__*/
