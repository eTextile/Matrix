/*
  This file is part of the eTextile-matrix-sensor project - http://matrix.eTextile.org
  Copyright (c) 2014-2019 Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "lifo.h"

void lifo_raz(lifo_t* src) {

  src->max_nodes = 0;
  src->head_ptr = NULL;
  src->index = -1;
}

void lifo_init(lifo_t* dst, xylr_t* nodesArray, uint8_t max_nodes) {

  dst->head_ptr = &nodesArray[max_nodes - 1];
  dst->head_ptr->prev_ptr = NULL;
  dst->index++;
  dst->max_nodes++;

  //Serial.printf("\n DEBUG_LIFO / lifo_init: %p", dst->head_ptr);

  for (uint8_t i = max_nodes - 1; i > 1; i--) {
    nodesArray[i - 1].prev_ptr = dst->head_ptr;
    dst->head_ptr = &nodesArray[i - 1];
    //Serial.printf("\n DEBUG_LIFO / lifo_init: %p", dst->head_ptr);
    dst->index++;
    dst->max_nodes++;
  }
}

xylr_t* lifo_dequeue(lifo_t* src) {
  xylr_t* node = NULL;
  //Serial.printf("\n DEBUG_LIFO / lifo_dequeue / get node: %p", src->head_ptr);

  if (src->index > -1) {
    node = src->head_ptr;
    //Serial.printf("\n DEBUG_LIFO / lifo_dequeue / get node: %p", node);
    if (src->index > 0) {
      src->head_ptr = src->head_ptr->prev_ptr;
      //Serial.printf("\n DEBUG_LIFO / lifo_dequeue / change src HEAD: %p", src->head_ptr);
      src->index--;
      return node;
    } else {
      //Serial.printf("\n DEBUG_LIFO / lifo_dequeue / last node: %p", src);
      src->head_ptr = NULL;
      src->index--;
      return node;
    }
    //Serial.printf("\n DEBUG_LIFO / lifo_dequeue / EXIT");
  } else {
    return NULL;
  }
}

void lifo_enqueue(lifo_t* dst, xylr_t* src) {

  //Serial.printf("\n DEBUG_LIFO / lifo_enqueue / set node: %p", src);

  if (dst->index > -1) {
    src->prev_ptr = dst->head_ptr;
    dst->head_ptr = src;
  } else {
    dst->head_ptr = src;
    src->prev_ptr = NULL;
  }
  dst->index++;
}


void lifo_save(lifo_t* dst, lifo_t* src) {

  while (src->index > -1) {
    xylr_t* tmpNode = src->head_ptr->prev_ptr;

    src->head_ptr->prev_ptr = dst->head_ptr;
    dst->head_ptr = src->head_ptr;

    src->head_ptr = tmpNode;

    src->index--;
    dst->index++;
  }
}
