#include "ofApp.h"

void ofApp::setup(void) {
  ofSetVerticalSync(true);
  ofSetWindowTitle("E256 - V1.0");

  ofSetLogLevel(OF_LOG_VERBOSE);
  /*
  OF_LOG_VERBOSE
  OF_LOG_NOTICE
  OF_LOG_WARNING
  OF_LOG_ERROR
  OF_LOG_FATAL_ERROR
  OF_LOG_SILENT
  */
  auto devicesInfo = SerialDeviceUtils::listDevices();
  ofLogNotice("ofApp::setup") << "Connected Devices: ";
  for (auto& SerialDevice: devicesInfo) ofLogNotice("ofApp::setup") << "\t" << SerialDevice;

  /*
  using SerialDevice::port;
  using SerialDevice::baudRate;
  using SerialDevice::dataBits;
  using SerialDevice::stopBits;
  using SerialDevice::timeout;
  using SerialDevice::isClearToSend;
  using SerialDevice::isDataSetReady;
  using SerialDevice::isRingIndicated;
  using SerialDevice::isCarrierDetected;
  using SerialDevice::isOpen;
  using SerialDevice::setDataTerminalReady;
  using SerialDevice::getPortName;
  */
  if (!devicesInfo.empty()) {
    bool success = serialDevice.setup(
      USB_PORT,
      BAUD_RATE
      //SerialDevice::DATA_BITS_EIGHT,
      //SerialDevice::PAR_NONE,
      //SerialDevice::STOP_ONE,
      //SerialDevice::FLOW_CTRL_HARDWARE
    );
    if (success) {
      serialDevice.registerAllEvents(this);
      ofLogNotice("ofApp::setup") << "Successfully setup: " << USB_PORT;
    }
    else {
      ofLogNotice("ofApp::setup") << "Unable to setup: " << USB_PORT;
    }
  }
  else {
    ofLogNotice("ofApp::setup") << "No devices connected!";
  }
  //sender.setup(HOST, UDP_OUTPUT_PORT); // OSC - UDP config
  //receiver.setup(UDP_INPUT_PORT); // SLIP-OSC via wifi

  setCalirationButton.addListener(this, &ofApp::E256_setCaliration);
  setTresholdSlider.addListener(this, &ofApp::E256_setTreshold);

  gui.setup("E256 - Parameters");
  gui.add(setCalirationButton.setup("Calibrate"));
  gui.add(setTresholdSlider.setup("Threshold", 30, 0, 100));
  gui.add(getRawDataToggle.setup("getRawData", false));
  gui.add(getBlobsToggle.setup("getBlobs", true));

  ofBackground(0);
  serialBlobs = true;
  serialRawData = true;
}

/////////////////////// SERIAL EVENT ///////////////////////
void ofApp::onSerialBuffer(const ofxIO::SerialBufferEventArgs& args) {
  message.OSCmessage = args.buffer().toString();
  if (getRawDataToggle) serialRawData = true;
  if (getBlobsToggle) serialBlobs = true;
}

void ofApp::onSerialError(const ofxIO::SerialBufferErrorEventArgs& args) {
  message.exception = args.exception().displayText();
  ofLogNotice("ofApp::onSerialError") << "E256 - Serial ERROR : " << args.exception().displayText();
  if (getRawDataToggle) serialRawData = true;
  if (getBlobsToggle) serialBlobs = true;
}

