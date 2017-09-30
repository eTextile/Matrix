/*
   This file is part of the OpenMV project.
   Copyright (c) 2013/2014 Ibrahim Abdelkader <i.abdalkader@gmail.com>
   This work is licensed under the MIT license, see the file LICENSE for details.
   
   Memory allocation functions.
*/

#ifndef __XALLOC_H__
#define __XALLOC_H__

#include "heap.h"

void *xalloc(heap_t *heap, uint32_t size);
void xfree(heap_t *heap, void *mem);

#endif /* __XALLOC_H__ */
