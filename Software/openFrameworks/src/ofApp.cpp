#include "ofApp.h"

void ofApp::setup(void) {
  ofSetVerticalSync(true);
  ofSetWindowTitle("E256 - V1.0");

  ofSetLogLevel(OF_LOG_SILENT);
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

//calirationButton.addListener(this, &ofApp::E256_setCaliration);
//tresholdValue.addListener(this, &ofApp::E256_setTreshold);

gui.setup("E256 - Parameters");
gui.add(calirationButton.setup("Calibrate"));
gui.add(tresholdValue.setup("Threshold", 30, 0, 100));
ofBackground(0);
E256_blobsRequest();
}

/////////////////////// SERIAL EVENT ///////////////////////
void ofApp::onSerialBuffer(const ofxIO::SerialBufferEventArgs& args) {
  serialMessage message;
  message.OSCmessage = args.buffer().toString();
  //ofLogNotice("ofApp::onSerialBuffer") << "E256 - Serial message size : "<< message.OSCmessage.size();
  //ofLogNotice("ofApp::onSerialBuffer") << "E256 - Serial message : " << message.OSCmessage;
  Poco::RegularExpression regex("/b(.){18}"); // GET X bytes after the "/b"
  Poco::RegularExpression::Match theMatch;

  /*
  for (size_t i=0; i<message.OSCmessage.size(); i++){
    ofLogNotice("ofApp::onSerialBuffer") << "INDEX_" << i << " val_" << ofToString(message.OSCmessage[i]);
  }
  */

  int offset = 12;
  size_t stringOffset = 0;

  // Expand SLIP-OSC serial message to OSC messages
  while (regex.match(message.OSCmessage, stringOffset, theMatch)){
    stringOffset = theMatch.offset + theMatch.length;
    std::string msg = std::string(message.OSCmessage, theMatch.offset, theMatch.length);
    ofxOscMessage oscMessage;
    oscMessage.setAddress("/b");
    oscMessage.addInt32Arg((uint8_t)msg[offset + 0]); // UID
    oscMessage.addInt32Arg((uint8_t)msg[offset + 1]); // Xcentroide
    oscMessage.addInt32Arg((uint8_t)msg[offset + 2]); // Ycentroid
    oscMessage.addInt32Arg((uint8_t)msg[offset + 3]); // boxW
    oscMessage.addInt32Arg((uint8_t)msg[offset + 4]); // boxH
    oscMessage.addInt32Arg((uint8_t)msg[offset + 5]); // boxD
    blobs.push_back(oscMessage);
  }
  E256_blobsRequest();
}

void ofApp::onSerialError(const ofxIO::SerialBufferErrorEventArgs& args) {
  serialMessage message;
  message.exception = args.exception().displayText();
  ofLogNotice("ofApp::onSerialError") << "E256 - Serial ERROR : " << args.exception().displayText();
  E256_blobsRequest();
}

/////////////////////// UPDATE ///////////////////////
void ofApp::update(void) {

}

