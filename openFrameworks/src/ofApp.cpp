/*
This software is made for the eTextile matrix sensor.
This sensor is open hardware but it use some materials that are complicate to buy.
For exempel the piedzo-resistive fabric use to sens the presure is not accesibel in small cantity.
Also the multiconductors ribbon (16 lignes) do not exist yet as a market product and have to be make by an industry.
*/

#include <cmath>
#include "ofApp.h"

#define DEBUG_SERIAL 0
#define DEBUG_PRINT 0
#define DEBUG_OSC 1

void ofApp::setup() {

    ofSetWindowTitle("eTextile matrix sensor");
    ofSetFrameRate(60);
    // 1. Upload the PacketSerialReverseEcho.ino sketch (in this example's
    //    Arduino/ folder) to an Arduino board.  This sketch requires
    //    the Arduino PacketSerial library https://github.com/bakercp/PacketSerial
    // 2. Check the "listDevices" call below to make sure the correct serial
    //    device is connected.
    // 3. Run this app.

    std::vector<SerialDeviceInfo> devicesInfo = SerialDeviceUtils::listDevices();
    ofLogNotice("ofApp::setup") << "Connected Devices: ";

    for (std::size_t i = 0; i < devicesInfo.size(); ++i) {
        ofLogNotice("ofApp::setup") << "\t" << devicesInfo[i];
    }

    if (!devicesInfo.empty()) {
        // Connect to the first device (ttyACM0).
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
    sliderTransposition.addListener(this, &ofApp::sliderTranspositionValue);
    buttonModeA.addListener(this, &ofApp::buttonModePressedA);

    gui.setup("Parameters");
    gui.add(toggleDsp.setup(" Dsp ", true ));
    gui.add(sliderVolume.setup(" Volume ", 0.8, 0, 1));
    gui.add(sliderTransposition.setup(" Threshold ", 100, 0, 150));
    gui.add(buttonModeA.setup(" RAZ "));

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

    // Initialize buffer
    ofLogNotice("ofApp::setup") << "Ping the Teensy";
    buffer.writeByte(65); // "A"
    device.send(buffer);  // Request a frame from the Teensy matrix sensor
    bLearnBakground = true;
}

/////////////////////// SERIAL EVENT ///////////////////////
void ofApp::onSerialBuffer(const SerialBufferEventArgs& args) {

    // Copy the frame buffer (256 values) into serialData array.
    std::copy(args.getBuffer().begin(), args.getBuffer().end(), serialData);

    if (DEBUG_SERIAL) {
    cout << "NEW packet : ";
        for (int index=0; index<DATAS; index++) {
            cout << "SENSOR_ID: " << sensorID << " VALUE: " << serialData[index] << endl;
        }
    }
    device.send(buffer); // Request a frame from the Teensy matrix sensor
    newFrame = true;
}

//////////////////////////////////////////// UPDATE ////////////////////////////////////////////
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
        // Find contours which are between the size of 9 pixels ($1) and 100 pixels ($2).
        // Also, find holes is set to false ($4) so it will not get interior contours.
        contourFinder.findContours(grayDiff, 9, 300, 8, false, true);


        vector<centroid> currentCentroids;
        for (int i=0; i<contourFinder.blobs.size(); i++) {
            float posX = contourFinder.blobs[i].centroid.x;    // Get blob posX
            float posY = contourFinder.blobs[i].centroid.y;    // Get blob posY
            int index = int(posY) * X_NEWSIZE + int(posX);     // Calculate the index
            int posZ = grayImageCopy[index];                   // Get the Z coordinate
            if (DEBUG_PRINT) {
                ofLog() << "BlobID: " << int(i) << " posX: " << int(posX) << " posY: " << int(posY) << " posZ: " << posZ << endl;
            }
            centroid c = {};
            c.position = ofVec2f(posX, posY);
            c.pressure = posZ;
            c.UID = i;
            c.isDead = false;
            currentCentroids.push_back(c);
            if (DEBUG_PRINT) {
                ofLog()<<"current frame -> "<<ofToString(i)<<" ["<<c.position<<"]";
            }
        }

        vector<centroid> oldCentroidsToUpdate;
        vector<centroid> centroidsToUpdate;
        vector<centroid> centroidsToAdd;

        for(int i = 0; i < currentCentroids.size(); i++ ) {
            float minDist = 1000;
            int nearestIndex = -1;
            for(int j = 0; j < centroids.size(); j++) {
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
                    for(int j = 0; j < centroids.size(); j++) {
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


        vector<centroid> centroidsToRemove;
        vector<int> indexesToRemove;
        for(int j = 0; j < centroids.size(); j++) {
            bool found = false;
            for(int i = 0; i < centroidsToUpdate.size(); i++) {
                if(oldCentroidsToUpdate[i].UID == centroids[j].UID) {
                    found = true;
                    centroids[j].position = centroidsToUpdate[i].position;
                    centroids[j].pressure = centroidsToUpdate[i].pressure;
                }
            }
            if (!found) {
                centroids[j].isDead = true;
            }
        }

        bool deadExists = true;
        while(deadExists) {
            int index = -1;
            for(int i = 0; i < centroids.size(); i++) {
                if(centroids[i].isDead) {
                    index = i;
                    break;
                }
            }
            if(index != -1) {
               centroids.erase(centroids.begin() + index);
               // Add OSC erase flag -1
               centroids[index].pressure = -1;
            }
            else {
                deadExists = false;
            }
        }

        for(int i = 0; i < centroidsToAdd.size(); i++ ) {
            centroids.push_back(centroidsToAdd[i]);
        }
        for(int i = 0; i < centroids.size(); i++) {
            if(!centroids[i].isDead) {
                ofxOscMessage message;
                message.setAddress("/sensor");
                message.addIntArg(centroids[i].UID);
                message.addIntArg(centroids[i].position.x);
                message.addIntArg(centroids[i].position.y);
                message.addIntArg(centroids[i].pressure);
                if (DEBUG_OSC) {
                    cout << "BlobID: "<<centroids[i].UID<< " posX: " << centroids[i].position.x << " posY: " << centroids[i].position.y << " posZ: " << (int)centroids[i].pressure << endl;
                  }
                sender.sendMessage(message, false);
            }
        }
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

    ofTranslate(400, 50);
    //ofRotate(30, 1, 0, 0);

    //////////////////////// DRAW BLOBS
    const int x = 0;   // X ofset
    const int y = -50; // Y ofset
    const int k = 10;  // Scale

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
    ofScale(10, 9);

    mesh.drawWireframe(); // draws lines

    //////////////////////// DRAW CENTROIDS
    ofSetColor(255);
    for(int i = 0; i < centroids.size(); i++) {
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
    sliderTransposition.removeListener(this, &ofApp::sliderTranspositionValue);
    buttonModeA.removeListener(this, &ofApp::buttonModePressedA);
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
void ofApp::sliderTranspositionValue(float & sliderValue_B) {
    threshold = sliderValue_B;
}
//--------------------------------------------------------------
void ofApp::buttonModePressedA() {
    bLearnBakground = true;
    centroids.clear();
}

