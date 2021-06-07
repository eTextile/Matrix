/*
  This file is part of the E256 - eTextile matrix sensor project - http://matrix.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#ifndef __SCAN_H__
#define __SCAN_H__

#include "config.h"
#include "presets.h"
#include "blob.h"

typedef struct image image_t;       // Forward declaration
typedef struct preset preset_t;     // Forward declaration

#include <SPI.h>                    // https://github.com/PaulStoffregen/SPI
#include <ADC.h>                    // https://github.com/pedvide/ADC

void SPI_SETUP(void);
void ADC_SETUP(void);
void SCAN_SETUP(image_t* inputFrame_ptr);
void calibrate_matrix(preset_t* presets_ptr);
void scan_matrix(void);

#endif /*__SCAN_H__*/
