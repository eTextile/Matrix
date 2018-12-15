#pragma once

#include "ofMain.h"
#include "ofxSerial.h"
#include "ofxOsc.h"
#include "ofxGui.h"

#include "ofx/IO/SLIPEncoding.h"

#define USB_PORT         "/dev/ttyACM0"
#define BAUD_RATE        230400  // With Teensy, it's always the same native speed. The baud rate setting is ignored.
#define DATAS            256     // Numbur of bytes received from the teensy
#define ROWS             16      // Number of rows in the hardwear sensor matrix
#define COLS             16      // Number of colums in the hardwear sensor matrix
#define X_NEWSIZE        64      // Number of rows after the softwear interpolation
#define Y_NEWSIZE        64      // Number of colums after the softwear interpolation

#define HOST             "localhost"
//#define HOST           "10.42.0.255"
#define UDP_OUTPUT_PORT  7771
#define UDP_INPUT_PORT   1234

#define DEBUG_SERIAL     1
#define DEBUG_DRAW       1

typedef struct blob {
  uint8_t blobID;
  int8_t posX;
  int8_t posY;
  int8_t posZ;
  int16_t pixels;
} blob_t;

typedef struct serialMessage {
    std::string OSCmessage;
    std::string exception;
} serialMessage_t;

using namespace ofxIO;

class ofApp: public ofBaseApp {

public:
    void                          setup();
    void                          update();
    void                          draw();
    void                          exit();

    SLIPPacketSerialDevice        serialDevice;
    std::vector<serialMessage_t>  serialMessages; // SerialMessages is a vector of SerialMessage
    std::vector<blob_t>           blobs;
    void                          onSerialBuffer(const SerialBufferEventArgs& args);
    void                          onSerialError(const SerialBufferErrorEventArgs& args);
    char                          threshold;
    //bool                          frameRequest;

    ofxPanel                      gui;
    ofxButton                     calirationButton; // Button to calibrate E256
    ofxIntSlider                  tresholdSlider; // Set E256 threshold value
    void                          setCaliration(bool & buttonState);
    void                          setTreshold(int & sliderValue);
    void                          keyPressed(int key);
  };
