#pragma once

#include "ofMain.h"
#include "ofxSerial.h"
#include "ofxGui.h"
#include "ofxOsc.h"
#include "ofxCvGrayscaleImage.h"
#include "ofxCvContourFinder.h"

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

#define DEBUG_SERIAL 1
#define DEBUG_PRINT  0
#define DEBUG_OSC    0

struct centroid {
    ofVec2f position;
    uint8_t pressure;
    float perimeter;
    int UID;
    bool isDead;
};

using namespace ofx::IO;

class ofApp: public ofBaseApp {

public:
    void                    setup();
    void                    update();
    void                    draw();
    void                    exit();
    void                    onSerialBuffer(const SerialBufferEventArgs& args);
    void                    onSerialError(const SerialBufferErrorEventArgs& args);
    void                    handleOSC();
    PacketSerialDevice      device;
    ByteBuffer              buffer;  // Create a byte buffer.
    int                     sensorID;
    uint8_t                 SerialData_SLIP[DATAS];        // SLIP Encoding
    uint8_t                 serialData[DATAS];             // Decoded
    uint8_t                 storedValueRast[ROWS * COLS];  // 1D array
    int8_t                  threshold;
    bool                    newFrame;
    ofPixels                interpolatedFrame;
    ofxCvGrayscaleImage     grayImage;
    ofxCvGrayscaleImage     grayBg;
    ofxCvGrayscaleImage     grayDiff;
    ofxCvContourFinder      contourFinder;
    bool                    bLearnBakground;
    ofxCvBlob               blob;
    ofPixels                grayImageCopy;
    void                    toggleDspPressed(bool & toggleState);
    void                    sliderVolumeValue(float & sliderValue_A);
    void                    sliderTresholdValue(float & sliderValue_B);
    void                    keyPressed(int key);
    ofxPanel                gui;
    ofxToggle               toggleDsp;         // OSC DSP switch
    ofxFloatSlider          sliderVolume;      // OSC audio volume
    ofxFloatSlider          sliderTreshold;    // OSC threshold
    ofMesh                  mesh;
    ofxOscSender            sender;
    ofxOscReceiver          receiver;
    ofTrueTypeFont          font;
    vector<centroid>        centroids;
};
