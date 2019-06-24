/*
  This file is part of the eTextile-matrix-sensor project - http://matrix.eTextile.org
  Copyright (c) 2014-2019 Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#ifndef __LIFO_H__
#define __LIFO_H__

#include "config.h"

typedef struct xylr {
  uint8_t x;
  uint8_t y;
  uint8_t l;
  uint8_t r;
  uint8_t t_l;
  uint8_t b_l;
  struct xylr* prev_ptr;
} xylr_t;

typedef struct lifo {
  xylr* head_ptr;
  uint8_t max_nodes; 
  int16_t index;     // If no element in the lifo, index is -1
} lifo_t;

void lifo_raz(lifo_t* src);
void lifo_init(lifo_t* dst, xylr_t* nodesArray, uint8_t max_nodes);
xylr_t* lifo_dequeue(lifo_t* src);
void lifo_enqueue(lifo_t* dst, xylr_t* src);
void lifo_save(lifo_t* dst, lifo_t* src);

#endif /*__LIFO_H__*/
