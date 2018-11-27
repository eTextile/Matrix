#include "ofApp.h"

void ofApp::setup() {

    ofSetVerticalSync(true);
    ofSetWindowTitle("E256 - eTextile matrix sensor");

    std::vector<SerialDeviceInfo> devicesInfo = SerialDeviceUtils::listDevices();
    ofLogNotice("ofApp::setup") << "Connected Devices: ";

    for (std::size_t i = 0; i < devicesInfo.size(); ++i) {
        ofLogNotice("ofApp::setup") << "\t" << devicesInfo[i];
    }

    if (!devicesInfo.empty()) {
        bool success = device.setup(USB_PORT, BAUD_RATE);
        if (success) {
            device.registerAllEvents(this);
            ofLogNotice("ofApp::setup") << "Successfully setup: " << USB_PORT;
        }
        else {
            ofLogNotice("ofApp::setup") << "Unable to setup: " << USB_PORT;
        }
    }
    else {
        ofLogNotice("ofApp::setup") << "No devices connected!";
    }

    sender.setup(HOST, UDP_OUTPUT_PORT); // OSC - UDP config
    receiver.setup(UDP_INPUT_PORT);
    toggleDsp.addListener(this, &ofApp::toggleDspPressed);
    sliderVolume.addListener(this, &ofApp::sliderVolumeValue);
    sliderTreshold.addListener(this, &ofApp::sliderTresholdValue);

    gui.setup("Parameters");
    gui.add(toggleDsp.setup(" Dsp ", true ));
    gui.add(sliderVolume.setup(" Volume ", 30, 0, 50));
    gui.add(sliderTreshold.setup(" Threshold ", 12, 0, 100));

    static const int OUTPUT_BUFFER_SIZE = 8192;
    char buffer[OUTPUT_BUFFER_SIZE];
    osc::OutboundPacketStream packet(buffer, OUTPUT_BUFFER_SIZE);

    // E256 matrix sensor - Calibration
    packet << osc::BeginBundleImmediate;
    packet << osc::BeginMessage("/calibrate");
    packet << osc::EndMessage;
    packet << osc::EndBundle;
    device.send(ofx::IO::ByteBuffer(packet.Data(), packet.Size()));
    ofLogNotice("ofApp::setup") << "E256 calibration requested : " << packet.Data();
    packet.Clear();

    // E256 matrix sensor - Blobs request
    packet << osc::BeginBundleImmediate;
    packet << osc::BeginMessage("/blobs");
    packet << osc::EndMessage;
    packet << osc::EndBundle;
    device.send(ofx::IO::ByteBuffer(packet.Data(), packet.Size()));
    ofLogNotice("ofApp::setup") << "E256 frame requested : " << packet.Data();
    packet.Clear();

    /*
    // E256 matrix sensor - Row datas request
    packet << osc::BeginBundleImmediate;
    packet << osc::BeginMessage("/rowData");
    packet << osc::EndMessage;
    packet << osc::EndBundle;
    device.send(ofx::IO::ByteBuffer(packet.Data(), packet.Size()));
    ofLogNotice("ofApp::setup") << "E256 rowData frame requested : " << packet.Data();
    packet.Clear();
    */

    ofBackground(0);
}

/////////////////////// SERIAL EVENT ///////////////////////
void ofApp::onSerialBuffer(const ofxIO::SerialBufferEventArgs& args) {

    SerialMessage message;
    message.message = args.buffer().toString();
    ofLogNotice("ofApp::onSerialBuffer") << "got serial buffer : " << message.message;
    serialMessages.push_back(message);


    static const int OUTPUT_BUFFER_SIZE = 8192;
    char buffer[OUTPUT_BUFFER_SIZE];
    osc::OutboundPacketStream packet(buffer, OUTPUT_BUFFER_SIZE);

    // E256 matrix sensor - Blobs request
    packet << osc::BeginBundleImmediate;
    packet << osc::BeginMessage("/blobs");
    packet << osc::EndMessage;
    packet << osc::EndBundle;
    device.send(ofx::IO::ByteBuffer(packet.Data(), packet.Size()));
    ofLogNotice("ofApp::onSerialBuffer") << "E256 frame requested : " << packet.Data();
    packet.Clear();

    frameRequest = true;
}


/////////////////////// UPDATE ///////////////////////
void ofApp::update() {

  if (frameRequest) {

    std::vector<SerialMessage>::iterator iter = serialMessages.begin();

    while (iter != serialMessages.end()){
      if (iter->UID){
        iter = serialMessages.erase(iter);
      } else {
        if (!iter->exception.empty()){
        }
        ++iter;
      }
    }
    frameRequest = false;
  }
}

//////////////////////////////////////////// DRAW ////////////////////////////////////////////
void ofApp::draw() {

    ofBackground(0);
    ofSetColor(255);
    gui.draw();

    std::stringstream ss;
    ss << "     Connected to : " << device.getPortName() << std::endl;
    ss << "SLIP-OSC-OUT port : " << UDP_OUTPUT_PORT << std::endl;
    ss << " SLIP-OSC-IN port : " << UDP_INPUT_PORT << std::endl;
    ss << "              FPS : " << (int) ofGetFrameRate() << std::endl;
    ofDrawBitmapString(ss.str(), ofVec2f(20, 200)); // Draw the GUI menu

    ofPushMatrix();
    // TODO
    ofPopMatrix();
}

void ofApp::onSerialError(const SerialBufferErrorEventArgs& args) {
    // Errors and their corresponding buffer (if any) will show up here.
    cout << args.getException().displayText() << endl;
}

void ofApp::exit() {
    device.unregisterAllEvents(this);
    toggleDsp.removeListener(this, &ofApp::toggleDspPressed);
    sliderVolume.removeListener(this, &ofApp::sliderVolumeValue);
    sliderTreshold.removeListener(this, &ofApp::sliderTresholdValue);
}

//--------------------------------------------------------------
void ofApp::toggleDspPressed(bool & toggleState) {

    ofxOscMessage message;
    message.setAddress("/dsp");
    if (toggleState) {
        message.addIntArg(1);
    } else {
        message.addIntArg(0);
    }
    sender.sendMessage(message);
}
//--------------------------------------------------------------
void ofApp::sliderVolumeValue(float & sliderValue_A) {
    ofxOscMessage message;
    message.setAddress("/volume");
    message.addFloatArg(sliderValue_A);
    sender.sendMessage(message);
}
//--------------------------------------------------------------
void ofApp::sliderTresholdValue(float & sliderValue_B) {
    threshold = sliderValue_B;
    ofxOscMessage message;
    message.setAddress("/threshold");
    message.addFloatArg(sliderValue_B);
    sender.sendMessage(message);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
    switch(key) {
        case 'f':
            ofToggleFullscreen();
            break;
        default:
            break;
    }
}
