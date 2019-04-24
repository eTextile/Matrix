#pragma once

#include "ofMain.h"
#include "ofxSerial.h"
#include "ofxOsc.h"
#include "ofxGui.h"

// #include "ofx/IO/SLIPEncoding.h"

#define USB_PORT         "/dev/ttyACM0"
#define BAUD_RATE        230400  // With Teensy, it's always the same native speed. The baud rate setting is ignored.
//#define BAUD_RATE        115200  // With Teensy, it's always the same native speed. The baud rate setting is ignored.
#define DATAS            256     // Numbur of bytes received from the teensy
#define ROWS             16      // Number of rows in the hardwear sensor matrix
#define COLS             16      // Number of colums in the hardwear sensor matrix
#define X_NEWSIZE        64      // Number of rows after the softwear interpolation
#define Y_NEWSIZE        64      // Number of colums after the softwear interpolation

const int OUTPUT_BUFFER_SIZE = 1024;

//#define HOST             "localhost"
//#define HOST             "10.42.0.255"
#define UDP_OUTPUT_PORT  7771
#define UDP_INPUT_PORT   1234

// E256 Firmware v1.0
//blobPaket[0] = blob->UID;        // uint8_t
//blobPaket[1] = blob->centroid.X; // uint8_t
//blobPaket[2] = blob->centroid.Y; // uint8_t
//blobPaket[3] = blob->box.W;      // uint8_t
//blobPaket[4] = blob->box.H;      // uint8_t
//blobPaket[5] = blob->box.D;      // uint8_t

struct blob {
  uint8_t UID;
  int8_t Xcentroid;
  int8_t Ycentroid;
  int8_t boxW;
  int8_t boxH;
  int8_t boxD;
  //int16_t pixels; // TODO in the Firmware 1.0
};

struct serialMessage {
    std::string OSCmessage;
    std::string exception;
};

using namespace ofxIO;

class ofApp: public ofBaseApp {

public:
    void                          setup(void);
    void                          update(void);
    void                          draw(void);
    void                          exit(void);
    char                          buffer[OUTPUT_BUFFER_SIZE];

    void                          onSerialBuffer(const ofxIO::SerialBufferEventArgs& args);
    void                          onSerialError(const ofxIO::SerialBufferErrorEventArgs& args);

    ofxIO::SLIPPacketSerialDevice serialDevice;
    std::vector<serialMessage>    serialMessages; // SerialMessages is a vector of SerialMessage
    std::vector<ofxOscMessage>    blobs;

    //ofxOscBundle                  OSCbundle;
    //serialMessage                 message;
    ofxPanel                      gui;
    ofxButton                     calirationButton; // Button to calibrate E256
    ofxIntSlider                  tresholdValue; // Set E256 threshold value
    void                          E256_setCaliration(void);
    void                          E256_setTreshold(int & sliderValue);
    void                          E256_blobsRequest(void);
    void                          E256_matrix_raw_data(void);

    //std::vector<ofboxPrimitive>  boxe;

    void                          keyPressed(int key);
  };