/////////////////////// UPDATE ///////////////////////
void ofApp::update(void) {

  // GET_BLOBS MODE

  //ofLogNotice("ofApp::onSerialBuffer") << "E256 - Serial message size : "<< message.OSCmessage.size();
  //ofLogNotice("ofApp::onSerialBuffer") << "E256 - Serial message : " << message.OSCmessage;
  // https://en.cppreference.com/w/cpp/regex
  int offset = 12;
  int stringOffset = 0;

  if (getRawDataToggle && serialRawData){
    Poco::RegularExpression regex("/r(.){256}"); // GET X bytes after the "/b"
    Poco::RegularExpression::Match theMatch;
    for (size_t i=0; i<message.OSCmessage.size(); i++){
      ofLogNotice("ofApp::onSerialBuffer") << "INDEX_" << i << " val_" << ofToString(message.OSCmessage[i]);
    }
    E256_getRawData();
    serialRawData = false;
  }

  if (getBlobsToggle && serialBlobs){
    Poco::RegularExpression regex("/b(.){20}"); // GET X bytes after the "/b"
    Poco::RegularExpression::Match theMatch;

    /*
    for (size_t i=0; i<message.OSCmessage.size(); i++){
    ofLogNotice("ofApp::onSerialBuffer") << "INDEX_" << i << " val_" << ofToString(message.OSCmessage[i]);
    }
    */

    // Expand SLIP-OSC serial message to OSC messages
    while (regex.match(message.OSCmessage, stringOffset, theMatch)){
      std::string msg = std::string(message.OSCmessage, theMatch.offset, theMatch.length);

      ofLog(OF_LOG_VERBOSE,"E256_INPUT: UID:%d ALIVE:%d CX:%d CY:%d BW:%d BH:%d BD:%d",
      msg[offset],
      msg[offset + 1],
      msg[offset + 2],
      msg[offset + 3],
      msg[offset + 4],
      msg[offset + 5],
      msg[offset + 6]
    );

    ofxOscMessage oscMessage;
    oscMessage.setAddress("/b");
    oscMessage.addInt32Arg(msg[offset]);     // UID
    oscMessage.addInt32Arg(msg[offset + 1]); // alive
    oscMessage.addInt32Arg(msg[offset + 2]); // Xcentroide
    oscMessage.addInt32Arg(msg[offset + 3]); // Ycentroid
    oscMessage.addInt32Arg(msg[offset + 4]); // boxW
    oscMessage.addInt32Arg(msg[offset + 5]); // boxH
    oscMessage.addInt32Arg(msg[offset + 6]); // boxD
    blobs.push_back(oscMessage);

    stringOffset = theMatch.offset + theMatch.length;
  }
  E256_getBlobs();
  serialBlobs = false;
  }
}

//////////////////////// DRAW ////////////////////////
void ofApp::draw(void) {

  ofSetColor(255);
  gui.draw();

  std::stringstream dashboard;
  dashboard << "     Connected to : " << serialDevice.port() << std::endl;
  dashboard << "SLIP-OSC-OUT port : " << UDP_OUTPUT_PORT << std::endl;
  dashboard << " SLIP-OSC-IN port : " << UDP_INPUT_PORT << std::endl;
  dashboard << "              FPS : " << (int)ofGetFrameRate() << std::endl;
  ofDrawBitmapString(dashboard.str(), ofVec2f(20, 200)); // Draw the GUI menu

  // GET_BLOBS MODE
  const int BLOB_SCALE = 10;
  for (size_t index = 0; index < blobs.size(); ++index){

    uint8_t blobID    = blobs[index].getArgAsInt(0) & 0xFF;
    uint8_t alive     = blobs[index].getArgAsInt(1) & 0xFF;
    float Xcentroid   = blobs[index].getArgAsInt(2) & 0xFF;
    float Ycentroid   = blobs[index].getArgAsInt(3) & 0xFF;
    uint8_t boxW      = blobs[index].getArgAsInt(4) & 0xFF;
    uint8_t boxH      = blobs[index].getArgAsInt(5) & 0xFF;
    uint8_t boxD      = blobs[index].getArgAsInt(6) & 0xFF;

    //ofLog(OF_LOG_VERBOSE,"E256_INPUT: UID:%d ALIVE:%d CX:%f CY:%f BW:%d BH:%d BD:%d",blobID, alive, Xcentroid, Ycentroid, boxW, boxH, boxD);

    if(blobs[index].getAddress() == "/r"){
      //TODO
    }

    if(blobs[index].getAddress() == "/b"){
      Xcentroid = (Xcentroid / 64) * ofGetWindowWidth();
      Ycentroid = (Ycentroid / 64) * ofGetWindowHeight();
      boxW = boxW * BLOB_SCALE;
      boxH = boxH * BLOB_SCALE;
      boxD = boxD * BLOB_SCALE;

      if (alive > 0){

        ofBoxPrimitive box;
        box.setMode(OF_PRIMITIVE_TRIANGLES);
        box.setResolution(1);
        box.set(boxW, boxH, boxD/4);
        box.setPosition(Xcentroid - boxW*0.5, Ycentroid - boxH*0.5, 0);

        ofCylinderPrimitive cylinder;
        cylinder.set(10, 1);
        cylinder.setPosition(Xcentroid - boxW*0.5, Ycentroid - boxH*0.5, 0);

        ofPushMatrix();
        ofTranslate(0, 0);
        ofRotateDeg(4, 10, 0, 0);
        ofSetColor(255);
        box.drawWireframe();
        //box.draw();
        ofSetColor(255, 0, 0);
        cylinder.draw();
        ofPopMatrix();
      }
      else {
        blobs.erase(blobs.begin() + index);
      }
    }
  }
  blobs.clear();

  //ofPushMatrix();
  //ofTranslate(0, 0);
  //ofRotateDeg(4, 80, 0, 0);
  //ofPopMatrix();
  //E256_blobsRequest();
}

