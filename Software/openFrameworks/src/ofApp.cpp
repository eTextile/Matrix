#include "ofApp.h"

void ofApp::setup(void) {
  ofSetVerticalSync(true);
  ofSetWindowTitle("E256 - eTextile matrix sensor");

  ofSetLogLevel(OF_LOG_ERROR);
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
      BAUD_RATE,
      SerialDevice::DATA_BITS_EIGHT,
      SerialDevice::PAR_NONE,
      SerialDevice::STOP_ONE,
      SerialDevice::FLOW_CTRL_HARDWARE
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
  serialMessage oscMessage;
  oscMessage.OSCmessage = args.buffer().toString();
  serialMessages.push_back(oscMessage);
  ofLogNotice("ofApp::onSerialBuffer") << "E256 - OSCmessage : " << oscMessage.OSCmessage;
  E256_blobsRequest();
}

void ofApp::onSerialError(const ofxIO::SerialBufferErrorEventArgs& args) {
  serialMessage oscMessage;
  oscMessage.exception = args.exception().displayText();
  serialMessages.push_back(oscMessage);
  ofLogNotice("ofApp::onSerialError") << "E256 - Serial ERROR : " << args.exception().displayText();
}

/////////////////////// UPDATE ///////////////////////
void ofApp::update(void) {

for (size_t i = 0; i < serialMessages.size(); ++i){

  std::string oscMessages = serialMessages[i].OSCmessage;
  ofLogNotice("ofApp::update") << "serialMessage : "<< oscMessages;

  Poco::RegularExpression regex("/blob(.){35}"); // TODO adding /matrix
  Poco::RegularExpression::Match theMatch;

  size_t stringOffset = 0;
  int offset = 16;

  while (regex.match(oscMessages, stringOffset, theMatch)){

    stringOffset = theMatch.offset + theMatch.length;
    std::string OSCmsg = std::string(oscMessages, theMatch.offset, theMatch.length);

    /*
    for (size_t i=0; i<OSCmsg.size(); i++){
      ofLogNotice("ofApp::update") << "OSCmsg pos : " << i << " val : " << OSCmsg[i];
    }
    */

    ofxOscMessage oscMessage;
    oscMessage.setAddress("/blob");

    oscMessage.addInt32Arg((((uint8_t)OSCmsg[0+offset] << 24) | ((uint8_t)OSCmsg[1+offset] << 16) | ((uint8_t)OSCmsg[2+offset] << 8) | (uint8_t)OSCmsg[3+offset])); // blobID
    oscMessage.addInt32Arg((((uint8_t)OSCmsg[4+offset] << 24) | ((uint8_t)OSCmsg[5+offset] << 16) | ((uint8_t)OSCmsg[6+offset] << 8) | (uint8_t)OSCmsg[7+offset])); // posX
    oscMessage.addInt32Arg((((uint8_t)OSCmsg[8+offset] << 24) | ((uint8_t)OSCmsg[9+offset] << 16) | ((uint8_t)OSCmsg[10+offset] << 8) | (uint8_t)OSCmsg[11+offset])); // posY
    oscMessage.addInt32Arg((((uint8_t)OSCmsg[12+offset] << 24) | ((uint8_t)OSCmsg[13+offset] << 16) | ((uint8_t)OSCmsg[14+offset] << 8) | (uint8_t)OSCmsg[15+offset])); // boxW
    oscMessage.addInt32Arg((((uint8_t)OSCmsg[16+offset] << 24) | ((uint8_t)OSCmsg[17+offset] << 16) | ((uint8_t)OSCmsg[18+offset] << 8) | (uint8_t)OSCmsg[19+offset])); // boxH
    oscMessage.addInt32Arg((((uint8_t)OSCmsg[20+offset] << 24) | ((uint8_t)OSCmsg[21+offset] << 16) | ((uint8_t)OSCmsg[22+offset] << 8) | (uint8_t)OSCmsg[23+offset])); // bowD

    blobs.push_back(oscMessage);
    }
  }
  serialMessages.clear();
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

  for (size_t index = 0; index < blobs.size(); ++index){
    if(blobs[index].getAddress() == "/blob"){

      int blobID = blobs[index].getArgAsInt(0);
      int posX = blobs[index].getArgAsInt(1) * ofGetWidth();
      int posY = blobs[index].getArgAsInt(2) * ofGetHeight();
      //int posX = (int)((blobs[index].getArgAsInt(1) / 64) * ofGetWidth());
      //int posY = (int)((blobs[index].getArgAsInt(2) / 64) * ofGetHeight());
      int rectW = blobs[index].getArgAsInt(3);
      int rectH = blobs[index].getArgAsInt(4);
      int rectD = blobs[index].getArgAsInt(5);

      if (posX > -1){
        ofConePrimitive cone;

        ofSetColor(255);
        ofDrawCircle(posX - rectW*0.5, posY- rectH*0.5, rectW);
        box.set( boxHeight, boxWidth, boxDepth );
      }
      else {
        blobs.erase(blobs.begin() + index);
      }
    }
  }
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

// E256 matrix sensor - Set threshold
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
  //OSCbundle.clear();

  ofxOscMessage OSCmsg;
  OSCmsg.setAddress("/blobs");
  //OSCmsg.addInt32Arg(0);

  osc::OutboundPacketStream packet(buffer, OUTPUT_BUFFER_SIZE);
  packet.Clear();

  //packet << osc::BeginBundleImmediate;
  packet << osc::BeginMessage(OSCmsg.getAddress().data());
  //packet << OSCmsg.getArgAsInt32(0);
  packet << osc::EndMessage;
  //packet << osc::EndBundle;

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
