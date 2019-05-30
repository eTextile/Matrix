#pragma once

#include "ofMain.h"
#include "ofxSerial.h"
#include "ofxOsc.h"
#include "ofxGui.h"

#define USB_PORT              "/dev/ttyACM0"
#define BAUD_RATE             230400  // With Teensy, it's always the same native speed. The baud rate setting is ignored.
#define COLS                  16
#define ROWS                  16
#define DUAL_ROWS             (ROWS / 2)
#define SCALE_X               4
#define SCALE_Y               4
#define ROW_FRAME             (COLS * ROWS)
#define NEW_COLS              (COLS * SCALE_X)
#define NEW_ROWS              (ROWS * SCALE_Y)
#define NEW_FRAME             (NEW_COLS * NEW_ROWS)

#define OUT_BUFFER_SIZE       1024
#define IN_BUFFER_SIZE        65535

//#define HOST                "10.42.0.255"
//#define UDP_OUTPUT_PORT     7771
//#define UDP_INPUT_PORT      1234

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
    char                          requestBuffer[OUT_BUFFER_SIZE];

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
    ofxToggle                     getInterpDataToggle;

    uint8_t                       inputFrameBuffer[IN_BUFFER_SIZE];

    uint8_t                       rawValues[ROW_FRAME]; // 1D array
    uint8_t                       interpValues[NEW_FRAME]; // 1D array

    void                          E256_setCaliration(void);
    void                          E256_setTreshold(int & sliderValue);

    bool                          getRawData;
    bool                          getInterpData;
    bool                          getBlobs;

    void                          E256_getRawDataStart(bool & val);
    void                          E256_getInterpDataStart(bool & val);
    void                          E256_getBlobsStart(bool & val);

    void                          E256_getRawData();
    void                          E256_getInterpData();
    void                          E256_getBlobs();

    ofMesh                        rawDataMesh;
    ofMesh                        interpDataMesh;

    void                          keyPressed(int key);
    //std::vector<ofboxPrimitive>  boxe;
  };
