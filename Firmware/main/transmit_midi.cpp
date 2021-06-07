/*
  This file is part of the E256 - eTextile matrix sensor project - http://matrix.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "transmit_midi.h"

midiNode_t midiInArray[MAX_SYNTH] = {0}; // 1D Array to alocate memory for incoming midi notes
llist_t  midiNodes_stack;                // Midi nodes stack

#if USB_MIDI

void USB_MIDI_SETUP(void) {
  usbMIDI.begin();
}

void midi_llist_init(llist_t* nodes_ptr, midiNode_t* nodeArray_ptr) {
  llist_raz(nodes_ptr);
  for (int i = 0; i < MAX_SYNTH; i++) {
    llist_push_front(nodes_ptr, &nodeArray_ptr[i]);
  }
}

boolean handleMidiInput(llist_t* llist_ptr) {
  if (usbMIDI.read()) {                   // Is there a MIDI message incoming
    switch (usbMIDI.getType()) {
      case usbMIDI.NoteOn:
        midiNode_t* midiNode = (midiNode_t*)llist_pop_front(&midiNodes_stack);
        midiNode->pithch = usbMIDI.getData1();
        midiNode->velocity = usbMIDI.getData2();
        midiNode->channel = usbMIDI.getChannel();
        llist_push_front(llist_ptr, midiNode);
        break;
      case usbMIDI.NoteOff:
        midiNode_t* prevNode_ptr = NULL;
        for (midiNode_t* midiNode = (midiNode_t*)ITERATOR_START_FROM_HEAD(llist_ptr); midiNode != NULL; midiNode = (midiNode_t*)ITERATOR_NEXT(midiNode)) {
          if (midiNode->pithch == usbMIDI.getData1()) {
            llist_extract_node(llist_ptr, prevNode_ptr, midiNode);
            llist_push_front(&midiNodes_stack, midiNode);
            break;
          };
          prevNode_ptr = midiNode;
        };
        break;
      default:
        break;
    };
    return true;
  }
  else {
    return false;
  };
};

// Send blobs values using ControlChange MIDI format
// Send only the last blob that have been added to the sensor surface
// Separate blob's values according to the encoder position to allow the mapping into Max4Live
void usb_midi_learn(llist_t* llist_ptr, preset_t* preset_ptr) {
  if (llist_ptr->tail_ptr != NULL) {
    blob_t* tailBlob_ptr = (blob_t*)llist_ptr->tail_ptr;
    switch (preset_ptr->val) {
      case BS:
        usbMIDI.sendControlChange(BS, tailBlob_ptr->state, tailBlob_ptr->UID + 1);
        break;
      case BX:
        usbMIDI.sendControlChange(BX, (uint8_t)round(map(tailBlob_ptr->centroid.X, 0.0, 59.0, 0, 127)), tailBlob_ptr->UID + 1);
        break;
      case BY:
        usbMIDI.sendControlChange(BY, (uint8_t)round(map(tailBlob_ptr->centroid.Y, 0.0, 59.0, 0, 127)), tailBlob_ptr->UID + 1);
        break;
      case BW:
        usbMIDI.sendControlChange(BW, tailBlob_ptr->box.W, tailBlob_ptr->UID + 1);
        break;
      case BH:
        usbMIDI.sendControlChange(BH, tailBlob_ptr->box.H, tailBlob_ptr->UID + 1);
        break;
      case BD:
        usbMIDI.sendControlChange(BD, constrain(tailBlob_ptr->box.D, 0, 127), tailBlob_ptr->UID + 1);
        break;
    }
    while (usbMIDI.read()); // Read and discard any incoming MIDI messages
  }
}

// Send all blobs values using ControlChange MIDI format
void usb_midi_play(llist_t* llist_ptr) {
  for (blob_t* blob_ptr = (blob_t*)ITERATOR_START_FROM_HEAD(llist_ptr); blob_ptr != NULL; blob_ptr = (blob_t*)ITERATOR_NEXT(blob_ptr)) {
    usbMIDI.sendControlChange(BS, blob_ptr->state, blob_ptr->UID + 1);
    usbMIDI.sendControlChange(BX, (uint8_t)round(map(blob_ptr->centroid.X, 0.0, 59.0, 0, 127)), blob_ptr->UID + 1);
    usbMIDI.sendControlChange(BY, (uint8_t)round(map(blob_ptr->centroid.Y, 0.0, 59.0, 0, 127)), blob_ptr->UID + 1);
    usbMIDI.sendControlChange(BW, blob_ptr->box.W, blob_ptr->UID + 1);
    usbMIDI.sendControlChange(BH, blob_ptr->box.H, blob_ptr->UID + 1);
    usbMIDI.sendControlChange(BD, constrain(blob_ptr->box.D, 0, 127), blob_ptr->UID + 1);
  }
  while (usbMIDI.read()); // Read and discard any incoming MIDI messages
}

// ccPesets_ptr -> ARGS[blobID, [BX,BY,BW,BH,BD], cChange, midiChannel, Val]
void controlChange(llist_t* llist_ptr, ccPesets_t* ccPesets_ptr) {
  for (blob_t* blob_ptr = (blob_t*)ITERATOR_START_FROM_HEAD(llist_ptr); blob_ptr != NULL; blob_ptr = (blob_t*)ITERATOR_NEXT(blob_ptr)) {
    // Test if we are within the blob limit
    if (blob_ptr->UID == ccPesets_ptr->blobID) {
      // Test if the blob is alive
      if (blob_ptr->state) {
        switch (ccPesets_ptr->mappVal) {
          case BX:
            if (blob_ptr->centroid.X != ccPesets_ptr->val) {
              ccPesets_ptr->val = blob_ptr->centroid.X;
              usbMIDI.sendControlChange(ccPesets_ptr->cChange, constrain(blob_ptr->centroid.X, 0, 127), ccPesets_ptr->midiChannel);
            }
            break;
          case BY:
            if (blob_ptr->centroid.Y != ccPesets_ptr->val) {
              ccPesets_ptr->val = blob_ptr->centroid.Y;
              usbMIDI.sendControlChange(ccPesets_ptr->cChange, constrain(blob_ptr->centroid.Y, 0, 127), ccPesets_ptr->midiChannel);
            }
            break;
          case BW:
            if (blob_ptr->box.W != ccPesets_ptr->val) {
              ccPesets_ptr->val = blob_ptr->box.W;
              usbMIDI.sendControlChange(ccPesets_ptr->cChange, constrain(blob_ptr->box.W, 0, 127), ccPesets_ptr->midiChannel);
            }
            break;
          case BH:
            if (blob_ptr->box.H != ccPesets_ptr->val) {
              ccPesets_ptr->val = blob_ptr->box.H;
              usbMIDI.sendControlChange(ccPesets_ptr->cChange, constrain(blob_ptr->box.H, 0, 127), ccPesets_ptr->midiChannel);
            }
            break;
          case BD:
            if (blob_ptr->box.D != ccPesets_ptr->val) {
              ccPesets_ptr->val = blob_ptr->box.D;
              usbMIDI.sendControlChange(ccPesets_ptr->cChange, constrain(blob_ptr->box.D, 0, 127), ccPesets_ptr->midiChannel);
            }
            break;
        }
#if DEBUG_MAPPING
        switch (ccPesets_ptr->mappVal) {
          case BX:
            if (blob_ptr->centroid.X != ccPesets_ptr->val) {
              ccPesets_ptr->val = blob_ptr->centroid.X;
              Serial.printf("\nMIDI\tCC:%d\tVAL:%d\tCHAN:%d", blob_ptr->UID, constrain(blob_ptr->centroid.X, 0, 127), ccPesets_ptr->midiChannel);
            }
            break;
          case BY:
            if (blob_ptr->centroid.Y != ccPesets_ptr->val) {
              ccPesets_ptr->val = blob_ptr->centroid.Y;
              Serial.printf("\nMIDI\tCC:%d\tVAL:%d\tCHAN:%d", blob_ptr->UID, constrain(blob_ptr->centroid.Y, 0, 127), ccPesets_ptr->midiChannel);
            }
            break;
          case BW:
            if (blob_ptr->box.W != ccPesets_ptr->val) {
              ccPesets_ptr->val = blob_ptr->box.W;
              Serial.printf("\nMIDI\tCC:%d\tVAL:%d\tCHAN:%d", blob_ptr->UID, constrain(blob_ptr->box.W, 0, 127), ccPesets_ptr->midiChannel);
            }
            break;
          case BH:
            if (blob_ptr->box.H != ccPesets_ptr->val) {
              ccPesets_ptr->val = blob_ptr->box.H;
              Serial.printf("\nMIDI\tCC:%d\tVAL:%d\tCHAN:%d", blob_ptr->UID, constrain(blob_ptr->box.H, 0, 127), ccPesets_ptr->midiChannel);
            }
            break;
          case BD:
            if (blob_ptr->box.D != ccPesets_ptr->val) {
              ccPesets_ptr->val = blob_ptr->box.D;
              Serial.printf("\nBLOB:%d\tCC:%d\tVAL:%d\tCHAN:%d", blob_ptr->UID, ccPesets_ptr->cChange, constrain(blob_ptr->box.D, 0, 127), ccPesets_ptr->midiChannel);
            }
            break;
        }
#endif
      }
    }
  }
}
#endif
