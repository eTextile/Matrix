/*
  This file is part of the eTextile-matrix-sensor project - http://matrix.eTextile.org
  Copyright (c) 2014-2019 Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#ifndef __LIFO_H__
#define __LIFO_H__

// node <- node <- node

typedef struct lifo {
  xylf_t* head_ptr;
  xylf_t* prev_ptr;
  uint8_t max_nodes;
  int8_t index; // If no element in the linked list, index is -1
} lifo_t;

typedef struct xylf {
  uint8_t x;
  uint8_t y;
  uint8_t l;
  uint8_t r;
} xylf_t;

void lifo_raz(llist_t* ptr);
void lifo_init(llist_t* dst, blob_t* nodesArray, const uint8_t max_nodes);
blob_t* lifo_enqueue(lifo_t* dst);
blob_t* lifo_dequeue(lifo_t* src, xylf_t node);

#endif /*__LIFO_H__*/
