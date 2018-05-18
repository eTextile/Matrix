#include <cmath>
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

    ofBackground(0);

    for (int y=0; y<Y_NEWSIZE; y++) {
        for (int x=0; x<X_NEWSIZE; x++) {
            mesh.addVertex(ofPoint(x, y, 0));             // make a new vertex
            mesh.addColor(ofFloatColor(255, 255, 255));   // set vertex color to white
        }
    }

    for (int y=0; y<Y_NEWSIZE-1; y++) {
        for (int x=0; x<X_NEWSIZE-1; x++) {
            int i1 = x + y * X_NEWSIZE;           // 0, 1, 2, 3, 4
            int i2 = (x+1) + y * X_NEWSIZE;       // 1, 2, 3, 4,
            int i3 = x + (y+1) * X_NEWSIZE;       // 18, 19,
            int i4 = (x+1) + (y+1) * X_NEWSIZE;
            mesh.addTriangle(i1, i2, i4);
            mesh.addTriangle(i1, i3, i4);
        }
    }

    // Initialize blob tracking
    grayImage.allocate(X_NEWSIZE, Y_NEWSIZE);
    grayBg.allocate(X_NEWSIZE, Y_NEWSIZE);
    grayDiff.allocate(X_NEWSIZE, Y_NEWSIZE);

    ofLogNotice("ofApp::setup") << "Teensy frame request";

    ofxOscMessage msg;
    msg.setAddress("/calibrate"); // Calibrate the matrix sensor
    sender.sendMessage(msg);

    bLearnBakground = true;
}

/////////////////////// SERIAL EVENT ///////////////////////
// https://github.com/workergnome/ofxOscuino
void ofApp::onSerialBuffer(const SerialBufferEventArgs& args) {
    if(serial.isInitialized() && oscBridge.update()) {
      while(receiver.hasWaitingMessages()) {
        ofxOscBundle bundle;
        bundle = receiver.getBundleAt(0);
        ofLog(OF_LOG_NOTICE) << "address:" << bundle.getAddress() << ", message: " << bundle.getArgAsInt32(0);
        // serialData
      }
    }
    msg.setAddress("/rowData");   // Raw frame request
    sender.sendMessage(msg);
    newFrame = true;
}

/////////////////////// UPDATE ///////////////////////
void ofApp::update() {

    if (newFrame) {

        newFrame = false;
        /////////////////// INTERPOLATE
        interpolatedFrame.setFromPixels(serialData, ROWS, COLS, OF_PIXELS_GRAY);
        interpolatedFrame.resize(X_NEWSIZE, Y_NEWSIZE, OF_INTERPOLATE_BICUBIC);
        grayImage.setFromPixels(interpolatedFrame.getData(), X_NEWSIZE, Y_NEWSIZE);

        if (bLearnBakground) {
            grayBg = grayImage; // the = sign copys the pixels from grayImage into grayBg (operator overloading)
            bLearnBakground = false;
        }
        // Take the abs value of the difference between background and incoming and then threshold:
        grayDiff.absDiff(grayBg, grayImage);
        grayDiff.threshold(threshold);
        // Find contours which are between the size of 5 pixels ($1) and 1500 pixels ($2).
        // Also, find holes is set to false ($4) so it will not get interior contours.
        contourFinder.findContours(grayDiff, 5, 1500, 8, false, true);

        vector<centroid> currentCentroids;

        for (uint32_t i=0; i<contourFinder.blobs.size(); i++) {
            float posX = contourFinder.blobs[i].centroid.x;    // Get blob posX
            float posY = contourFinder.blobs[i].centroid.y;    // Get blob posY
            float blobPerimeter = contourFinder.blobs[i].length;
            int index = int(posY) * X_NEWSIZE + int(posX);     // Calculate the index
            int posZ = grayImageCopy[index];                   // Get the Z coordinate
            if (DEBUG_PRINT) {
                ofLog() << "BlobID: " << int(i) << " posX: " << int(posX) << " posY: " << int(posY) << " posZ: " << posZ << endl;
            }
            centroid c = {};
            c.position = ofVec2f(posX, posY);
            c.pressure = posZ;
            c.perimeter = blobPerimeter;
            c.UID = i; // Do we nead to set it to -1 (NULL)?
            c.isDead = false;
            currentCentroids.push_back(c);
            if (DEBUG_PRINT) {
                ofLog() << "current frame -> " << ofToString(i) << " [" << c.position << "]";
            }
        }

        vector<centroid> oldCentroidsToUpdate;
        vector<centroid> centroidsToUpdate;
        vector<centroid> centroidsToAdd;

        ofxOscBundle bundle;
        ofxOscMessage message;
        message.setAddress("/sensors");

        for(uint32_t i = 0; i < currentCentroids.size(); i++ ) {
            float minDist = 1000;
            int nearestIndex = -1;
            for(uint32_t j = 0; j < centroids.size(); j++) {
                float dist = centroids[j].position.distance(currentCentroids[i].position);
                if(dist < minDist) {
                    minDist = dist;
                    nearestIndex = j;
                }
            }
            if (minDist < 10) {
                oldCentroidsToUpdate.push_back(centroids[nearestIndex]);
                centroidsToUpdate.push_back(currentCentroids[i]);
            }
            else {
                bool reachEnd = false;
                int minID = 0;
                while (!reachEnd) {
                    reachEnd = true;
                    for(uint32_t j = 0; j < centroids.size(); j++) {
                        if(centroids[j].UID == minID) {
                            minID ++;
                            reachEnd = false;
                            break;
                        }
                    }
                }
                currentCentroids[i].UID = minID;
                centroidsToAdd.push_back(currentCentroids[i]);
            }
        }

        for(uint32_t j = 0; j < centroids.size(); j++) {
            bool found = false;
            for(uint32_t i = 0; i < centroidsToUpdate.size(); i++) {
                if(oldCentroidsToUpdate[i].UID == centroids[j].UID) {
                    found = true;
                    centroids[j].position = centroidsToUpdate[i].position;
                    centroids[j].pressure = centroidsToUpdate[i].pressure;
                    centroids[j].perimeter = centroidsToUpdate[i].perimeter;
                }
            }
            if (!found) {
                centroids[j].isDead = true;
                message.addIntArg(centroids[j].UID);
                message.addIntArg(-1);
                message.addIntArg(-1);
                message.addIntArg(-1);
                message.addIntArg(-1);
                bundle.addMessage(message);
            }
        }

        bool deadExists = true;
        while(deadExists) {
            int index = -1;
            for(uint32_t i = 0; i < centroids.size(); i++) {
                if(centroids[i].isDead) {
                    index = i;
                    break;
                }
            }
            if(index != -1) {
               centroids.erase(centroids.begin() + index);
            }
            else {
                deadExists = false;
            }
        }

        for(uint32_t i = 0; i < centroidsToAdd.size(); i++ ) {
            centroids.push_back(centroidsToAdd[i]);
        }
        for(uint32_t i = 0; i < centroids.size(); i++) {
            if(!centroids[i].isDead) {
                message.addIntArg(centroids[i].UID);
                message.addIntArg(centroids[i].position.x);
                message.addIntArg(centroids[i].position.y);
                message.addFloatArg(centroids[i].pressure);
                message.addFloatArg(centroids[i].perimeter);
                bundle.addMessage(message);
                if (DEBUG_OSC) {
                    cout <<
                        "    BlobID : " << centroids[i].UID <<
                        "      posX : " << centroids[i].position.x <<
                        "      posY : " << centroids[i].position.y <<
                        "      posZ : " << (int)centroids[i].pressure <<
                        " perimeter : " << (int)centroids[i].perimeter <<
                    endl;
                }
            }
        }
        sender.sendBundle(bundle);
    }

    //////// Change vertices by the matrix sensors values
    for (int index=0; index<X_NEWSIZE*Y_NEWSIZE; index++) {
        ofPoint p = mesh.getVertex(index);             // Get the point coordinates
        grayImageCopy = grayImage.getPixels();         // Get the pixel value from the image
        p.z = - grayImageCopy[index];                  // Change the z-coordinates
        mesh.setVertex(index, p);                      // Set the new coordinates
        mesh.setColor(index, ofColor(grayImageCopy[index], 0, 255));    // Change vertex color
    }
    handleOSC();
}


