/*
   This file is part of the OpenMV project.
   Copyright (c) 2013/2014 Ibrahim Abdelkader <i.abdalkader@gmail.com>
   This work is licensed under the MIT license, see the file LICENSE for details.

   Memory allocation functions.
*/

#include "xalloc.h"

static void xalloc_fail() {
  // nlr_raise(mp_obj_new_exception_msg(&mp_type_MemoryError, "Out of Memory!!!"));
}

// returns null pointer without error if size==0
void *xalloc(uint32_t size) {
  void *mem = gc_alloc(size, false); // TODO: repace it!
  if (size && (mem == NULL)) {
    // xalloc_fail();
  }
  return mem;
}

// returns without error if mem==null
void xfree(void *mem) {
  gc_free(mem); // TODO: repace it!
}
