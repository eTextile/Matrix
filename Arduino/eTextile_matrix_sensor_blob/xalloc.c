/*
   This file is part of the OpenMV project.
   Copyright (c) 2013/2014 Ibrahim Abdelkader <i.abdalkader@gmail.com>
   This work is licensed under the MIT license, see the file LICENSE for details.

   Memory allocation functions.
*/

// #include <mp.h> // OR https://github.com/micropython/micropython/wiki/Board-Teensy-3.1-3.5-3.6
#include "xalloc.h"

NORETURN static void xalloc_fail() {
  nlr_raise(mp_obj_new_exception_msg(&mp_type_MemoryError, "Out of Memory!!!"));
}

// returns null pointer without error if size==0
void *xalloc(uint32_t size) {
  void *mem = gc_alloc(size, false); // 
  if (size && (mem == NULL)) {
    xalloc_fail();
  }
  return mem;
}

// returns null pointer without error if size==0
void *xalloc_try_alloc(uint32_t size) {
  MP_STATE_MEM(gc_lock_depth)++;
  void *mem = gc_alloc(size, false);
  if (size && (mem == NULL)) {
    return NULL;
  }
  return mem;
}

// returns null pointer without error if size==0
void *xalloc0(uint32_t size) {
  void *mem = gc_alloc(size, false);
  if (size && (mem == NULL)) {
    xalloc_fail();
  }
  memset(mem, 0, size);
  return mem;
}

// returns without error if mem==null
void xfree(void *mem) {
  gc_free(mem);
}
