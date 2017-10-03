/*
   This file is part of the OpenMV project.
   Copyright (c) 2013/2014 Ibrahim Abdelkader <i.abdalkader@gmail.com>
   This work is licensed under the MIT license, see the file LICENSE for details.

   Framebuffer stuff.
*/

#include "framebuffer.h"

char _fb_base;

framebuffer_t *fb_framebuffer = (framebuffer_t *) &_fb_base;

uint32_t fb_buffer_size() {
  return (MAIN_FB()->w * MAIN_FB()->h) * sizeof(uint8_t);
}
