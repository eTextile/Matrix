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

struct blob {
  uint8_t UID;
  uint8_t alive;
  uint8_t Xcentroid;
  uint8_t Ycentroid;
  uint8_t boxW;
  uint8_t boxH;
  uint8_t boxD;
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
    serialMessage                 message;
    bool                          serialRawData;
    bool                          serialBlobs;
    ofxPanel                      gui;
    ofxButton                     setCalirationButton; // Button to calibrate E256
    ofxIntSlider                  setTresholdSlider; // Set E256 threshold value
    ofxToggle                     getBlobsToggle;
    ofxToggle                     getRawDataToggle;
    uint8_t                       frameBuffer[OUTPUT_BUFFER_SIZE];
    uint8_t                       storedValueRast[DATAS]; // 1D array
    void                          E256_setCaliration(void);
    void                          E256_setTreshold(int & sliderValue);

    bool                          getRawDataVal;
    bool                          getBlobsVal;
    void                          E256_getBlobsState(bool & val);
    void                          E256_getRawDataState(bool & val);
    void                          E256_getBlobs();
    void                          E256_getRawData();
    ofMesh                        mesh;
    void                          keyPressed(int key);
    //std::vector<ofboxPrimitive>  boxe;
  };
