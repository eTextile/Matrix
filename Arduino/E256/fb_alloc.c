/*
   This file is part of the OpenMV project.
   Copyright (c) 2013-2016 Kwabena W. Agyeman <kwagyeman@openmv.io>
   This work is licensed under the MIT license, see the file LICENSE for details.

   Interface for using extra frame buffer RAM as a stack.

*/
#include "fb_alloc.h"

extern char _fballoc;
static char *pointer = &_fballoc;

// returns null pointer without error if passed size==0
void *fb_alloc0(uint32_t size) {
  void *mem = fb_alloc(size);
  memset(mem, 0, size); // does nothing if size is zero.
  return mem;
}

void *fb_alloc_all(uint32_t *size) {
  int32_t temp = pointer - ((char *) MAIN_FB_PIXELS()) - sizeof(uint32_t);

  if (temp < sizeof(uint32_t)) {
    *size = 0;
    return NULL;
  }

  *size = (temp / sizeof(uint32_t)) * sizeof(uint32_t); // Round Down
  char *result = pointer - *size;
  char *new_pointer = result - sizeof(uint32_t);

  // size is always 4/8/12/etc. so the value below must be 8 or more.
  *((uint32_t *) new_pointer) = *size + sizeof(uint32_t); // Save size.
  pointer = new_pointer;
  return result;
}

void fb_free() {
  if (pointer < &_fballoc) {
    pointer += *((uint32_t *) pointer); // Get size and pop.
  }
}
