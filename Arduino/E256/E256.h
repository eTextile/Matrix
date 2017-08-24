// #ifndef __E256_H__
// #define __E256_H__

#include <arm_math.h>

#define  BAUD_RATE            230400
#define  COLS                 16
#define  ROWS                 16
#define  SCALE                4
#define  FRAME                COLS*ROWS*SCALE
#define  CALIBRATION_CYCLES   4

#define MIN_BLOB_PIX          4 // Only blobs that with more pixels than 4

#define A0_PIN                A0  // The output of multiplexerA (SIG pin) is connected to Arduino Analog pin 0
#define A1_PIN                A1  // The output of multiplexerB (SIG pin) is connected to Arduino Analog pin 1

void onPacket(const uint8_t* buffer, size_t size);
void calibrate(uint8_t* id, int val, int frame);