//////////////////////////////////////////// DRAW ////////////////////////////////////////////
void ofApp::draw(void) {

  ofBackground(0);
  ofSetColor(255);
  gui.draw();

  std::stringstream ss;
  ss << "     Connected to : " << serialDevice.port() << std::endl;
  ss << "SLIP-OSC-OUT port : " << UDP_OUTPUT_PORT << std::endl;
  ss << " SLIP-OSC-IN port : " << UDP_INPUT_PORT << std::endl;
  ss << "              FPS : " << (int)ofGetFrameRate() << std::endl;
  ofDrawBitmapString(ss.str(), ofVec2f(20, 200)); // Draw the GUI menu

  ofNoFill();
  ofSetLineWidth(1.5); // set line width to 1
  ofSetColor(255);

  const int BLOB_SCALE = 4;
  for (size_t index = 0; index < blobs.size(); ++index){

    if(blobs[index].getAddress() == "/b"){
      int blobID = blobs[index].getArgAsInt(0);
      float Xcentroid = ((float)blobs[index].getArgAsInt(1) / 64) * ofGetWindowWidth();
      float Ycentroid = ((float)blobs[index].getArgAsInt(2) / 64) * ofGetWindowHeight();
      int boxW = blobs[index].getArgAsInt(3) * BLOB_SCALE;
      int boxH = blobs[index].getArgAsInt(4) * BLOB_SCALE;
      int boxD = blobs[index].getArgAsInt(5) * BLOB_SCALE;

      ofLogNotice("ofApp::draw") << "UID_" << blobID << " CX_" << ofToString(Xcentroid) << " CY_" << ofToString(Ycentroid)
      << " W_" << ofToString(boxW) << " H_" << ofToString(boxH) << " D_" << ofToString(boxD);

      if (Xcentroid > 0 || Ycentroid > 0){
        ofBoxPrimitive box;
        box.setMode(OF_PRIMITIVE_TRIANGLES);
        box.setResolution(1);
        box.set(boxW, boxH, (int)(boxD/4));
        box.setPosition(Xcentroid - boxW*0.5, Ycentroid - boxH*0.5, boxH/2);
        box.drawWireframe();
        //box.draw();
      }
      else {
        blobs.erase(blobs.begin() + index);
      }
    }
  }

  //ofPushMatrix();
  //ofTranslate(0, 0);
  //ofRotateDeg(4, 80, 0, 0);
  //ofPopMatrix();
  blobs.clear();
  //E256_blobsRequest();
}


// E256 matrix sensor - Calibration
void ofApp::E256_setCaliration(void) {
  ofxOscMessage OSCmsg;
  OSCmsg.setAddress("/calibrate");
  OSCmsg.addInt32Arg(20); // Set calibration cycles

  osc::OutboundPacketStream packet(buffer, OUTPUT_BUFFER_SIZE);
  packet.Clear();
  //packet << osc::BeginBundleImmediate;
  packet << osc::BeginMessage(OSCmsg.getAddress().data());
  packet << OSCmsg.getArgAsInt32(0);
  packet << osc::EndMessage;
  //packet << osc::EndBundle;

  serialDevice.send(ByteBuffer(packet.Data(), packet.Size()));
  ofLogNotice("ofApp::E256_setCaliration") << "E256 - Calibration requested : " << OSCmsg.getArgAsInt32(0);
}

// E256 matrix sensor - Set THRESHOLD
void ofApp::E256_setTreshold(int & tresholdValue) {

  ofxOscMessage OSCmsg;
  OSCmsg.setAddress("/threshold");
  OSCmsg.addInt32Arg((int32_t)tresholdValue);

  osc::OutboundPacketStream packet(buffer, OUTPUT_BUFFER_SIZE);
  packet.Clear();
  packet << osc::BeginBundleImmediate;
  packet << osc::BeginMessage(OSCmsg.getAddress().data());
  packet << OSCmsg.getArgAsInt32(0);
  packet << osc::EndMessage;
  packet << osc::EndBundle;

  serialDevice.send(ByteBuffer(packet.Data(), packet.Size()));
  ofLogNotice("ofApp::E256_setTreshold") << "E256 - Threshold seted : " << OSCmsg.getArgAsInt32(0);
}

// E256 matrix sensor - Blobs request
void ofApp::E256_blobsRequest(void) {
  ofxOscMessage OSCmsg;
  OSCmsg.setAddress("/blobs");
  osc::OutboundPacketStream packet(buffer, OUTPUT_BUFFER_SIZE);
  packet.Clear();
  packet << osc::BeginMessage(OSCmsg.getAddress().data());
  packet << osc::EndMessage;
  serialDevice.send(ByteBuffer(packet.Data(), packet.Size()));
  ofLogNotice("ofApp::E256_blobsRequest") << "E256 - Blobs requested : " << packet.Data();
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
    calirationButton.removeListener(this, &ofApp::E256_setCaliration);
    tresholdValue.removeListener(this, &ofApp::E256_setTreshold);
    serialDevice.unregisterAllEvents(this);
}
