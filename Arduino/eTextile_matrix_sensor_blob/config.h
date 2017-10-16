#ifndef __CONFIG_H__
#define __CONFIG_H__

#define BUILTIN_LED          13
#define BUTTON_PIN           32  // Button on the eTextile Teensy shield
#define BAUD_RATE            230400
#define COLS                 16
#define ROWS                 16
#define SCALE                4  
#define MAX_BLOBS            20  // Set the maximum blob number

#define ROW_FRAME            (COLS * ROWS)
#define NEW_COLS             (COLS * SCALE)
#define NEW_ROWS             (ROWS * SCALE)
#define NEW_FRAME            (NEW_COLS * NEW_ROWS)
#define INC                  (1.0 / SCALE)
#define CALIBRATION_CYCLES   4   // Set the calibration cycles
#define THRESHOLD            200 // Set the threshold that determine toutch sensitivity (10 is low 30 is high)
#define MIN_BLOB_PIX         4   // Set the minimum blob pixels
#define MIN_BLOB_SIZE        9   // Set the minimum blob size
#define A0_PIN               A0  // The output of multiplexerA (SIG pin) is connected to Analog pin 0

#define DEBUG_INPUT          true
#define DEBUG_INTERP         false
#define DEBUG_BLOB           true
#define DEBUG_OUTPUT         false

#endif /*__CONFIG_H__*/