// E256 matrix sensor - SET CALIBRATION
void ofApp::E256_setCaliration(void) {
  ofxOscMessage OSCmsg;
  OSCmsg.setAddress("/c"); // calibrate
  OSCmsg.addInt32Arg(10); // Set calibration cycles

  osc::OutboundPacketStream packet(buffer, OUTPUT_BUFFER_SIZE);
  packet.Clear();
  packet << osc::BeginMessage(OSCmsg.getAddress().data());
  packet << OSCmsg.getArgAsInt32(0);
  packet << osc::EndMessage;
  serialDevice.send(ByteBuffer(packet.Data(), packet.Size()));
  ofLogNotice("ofApp::E256_setCaliration") << "E256 - Calibration requested : " << OSCmsg.getArgAsInt32(0);
}

// E256 matrix sensor - SET THRESHOLD
void ofApp::E256_setTreshold(int & tresholdValue) {
  ofxOscMessage OSCmsg;
  OSCmsg.setAddress("/t"); // threshold
  OSCmsg.addIntArg((int32_t)tresholdValue);

  osc::OutboundPacketStream packet(buffer, OUTPUT_BUFFER_SIZE);
  packet.Clear();
  packet << osc::BeginMessage(OSCmsg.getAddress().data());
  packet << OSCmsg.getArgAsInt32(0);
  packet << osc::EndMessage;
  serialDevice.send(ByteBuffer(packet.Data(), packet.Size()));
  ofLogNotice("ofApp::E256_setTreshold") << "E256 - Threshold seted : " << OSCmsg.getArgAsInt32(0);
}

// E256 matrix sensor - BLOBS REQUEST
void ofApp::E256_getBlobs(void) {
  ofxOscMessage OSCmsg;
  OSCmsg.setAddress("/b"); // Blobs
  osc::OutboundPacketStream packet(buffer, OUTPUT_BUFFER_SIZE);
  packet.Clear();
  packet << osc::BeginMessage(OSCmsg.getAddress().data());
  packet << osc::EndMessage;
  serialDevice.send(ByteBuffer(packet.Data(), packet.Size()));
  //ofLogNotice("ofApp::E256") << "E256 - BLOBS REQUEST: " << packet.Data();
}

// E256 matrix sensor - MATRIX DATA REQUEST
// 16*16 matrix row data request
void ofApp::E256_getRawData(void) {
  ofxOscMessage OSCmsg;
  OSCmsg.setAddress("/r"); //rowData
  osc::OutboundPacketStream packet(buffer, OUTPUT_BUFFER_SIZE);
  packet.Clear();
  packet << osc::BeginMessage(OSCmsg.getAddress().data());
  packet << osc::EndMessage;
  serialDevice.send(ByteBuffer(packet.Data(), packet.Size()));
  //ofLogNotice("ofApp::E256") << "E256 - RAW DATA REQUEST: " << packet.Data();
}

// E256 matrix sensor - Toggle full screen mode
void ofApp::keyPressed(int key) {
  switch(key) {
    case 'f':
    ofToggleFullscreen();
    break;
    default:
    break;
  }
}

void ofApp::exit(void) {
  setCalirationButton.removeListener(this, &ofApp::E256_setCaliration);
  setTresholdSlider.removeListener(this, &ofApp::E256_setTreshold);
  serialDevice.unregisterAllEvents(this);
}
