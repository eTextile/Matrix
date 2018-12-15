#include "ofApp.h"

void ofApp::setup() {

  ofSetVerticalSync(true);
  ofSetWindowTitle("E256 - eTextile matrix sensor");

  std::vector<SerialDeviceInfo> devicesInfo = SerialDeviceUtils::listDevices();
  ofLogNotice("ofApp::setup") << "Connected Devices: ";

  for (std::size_t i = 0; i < devicesInfo.size(); ++i) {
        ofLogNotice("ofApp::setup") << "\t" << devicesInfo[i];
  }

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
//receiver.setup(UDP_INPUT_PORT);

//calirationButton.addListener(this, &ofApp::setCaliration);
tresholdSlider.addListener(this, &ofApp::setTreshold);

gui.setup("E256 - Parameters");
gui.add(calirationButton.setup(" Calibrate "));
gui.add(tresholdSlider.setup(" Threshold ", 30, 0, 100));

// E256 matrix sensor - Blobs request
static const int OUTPUT_BUFFER_SIZE = 8192;
char buffer[OUTPUT_BUFFER_SIZE];
osc::OutboundPacketStream packet(buffer, OUTPUT_BUFFER_SIZE);

ofxOscMessage OSCmsg;
OSCmsg.setAddress("/blobs");
OSCmsg.addInt32Arg(1);	// Stet it to 1 to BEBUG
packet << osc::BeginMessage(OSCmsg.getAddress().data());
packet << OSCmsg.getArgAsInt32(0);
packet << osc::EndMessage;

serialDevice.send(ofx::IO::ByteBuffer(packet.Data(), packet.Size()));
if (DEBUG_SERIAL) ofLogNotice("ofApp::setup") << "E256 - Frame requested : " << OSCmsg.getAddress().data();

ofBackground(0);
}

/////////////////////// SERIAL EVENT ///////////////////////
void ofApp::onSerialBuffer(const SerialBufferEventArgs& args) {

  serialMessage_t message;
  message.OSCmessage = args.buffer().toString();
  serialMessages.push_back(message);

  // E256 matrix sensor - Blobs request
  static const int OUTPUT_BUFFER_SIZE = 255;
  char buffer[OUTPUT_BUFFER_SIZE];
  osc::OutboundPacketStream packet(buffer, OUTPUT_BUFFER_SIZE);

  ofxOscMessage OSCmsg;
  OSCmsg.setAddress("/blobs");
  OSCmsg.addInt32Arg(1);
  packet << osc::BeginMessage(OSCmsg.getAddress().data());
  packet << OSCmsg.getArgAsInt32(0);
  packet << osc::EndMessage;

  serialDevice.send(ofx::IO::ByteBuffer(packet.Data(), packet.Size()));
  if (DEBUG_SERIAL) ofLogNotice("ofApp::onSerialBuffer") << "E256 - Frame requested : " << packet.Data();
  packet.Clear();

  //frameRequest = true;
}

void ofApp::onSerialError(const SerialBufferErrorEventArgs& args) {
  // Errors and their corresponding buffer (if any) will show up here.
  serialMessage_t message;
  message.exception = args.exception().displayText();
  serialMessages.push_back(message);
  if (DEBUG_SERIAL) ofLogNotice("ofApp::onSerialError") << "E256 - Serial ERROR : " << args.exception().displayText();
}

