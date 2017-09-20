/*
   This file is part of the OpenMV project.
   Copyright (c) 2013/2014 Ibrahim Abdelkader <i.abdalkader@gmail.com>
   This work is licensed under the MIT license, see the file LICENSE for details.

   Framebuffer stuff.
*/

#ifndef __FRAMEBUFFER_H__
#define __FRAMEBUFFER_H__

#include <stdint.h>

typedef struct framebuffer {
  int w, h;
  uint16_t pixels[];
} framebuffer_t;

extern framebuffer_t *fb_framebuffer;

// Use these macros to get a pointer to main or JPEG framebuffer.
#define MAIN_FB()           (fb_framebuffer)

// Use this macro to get a pointer to the free SRAM area located after the framebuffer.
#define MAIN_FB_PIXELS()    (MAIN_FB()->pixels + fb_buffer_size())

// Returns the main frame buffer size, factoring in pixel formats.
uint32_t fb_buffer_size();


#endif /* __FRAMEBUFFER_H__ */
