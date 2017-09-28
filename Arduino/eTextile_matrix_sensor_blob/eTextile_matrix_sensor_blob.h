#ifndef __ETEXTILE_MATRIX_SENSOR_BLOB_H__
#define __ETEXTILE_MATRIX_SENSOR_BLOB_H__

#include "blob.h"

// Control pins to send values to the 8-BITs shift registers used on the E-256 PCB
// shiftOut using SPI library : https://forum.arduino.cc/index.php?topic=52383.0
// Arduino UNO - SPI PINS
// DATA_PIN -> SPI:MOSI -> D11 // Pin connected to Data in (DS) of the firdt 74HC595 8-BIT shift register
// CLOCK_PIN -> SPI:SCK -> D13 // Pin connected to clock pin (SH_CP) of the first 74HC595 8-BIT shift register
// LATCH_PIN -> SPI:SS -> D10  // Pin connected to latch pin (ST_CP) of the first 74HC595 8-BIT shift register

// Teensy - SPI PINS https://www.pjrc.com/teensy/td_libs_SPI.html

#define  BUILTIN_LED          13
#define  BUTTON_PIN           32  // Button on the eTextile Teensy shield
#define  BAUD_RATE            230400
#define  COLS                 16
#define  ROWS                 16
#define  ROW_FRAME            (COLS * ROWS)

#define  SCALE                4
#define  INC                  (1.0 / SCALE)
#define  NEW_COLS             (COLS * SCALE)
#define  NEW_ROWS             (ROWS * SCALE)
#define  NEW_FRAME            (NEW_COLS * NEW_ROWS)

#define  CALIBRATION_CYCLES   4   // Set the calibration cycles
#define  THRESHOLD            15  // Set the threshold that determine toutch sensitivity (10 is low 30 is high)
#define  MIN_BLOB_PIX         4   // Set the minimum blob pixels
#define  MIN_BLOB_SIZE        9   // Set the minimum blob size
#define  A0_PIN               A0  // The output of multiplexerA (SIG pin) is connected to Analog pin 0


// Digital pins array
// See the attached home made PCB (Eagle file) to understand the Digital and Analog pin mapping
const int rowPins[ROWS] = {
  27, 26, 25, 24, 12, 11, 10, 9, 8, 7, 6, 5, 33, 2, 1, 0
};

// Analog pins array
const int columnPins[COLS] = {
  A17, A18, A19, A0, A20, A1, A2, A3, A4, A5, A6, A7, A11, A8, A10, A9
};

volatile uint16_t minVals[ROW_FRAME] = {0};  // Array to store smallest values
float32_t frameValues[ROW_FRAME] = {0};      // Array to store ofset input values
uint16_t bilinIntOutput[NEW_FRAME] = {0};    // Bilinear interpolation Output buffer
uint8_t myPacket[ROW_FRAME] = {0};           // Array to store values to transmit

#ifdef CORE_TEENSY
arm_bilinear_interp_instance_f32 S;
#endif // __CORE_TEENSY__

image_t       frame;
list_t        BlobOut;
rectangle_t   Roi;

void onPacket(const uint8_t *buffer, size_t size);
void _calibrate(volatile uint16_t *sumArray);
void bootBlink(int flash);
void pushButton();

#endif /* __ETEXTILE_MATRIX_SENSOR_BLOB_H__ */