//////////////////////////////////////////// DRAW ////////////////////////////////////////////
void ofApp::draw() {

    ofBackground(0);
    ofSetColor(255);
    gui.draw();

    std::stringstream ss;
    ss << "         FPS : " << (int) ofGetFrameRate() << std::endl;
    ss << "Connected to : " << device.getPortName() << std::endl;
    ss << "OSC out port : " << UDP_OUTPUT_PORT << std::endl;
    ss << " OSC in port : " << UDP_INPUT_PORT << std::endl;
    ss << "       Blobs : " << contourFinder.nBlobs << std::endl;

    ofDrawBitmapString(ss.str(), ofVec2f(20, 200)); // Draw the GUI menu

    // grayImage.draw(20, 300);
    // grayBg.draw(20, 400);
    // grayDiff.draw(20, 500);

    ofPushMatrix();

    ofTranslate(360, 99);
    ofRotate(25, 1, 0, 0);

    //////////////////////// DRAW BLOBS
    const int x = 0;  // X ofset
    const int y = 0;  // Y ofset FIXME : dont afect the matrix graph
    const int k = 9;  // Scale factor TODO : make it interactive

    ofNoFill();
    ofSetLineWidth(3);
    ofSetColor(255, 0, 0);

    for (int i=0; i<contourFinder.nBlobs; i++) {
        blob = contourFinder.blobs[i];
        ofBeginShape();
        for (int i=0; i<blob.nPts; i++) {
            ofVertex(x+k*blob.pts[i].x, y+k*blob.pts[i].y);
        }
        ofEndShape(true);
    }

    ofSetLineWidth(1);    // set line width to 1
    ofScale(k, k);

    mesh.drawWireframe(); // draws lines

    //////////////////////// DRAW CENTROIDS
    ofSetColor(255);
    for(uint32_t i = 0; i < centroids.size(); i++) {
        if( !centroids[i].isDead ) {
            ofDrawBitmapString(ofToString(centroids[i].UID), centroids[i].position);
        }
    }
    ofPopMatrix();
}


void ofApp::onSerialError(const SerialBufferErrorEventArgs& args) {

    // Errors and their corresponding buffer (if any) will show up here.
    cout << args.getException().displayText() << endl;
}

void ofApp::handleOSC() {

    while(receiver.hasWaitingMessages()) {
        ofxOscMessage m;
        receiver.getNextMessage(m);
        if(m.getAddress() == "/point") {
            int row = m.getArgAsFloat(0)*ROWS;
            int col = m.getArgAsFloat(1)*COLS;
            int startIndex = col * COLS + row;
            serialData[startIndex] = (uint8_t)ofMap(m.getArgAsFloat(2), 0, 1, 0, 255);
            if (DEBUG_PRINT) {
                cout << "NEW VIRTUAL POINT [ "<< row <<","<< col <<","<< unsigned(storedValueRast[startIndex]) << "]"<<endl;
            }
            newFrame = true;
        }
        else if(m.getAddress() == "/reset") {
            memset(storedValueRast, 0, ROWS*COLS);
            newFrame = true;
        }
    }
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
