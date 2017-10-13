#ifndef __ETEXTILE_MATRIX_SENSOR_BLOB_H__
#define __ETEXTILE_MATRIX_SENSOR_BLOB_H__

#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <arm_math.h>

#include "blob.h"

#define  BUILTIN_LED          13
#define  BUTTON_PIN           32  // Button on the eTextile Teensy shield
#define  BAUD_RATE            230400
#define  COLS                 16
#define  ROWS                 16
#define  ROW_FRAME            (COLS * ROWS)

#define  SCALE                4
#define  NEW_COLS             (COLS * SCALE)
#define  NEW_ROWS             (ROWS * SCALE)
#define  NEW_FRAME            (NEW_COLS * NEW_ROWS)
#define  INC                  (1.0 / SCALE)

#define  CALIBRATION_CYCLES   4   // Set the calibration cycles
#define  THRESHOLD            100 // Set the threshold that determine toutch sensitivity (10 is low 30 is high)
#define  MIN_BLOB_PIX         4   // Set the minimum blob pixels
#define  MIN_BLOB_SIZE        9   // Set the minimum blob size
#define  A0_PIN               A0  // The output of multiplexerA (SIG pin) is connected to Analog pin 0

#define  MAX_BLOBS            20

// Digital pins array
// See the attached home made PCB (Eagle file) to understand the Digital and Analog pin mapping
const int rowPins[ROWS] = {
  27, 26, 25, 24, 12, 11, 10, 9, 8, 7, 6, 5, 33, 2, 1, 0
};

// Analog pins array
const int columnPins[COLS] = {
  A17, A18, A19, A0, A20, A1, A2, A3, A4, A5, A6, A7, A11, A8, A10, A9
};

// volatile uint16_t minVals[ROW_FRAME] = {0};  // Array to store smallest values
uint16_t minVals[ROW_FRAME] = {0};  // Array to store smallest values
float32_t frameValues[ROW_FRAME] = {0};      // Array to store ofset input values
uint8_t bilinIntOutput[NEW_FRAME] = {0};     // Bilinear interpolation Output buffer
uint8_t myPacket[ROW_FRAME] = {0};           // Array to store values to transmit

#ifdef CORE_TEENSY
arm_bilinear_interp_instance_f32 S;
#endif // __CORE_TEENSY__


char bitmap[NEW_FRAME] = {0};
char *bitmapPtr;
int fps = 0;

image_t       inputFrame;
node_t        tmpNode;
list_t        outputBlobs;

void onPacket(const uint8_t *buffer, size_t size);
void calibrate(volatile uint16_t *sumArray);
void bootBlink(uint8_t flash);
void pushButton();

#endif /*__ETEXTILE_MATRIX_SENSOR_BLOB_H__*/
