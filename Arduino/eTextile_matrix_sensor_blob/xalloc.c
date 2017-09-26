/*
   This file is part of the OpenMV project.
   Copyright (c) 2013/2014 Ibrahim Abdelkader <i.abdalkader@gmail.com>
   This work is licensed under the MIT license, see the file LICENSE for details.

   Memory allocation functions.
*/

#include "xalloc.h"

heap_t  *heap;

void *xalloc(heap_t *heap, uint32_t size) {
  void *mem = heap_alloc(heap, size);
  return mem;
}

void xfree(heap_t *heap, void *mem) {
  heap_free(heap, mem);
}
