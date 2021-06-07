/*
  This file is part of the E256 - eTextile matrix sensor project - http://matrix.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "transmit_osc.h"

#if USB_SLIP_OSC

SLIPEncodedUSBSerial SLIPSerial(thisBoardsSerialUSB);

void USB_SLIP_OSC_SETUP(void) {
  SLIPSerial.begin(BAUD_RATE);
}

void usb_slipOsc(preset_t* presets_ptr, image_t* rawFrame_ptr, image_t*interpFrame_ptr, llist_t* blobs_ptr) {

  OSCMessage request;

  int size;

  while (!SLIPSerial.endofPacket()) {
    if ((size = SLIPSerial.available()) > 0) {
      while (size--)
        request.fill(SLIPSerial.read());
    }
  }

  if (!request.hasError()) {
    if (request.fullMatch("/c")) { // Calibrate
      lastMode = currentMode;
      currentMode = CALIBRATE;
      presets_ptr[CALIBRATE].setLed = true;
      presets_ptr[CALIBRATE].updateLed = true;
    }
    else if (request.fullMatch("/t")) { // Set threshold
      /*
        presets_ptr[THRESHOLD].ledVal = map(presets_ptr[THRESHOLD].val, presets_ptr[THRESHOLD].minVal, presets_ptr[THRESHOLD].maxVal, 0, 255);
        interpThreshold = constrain(presets_ptr[THRESHOLD].val - 5, presets_ptr[THRESHOLD].minVal, presets_ptr[THRESHOLD].maxVal);
        presets_ptr[THRESHOLD].updateLed = true;
        presets_ptr[THRESHOLD].update = true;
      */
    }
    else if (request.fullMatch("/r")) { // Get raw datas
      OSCMessage m("/r");
      m.add(rawFrame_ptr->pData, RAW_FRAME);
      SLIPSerial.beginPacket();
      m.send(SLIPSerial);
      SLIPSerial.endPacket();
    }
    else if (request.fullMatch("/i")) { // Get interp
      OSCMessage m("/i");
      m.add(interpFrame_ptr->pData, NEW_FRAME);
      SLIPSerial.beginPacket();
      m.send(SLIPSerial);
      SLIPSerial.endPacket();
    }
    else if (request.fullMatch("/b")) { // Get blobs
      OSCBundle OSCbundle;
      for (blob_t* blob_ptr = (blob_t*)ITERATOR_START_FROM_HEAD(blobs_ptr); blob_ptr != NULL; blob_ptr = (blob_t*)ITERATOR_NEXT(blob_ptr)) {
        OSCMessage msg("/b");
        /*
        msg.add(blob_ptr->UID);
        msg.add(blob_ptr->state);
        msg.add(blob_ptr->lastState);
        msg.add(blob_ptr->centroid.X);
        msg.add(blob_ptr->centroid.Y);
        msg.add(blob_ptr->box.W);
        msg.add(blob_ptr->box.H);
        msg.add(blob_ptr->box.D);
        */
        msg.add((uint8_t*)blobs_ptr, 14);
        OSCbundle.add(msg);
      }
      SLIPSerial.beginPacket();     // Send SLIP header
      OSCbundle.send(SLIPSerial);   // Send the OSC bundle
      SLIPSerial.endPacket();       // Send the SLIP end of packet
    }
  }
}
#endif
