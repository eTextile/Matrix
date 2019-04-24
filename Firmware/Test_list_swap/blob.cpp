/*
  FORKED FROM https://github.com/openmv/openmv/tree/master/src/omv/img
  Added custom blob détection algorithm to keep track of the blobs ID's
    This patch is part of the eTextile-matrix-sensor project - http://matrix.eTextile.org
    Copyright (c) 2014-2018 Maurin Donneaud <maurin@etextile.org>
    This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "blob.h"

#define BLOB_PACKET_SIZE      6     // OSC blob data packet

blob_t    blobArray[MAX_NODES] = {0};       // 40 nodes
uint8_t   blobPacket[BLOB_PACKET_SIZE] = {0};

llist_t   freeBlobs;
llist_t   blobs;
llist_t   outputBlobs;

void find_blobs(
  llist_t*              freeBlobs_ptr,
  llist_t*              inputBlobs_ptr,
  llist_t*              outputBlobs_ptr
) {

  llist_raz(inputBlobs_ptr);

  //Serial.printf("\n DEBUG_BLOBS / **blobs** linked list index: %d", inputBlobs_ptr->index);
  //Serial.printf("\n DEBUG_BLOBS / **freeBlobs** linked list index: %d", freeBlobs_ptr->index);
  //Serial.printf("\n DEBUG_BLOBS / **outputBlobs** linked list index: %d", outputBlobs_ptr->index);

  // Look for the nearest blob between curent blob position (blobs linked list) and last blob position (outputBlobs linked list)
  for (blob_t* blobA = iterator_start_from_head(inputBlobs_ptr); blobA != NULL; blobA = iterator_next(blobA)) {

    float minDist = 255.0f;

    blob_t* nearestBlob = NULL;
    //Serial.printf("\n DEBUG_BLOBS / Is input blob: %p have a coresponding blob in **outputBlobs**", blobA);

    for (blob_t* blobB = iterator_start_from_head(outputBlobs_ptr); blobB != NULL; blobB = iterator_next(blobB)) {
      uint8_t xa = blobA->centroid.X;
      uint8_t ya = blobA->centroid.Y;
      uint8_t xb = blobB->centroid.X;
      uint8_t yb = blobB->centroid.Y;
      float dist = sqrt(pow(xa - xb, 2) + pow(ya - yb, 2)); // fast_sqrt? & fast_pow? // arm_sqrt_f32(); ?
      //Serial.printf("\n DEBUG_BLOBS / Distance between input & output blobs positions: %f ", dist);

      if (dist < minDist) {
        minDist = dist;
        nearestBlob = blobB;
      }
    }

    // If the distance between curent blob and last blob position is less than minDist:
    // Copy the ID of the nearestBlob found in outputBlobs linked list and give it to the curent input blob.
    // Set the curent input blob state to TO_UPDATE.
    if (minDist < 10.0f) { // TODO: set it as global argument
      //Serial.printf("\n DEBUG_BLOBS / Found corresponding blob: %p in the **outputBlobs** linked list", nearestBlob);
      blobA->UID = nearestBlob->UID;
      blobA->state = TO_UPDATE;
    }

    // Found a new blob! we nead to give it an ID.
    else {
      //Serial.print("\n DEBUG_BLOBS / Found new blob without ID");

      // Bubble sort the outputBlobs linked list // FIXME!
      llist_bubbleSort(outputBlobs_ptr);

      // Find the smallest positive missing UID in the sorted linked list
      uint8_t minID = 1;
      while (1) {
        boolean isFree = true;
        for (blob_t* blob = iterator_start_from_head(outputBlobs_ptr); blob != NULL; blob = iterator_next(blob)) {
          if (blob->UID == minID) {
            isFree = false;
            minID++;
            break;
          }
        }
        if (isFree) {
          blobA->UID = minID;
          blobA->state = TO_ADD;
          break;
        }
      } // while_end / The blob have a new ID
    }
  }
  //Serial.printf(F("\n DEBUG_BLOBS / EXIT <<<<<<<<<<<<<<<<<"));

  // Update outputBlobs linked list with new inputs blobs that have ben seted TO_UPDATE.
  // If a blob in the outputBlobs linked do not have corresponding blob in the input blobs linked list (inputBlobs_ptr), set it state to DEAD.
  // TODO? : Adding time-tag to alow the reciver to suppress blob in the stream after a certain time / or use blob->lastState to do it in the firmware!
  for (blob_t* blobA = iterator_start_from_head(outputBlobs_ptr); blobA != NULL; blobA = iterator_next(blobA)) {
    boolean found = false;
    for (blob_t* blobB = iterator_start_from_head(inputBlobs_ptr); blobB != NULL; blobB = iterator_next(blobB)) {
      if (blobB->UID == blobA->UID && blobB->state == TO_UPDATE) {
        found = true;
        blob_copy(blobA, blobB);
        blobB->state = FREE;
        break;
      }
    }
    if (!found) {
      blobA->state = DEAD;
    }
  }

  // Suppress dead blobs from the outputBlobs linked list
  while (1) {
    boolean found = false;
    for (blob_t* blob = iterator_start_from_head(outputBlobs_ptr); blob != NULL; blob = iterator_next(blob)) {
      if (blob->state == DEAD) {
        found = true;
        llist_remove_blob(outputBlobs_ptr, blob);
        //Serial.printf("\n DEBUG_BLOBS / Blob: %p removed from **outputBlobs** linked list", blob);
        llist_push_back(freeBlobs_ptr, blob);
        //Serial.printf("\n DEBUG_BLOBS / Blob: %p saved to **freeBlobList** linked list", blob);
        break;
      }
    }
    if (!found) {
      break;
    }
  }

  // Add the new blobs to the outputBlobs linked list
  for (blob_t* blob = iterator_start_from_head(inputBlobs_ptr); blob != NULL; blob = iterator_next(blob)) {
    if (blob->state == TO_ADD) {
      blob_t* newBlob = llist_pop_front(freeBlobs_ptr);
      blob_copy(newBlob, blob);
      llist_push_back(outputBlobs_ptr, newBlob);
      //Serial.printf("\n DEBUG_BLOBS / Blob: %p added to **outputBlobs** linked list", blob);
    }
  }

  llist_save_blobs(freeBlobs_ptr, inputBlobs_ptr);
  //Serial.println("\n DEBUG_BLOBS / END OFF BLOB FONCTION");
}

////////////////////////////// Bitmap //////////////////////////////

char bitmap_bit_get(char* bitmap_ptr, uint16_t index) {
  return (bitmap_ptr[index >> CHAR_SHIFT] >> (index & CHAR_MASK)) & 1;
}

void bitmap_bit_set(char* bitmap_ptr, uint16_t index) {
  bitmap_ptr[index >> CHAR_SHIFT] |= 1 << (index & CHAR_MASK);
}

void bitmap_clear(char* bitmap_ptr, const uint16_t Size) {
  memset(bitmap_ptr, 0, Size * sizeof(char));
}

inline void blob_copy(blob_t* dst, blob_t* src) {
  //dst->timeTag = millis(); // TODO?
  dst->UID = src->UID;
  dst->centroid.X = src->centroid.X;
  dst->centroid.Y = src->centroid.Y;
  dst->box.W = src->box.W;
  dst->box.H = src->box.H;
  dst->box.D = src->box.D;
  //dst->pixels = src->pixels;
}

inline void blob_raz(blob_t* node) {
  //node->timeTag = 0; // TODO?
  node->UID = 1;
  node->state = FREE;
  node->centroid.X = 0;
  node->centroid.Y = 0;
  node->box.W = 0;
  node->box.H = 0;
  node->box.D = 0;
  //node->pixels = 0;
}