/////////////////////// UPDATE ///////////////////////
void ofApp::update(void) {

  auto message = serialMessages.begin();
  while (message != serialMessages.end()) {
    //ofLogNotice("ofApp::update") << "E256 - OSCmessage : " << message->OSCmessage;
    std::vector<string> splitString = ofSplitString( message->OSCmessage, "$");

      for (size_t i=1; i<splitString.size(); i++){

        int32_t blobID = (((uint8_t)splitString[i][16] << 24) | ((uint8_t)splitString[i][17] << 16) | ((uint8_t)splitString[i][18] << 8) | (uint8_t)splitString[i][19]);
        int32_t posX = (((uint8_t)splitString[i][20] << 24) | ((uint8_t)splitString[i][21] << 16) | ((uint8_t)splitString[i][22] << 8) | (uint8_t)splitString[i][23]);
        int32_t posY = (((uint8_t)splitString[i][24] << 24) | ((uint8_t)splitString[i][25] << 16) | ((uint8_t)splitString[i][26] << 8) | (uint8_t)splitString[i][27]);
        int32_t posZ = (((uint8_t)splitString[i][28] << 24) | ((uint8_t)splitString[i][29] << 16) | ((uint8_t)splitString[i][30] << 8) | (uint8_t)splitString[i][31]);
        int32_t pixels = (((uint8_t)splitString[i][32] << 24) | ((uint8_t)splitString[i][33] << 16) | ((uint8_t)splitString[i][34] << 8) | (uint8_t)splitString[i][35]);

        if (DEBUG_SERIAL) ofLogNotice("ofApp::update") << "E256 - blob : " << blobID << " " << posX << " " << posY << " " << posZ << " " << pixels;

        blob_t blob;
        blob.blobID = (uint8_t)blobID;
        blob.posX = (int8_t)posX;
        blob.posY = (int8_t)posY;
        blob.posZ = (int8_t)posZ;
        blob.pixels = (int16_t)pixels;

        blobs.push_back(blob);
      }
      //message = serialMessages.erase(message);
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

    auto iter = blobs.begin();
    while (iter != blobs.end()) {
       if (DEBUG_DRAW) ofLogNotice("ofApp::draw") << "E256 - blob : "
       << iter->blobID << ' ' << iter->posX << '_' << iter->posY << '_' << iter->posZ << '_' << iter->pixels;
    }

    // TODO
    //ofPushMatrix();
    //ofPopMatrix();
    blobs.clear();
}


// E256 matrix sensor - Calibration
void ofApp::setCaliration(bool & buttonState) {

  static const int OUTPUT_BUFFER_SIZE = 255;
  char buffer[OUTPUT_BUFFER_SIZE];
  osc::OutboundPacketStream packet(buffer, OUTPUT_BUFFER_SIZE);

  ofxOscMessage OSCmsg;
  OSCmsg.setAddress("/calibrate");
  OSCmsg.addInt32Arg(20); // Set calibration cycles

  //packet << osc::BeginBundleImmediate;
  packet << osc::BeginMessage(OSCmsg.getAddress().data());
  packet << OSCmsg.getArgAsInt32(0);
  packet << osc::EndMessage;
  //packet << osc::EndBundle;

  serialDevice.send(ofx::IO::ByteBuffer(packet.Data(), packet.Size()));
  if (DEBUG_SERIAL) ofLogNotice("ofApp::setup") << "E256 - Calibration requested : " << OSCmsg.getArgAsInt32(0);
  //packet.Clear();
}

// E256 matrix sensor - Set threshold
void ofApp::setTreshold(int & sliderValue) {

  static const int OUTPUT_BUFFER_SIZE = 255;
  char buffer[OUTPUT_BUFFER_SIZE];
  osc::OutboundPacketStream packet(buffer, OUTPUT_BUFFER_SIZE);

  ofxOscMessage OSCmsg;
  OSCmsg.setAddress("/threshold");
  OSCmsg.addInt32Arg(sliderValue);

  //packet << osc::BeginBundleImmediate;
  packet << osc::BeginMessage(OSCmsg.getAddress().data());
  packet << OSCmsg.getArgAsInt32(0);
  packet << osc::EndMessage;
  //packet << osc::EndBundle;

  serialDevice.send(ofx::IO::ByteBuffer(packet.Data(), packet.Size()));
  if (DEBUG_SERIAL) ofLogNotice("ofApp::onSetParameters") << "E256 - Set threshold : " << OSCmsg.getArgAsInt32(0);
  //packet.Clear();
}

void ofApp::exit() {
    serialDevice.unregisterAllEvents(this);
    calirationButton.removeListener(this, &ofApp::setCaliration);
    tresholdSlider.removeListener(this, &ofApp::setTreshold);
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
