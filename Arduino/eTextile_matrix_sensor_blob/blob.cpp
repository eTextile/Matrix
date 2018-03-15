/*
   This file is part of the OpenMV project - https://github.com/openmv/openmv
   Copyright (c) 2013-2017 Ibrahim Abdelkader <iabdalkader@openmv.io> & Kwabena W. Agyeman <kwagyeman@openmv.io>
   This work is licensed under the MIT license, see the file LICENSE for details.

   This file has been modified to fit the eTextile matrix sensor needs
   eTextile matrix sensor - http://matrix.eTextile.org
*/

#include "config.h"

void find_blobs(
  image_t*            inFrame_ptr,
  char*               bitmap_ptr,
  const int           rows,
  const int           cols,
  const int           pixelThreshold,
  const unsigned int  minBlobPix,
  const unsigned int  maxBlobPix,
  llist_t*            freeBlobs_ptr,
  llist_t*            blob_ptr,
  llist_t*            blobsToUpdate_ptr,
  llist_t*            blobsToAdd_ptr,
  llist_t*            outputBlobs_ptr
) {
  ////////// Connected-component labeling / Scanline flood fill algorithm //////////
  if (DEBUG_CCL) Serial.printf(F("\n DEBUG_CCL / START <<<<<<<<<<<<<<<<<<<<<<<<"));

  bitmap_clear(bitmap_ptr, NEW_FRAME);
  if (DEBUG_CCL) Serial.printf(F("\n DEBUG_CCL / Bitmap cleared"));

  llist_raz(blob_ptr);

  for (uint8_t posY = 0; posY < rows; posY++) {
    uint8_t* row_ptr_A = ROW_PTR(inFrame_ptr, posY);     // Return inFrame_ptr curent row pointer
    uint16_t row_index_A = ROW_INDEX(inFrame_ptr, posY); // Return inFrame_ptr curent row index (1D array) 0, 64, 128,... 4032
    for (uint8_t posX = 0; posX < cols; posX++) {
      // if (DEBUG_BITMAP) Serial.printf(F("%d "), GET_PIXEL(row_ptr_A, posX)); // Print INPUT_VALUES
      // if (DEBUG_BITMAP) Serial.printf(F("%d "), PIXEL_THRESHOLD(GET_PIXEL(row_ptr_A, posX), pixelThreshold));
      // if (DEBUG_BITMAP) Serial.printf(F("%d "), BITMAP_INDEX(row_index_A, posX));
      if (DEBUG_BITMAP) Serial.printf(F("%d "), bitmap_bit_get(bitmap_ptr, BITMAP_INDEX(row_index_A, posX)));

      if (!bitmap_bit_get(bitmap_ptr, BITMAP_INDEX(row_index_A, posX)) && PIXEL_THRESHOLD(GET_PIXEL(row_ptr_A, posX), pixelThreshold)) {
        if (DEBUG_CCL) Serial.printf(F("\n DEBUG_CCL / Found active pixel in row:%d\tcolumn:%d"), posY, posX);

        uint8_t oldX = posX;
        uint8_t oldY = posY;

        blob_t* blob = llist_pop_front(freeBlobs_ptr);
        if (DEBUG_CCL) Serial.printf(F("\n DEBUG_CCL / Get a blob from **freeBlobs** linked list: %p"), blob);
        raz_blob(blob);

        uint8_t blob_x1 = posX;
        uint8_t blob_y1 = posY;
        uint8_t blob_x2 = posX;
        uint8_t blob_y2 = posY;

        while (1) {
          uint8_t left = posX;
          uint8_t right = posX;

          uint8_t* row_ptr = ROW_PTR(inFrame_ptr, posY);     // Return inFrame_ptr curent row pointer
          uint16_t row_index = ROW_INDEX(inFrame_ptr, posY); // Return inFrame_ptr curent row index (1D array) 0, 64, 128,... 4032

          while ((left > 0) &&
                 (!bitmap_bit_get(bitmap_ptr, BITMAP_INDEX(row_index, left - 1))) &&
                 PIXEL_THRESHOLD(GET_PIXEL(row_ptr, left - 1), pixelThreshold)) {
            left--;
          }
          if (DEBUG_CCL) Serial.printf(F("\n DEBUG_CCL / The minimum activated left pixel ID is: %d"), left);

          while (right < (cols - 1) &&
                 (!bitmap_bit_get(bitmap_ptr, BITMAP_INDEX(row_index, right + 1))) &&
                 PIXEL_THRESHOLD(GET_PIXEL(row_ptr, right + 1), pixelThreshold)) {
            right++;
          }
          if (DEBUG_CCL) Serial.printf(F("\n DEBUG_CCL / The maximum activated right pixel ID is: %d"), right);

          blob_x1 = MIN(blob_x1, left);
          blob_y1 = MIN(blob_y1, posY);
          blob_x2 = MAX(blob_x2, right);
          blob_y2 = MAX(blob_y2, posY);

          if (DEBUG_CCL) Serial.printf(F("\n DEBUG_CCL / Save this activated pixels line to the bitmap array"));
          for (uint8_t x = left; x <= right; x++) {
            bitmap_bit_set(bitmap_ptr, BITMAP_INDEX(row_index, x));
            blob->centroid.Z = MAX(blob->centroid.Z, GET_PIXEL(row_ptr, x));
            blob->pixels++;
          }

          boolean break_out = true;
          if (posY < (rows - 1)) {
            row_ptr = ROW_PTR(inFrame_ptr, posY + 1);     // Return inFrame_ptr curent row pointer
            row_index = ROW_INDEX(inFrame_ptr, posY + 1); // Return inFrame_ptr curent row index (1D array) 0, 64, 128,... 4032
            for (uint8_t x = left; x <= right; x++) {
              if ((!bitmap_bit_get(bitmap_ptr, BITMAP_INDEX(row_index, x))) && PIXEL_THRESHOLD(GET_PIXEL(row_ptr, x), pixelThreshold)) {
                if (DEBUG_CCL) Serial.printf(F("\n DEBUG_CCL / Found an active pixel in row: %d"), posY + 1);
                posX = x;
                posY++;
                break_out = false;
                break;
              }
            }
          }
          if (break_out) break;
        }
        if (DEBUG_CCL) Serial.printf(F("\n DEBUG_CCL / BLOB COMPLEAT <<<<<<<<<<<<<<<<<<<<<<<<"));

        if ((blob->pixels > minBlobPix) && (blob->pixels < maxBlobPix)) {

          blob->centroid.X = (uint8_t) (blob_x2 - ((blob_x2 - blob_x1) / 2)); // x centroid position
          blob->centroid.Y = (uint8_t) (blob_y2 - ((blob_y1 - blob_y2) / 2)); // y centroid position
          // uint8_t* row_ptr = ROW_PTR(inFrame_ptr, blob->centroid.Y); // DO NOT WORK!?
          // blob->centroid.Z = GET_PIXEL(row_ptr, blob->centroid.X);   // DO NOT WORK!?
          if (DEBUG_CENTER) Serial.printf(F("\n DEBUG_CENTER / blob_cx: %d\tblob_cy: %d\tblob_cz: %d"), blob->centroid.X, blob->centroid.Y, blob->centroid.Z);

          llist_push_back(blob_ptr, blob);
          if (DEBUG_CCL) Serial.printf(F("\n DEBUG_CCL / Blob: %p added to the **blobs** linked list"), blob);
        } else {
          llist_push_back(freeBlobs_ptr, blob);
          if (DEBUG_CCL) Serial.printf(F("\n DEBUG_CCL / Blob %p saved to **freeBlobList** linked list"), blob);
        }
        posX = oldX;
        posY = oldY;
      }
    }
    if (DEBUG_BITMAP) Serial.println();
  }
  if (DEBUG_BITMAP) Serial.println();

  if (DEBUG_CCL) Serial.printf(F("\n DEBUG_CCL / **blobs** linked list index: %d"), blob_ptr->index);
  if (DEBUG_CCL) Serial.printf(F("\n DEBUG_CCL / **freeBlobs** linked list index: %d"), freeBlobs_ptr->index);

  if (DEBUG_CCL) Serial.print(F("\n DEBUG_CCL / END of scanline flood fill algorithm <<<<<<<<<<<<<<<<<<<<<<<<"));

  ///////////////////////////////////////////////////////////////////////////////////////// Percistant blobs ID
  if (PERSISTANT_ID) {

    OSCBundle bndl;
    OSCMessage msg("/sensors");

    if (DEBUG_BLOB) Serial.printf(F("\n DEBUG_BLOB / **blobs** linked list index: %d"), blob_ptr->index);
    if (DEBUG_BLOB) Serial.printf(F("\n DEBUG_BLOB / **freeBlobs** linked list index: %d"), freeBlobs_ptr->index);
    if (DEBUG_BLOB) Serial.printf(F("\n DEBUG_BLOB / **outputBlobs** linked list index: %d"), outputBlobs_ptr->index);

    // Look for the nearest blob between curent blob position (blobs linked list) and last blob position (outputBlobs linked list)
    for (blob_t* blobA = iterator_start_from_head(blob_ptr); blobA != NULL; blobA = iterator_next(blobA)) {
      float minDist = 127.0f;
      blob_t* nearestBlob = NULL;
      if (DEBUG_BLOB) Serial.printf(F("\n DEBUG_BLOB / Is input blob: %p have a coresponding blob in **outputBlobs**"), blobA);

      for (blob_t* blobB = iterator_start_from_head(outputBlobs_ptr); blobB != NULL; blobB = iterator_next(blobB)) {
        uint8_t xa = blobA->centroid.X;
        uint8_t ya = blobA->centroid.Y;
        uint8_t xb = blobB->centroid.X;
        uint8_t yb = blobB->centroid.Y;
        float dist = sqrt(pow(xa - xb, 2) + pow(ya - yb, 2)); // fast_sqrt? & fast_pow? // arm_sqrt_f32(); ?
        if (DEBUG_BLOB) Serial.printf(F("\n DEBUG_BLOB / Distance between input & output blobs positions: %f "), dist);

        if (dist < minDist) {
          minDist = dist;
          nearestBlob = blobB;
        }
      }
      // If the distance between curent blob and last blob position is less than minDist:
      // Take the ID of the nearestBlob in outputBlobs linked list and give it to the curent input blob.
      // Move the curent blob to the blobsToUpdate linked list.
      if (minDist < 5.0f) {
        if (DEBUG_BLOB) Serial.printf(F("\n DEBUG_BLOB / Found corresponding blob: %p in the **outputBlobs** linked list"), nearestBlob);
        blobA->UID = nearestBlob->UID;
        if (DEBUG_BLOB) Serial.printf(F("\n DEBUG_BLOB / Copy the corresponding **outputBlobs** ID: %d to the incoming blob ID"), nearestBlob->UID);
        llist_push_back(blobsToUpdate_ptr, blobA);
        if (DEBUG_BLOB) Serial.printf(F("\n DEBUG_BLOB / Blob: %p pushed back to the **blobsToUpdate** linked list"), blobA);
        break;
      } else {
        // Found a new blob! we nead to give it an ID.
        // We look for the minimum unused ID through the outputBlobs linked list &
        // We add the new blob to the nodesToAdd linked list.
        if (DEBUG_BLOB) Serial.printf(F("\n DEBUG_BLOB / Found new blob without ID"));
        int8_t minID = 0;
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
            if (DEBUG_BLOB) Serial.printf(F("\n DEBUG_BLOB / Blob ID seted to: %d"), minID);
            llist_push_back(blobsToAdd_ptr, blobA);
            if (DEBUG_BLOB) Serial.printf(F("\n DEBUG_BLOB / New incoming blob: %p pushed back to the **blobsToAdd** linked list"), blobA);
            break;
          }
        }
        if (DEBUG_BLOB) Serial.printf(F("\n DEBUG_BLOB / BREAK_1"));
      }
      if (DEBUG_BLOB) Serial.printf(F("\n DEBUG_BLOB / BREAK_2"));
    }
    if (DEBUG_BLOB) Serial.printf(F("\n DEBUG_BLOB / EXIT <<<<<<<<<<<<<<<<<"));

    // Update outputBlobs linked list with blobsToUpdate linked list.
    // If a blob in the outputBlobs linked list is not in the blobsToUpdate linked list, set it to dead.
    for (blob_t* blobA = iterator_start_from_head(outputBlobs_ptr); blobA != NULL; blobA = iterator_next(blobA)) {
      boolean found = false;
      for (blob_t* blobB = iterator_start_from_head(blobsToUpdate_ptr); blobB != NULL; blobB = iterator_next(blobB)) {
        if (blobB->UID == blobA->UID) {
          found = true;
          llist_update_blob(blobA, blobB);
          if (DEBUG_BLOB) Serial.printf(F("\n DEBUG_BLOB / BlobA: %p updated with blobB: %p"), blobA, blobB);

          // llist_push_back(freeBlobs_ptr, blobB); // NOT GOOD - IT BRAKE THE LINKED LIST!
          // if (DEBUG_BLOB) Serial.printf(F("\n DEBUG_BLOB / BlobB: %p saved to **freeBlobList** linked list"), blobB);

          // Add the blob values to the OSC bundle

          msg.add(blobA->UID);
          msg.add(blobA->centroid.X);
          msg.add(blobA->centroid.Y);
          msg.add(blobA->centroid.Z);
          msg.add(blobA->pixels);
          bndl.add(msg);

          if (DEBUG_OSC) {
            Serial.printf(F("\n DEBUG_OSC / UID:%d\tX:%d\tY:%d\tZ:%d\tPIX:%d"),
                          blobA->UID,
                          blobA->centroid.X,
                          blobA->centroid.Y,
                          blobA->centroid.Z,
                          blobA->pixels
                         );
          }
          break;
        }
      }
      if (!found) {
        blobA->isDead = true;
        if (DEBUG_BLOB) Serial.printf(F("\n DEBUG_BLOB / Found dead blob: %p in the **outputBlobs** linked list"), blobA);
      }
    }

    // Suppress dead blobs from the outputBlobs linked list
    while (1) {
      boolean found = false;
      for (blob_t* blob = iterator_start_from_head(outputBlobs_ptr); blob != NULL; blob = iterator_next(blob)) {
        if (blob->isDead) {
          found = true;
          llist_remove_blob(outputBlobs_ptr, blob);
          if (DEBUG_BLOB) Serial.printf(F("\n DEBUG_BLOB / Blob: %p removed from **outputBlobs** linked list"), blob);
          llist_push_back(freeBlobs_ptr, blob);
          if (DEBUG_BLOB) Serial.printf(F("\n DEBUG_BLOB / Blob: %p saved to **freeBlobList** linked list"), blob);
          // Add the blob values to the OSC bundle
          msg.add(blob->UID);
          msg.add(-1);
          msg.add(-1);
          msg.add(-1);
          msg.add(-1);
          bndl.add(msg);
          if (DEBUG_OSC) {
            Serial.printf(F("\n DEBUG_OSC / UID:%d\tX:%d\tY:%d\tZ:%d\tPIX:%d"), blob->UID, -1, -1, -1, -1);
          }
          break;
        }
      }
      if (!found) {
        break;
      }
    }

    // Add the new blobs to the outputBlobs linked list
    for (blob_t* blob = iterator_start_from_head(blobsToAdd_ptr); blob != NULL; blob = iterator_next(blob)) {
      llist_push_back(outputBlobs_ptr, blob);
      if (DEBUG_BLOB) Serial.printf(F("\n DEBUG_BLOB / Blob: %p added to **outputBlobs** linked list"), blob);
      // Add the blob values to the OSC bundle
      msg.add(blob->UID);
      msg.add(blob->centroid.X);
      msg.add(blob->centroid.Y);
      msg.add(blob->centroid.Z);
      msg.add(blob->pixels);
      bndl.add(msg);
      if (DEBUG_OSC) {
        Serial.printf(F("\n DEBUG_OSC / UID:%d\tX:%d\tY:%d\tZ:%d\tPIX:%d"),
                      blob->UID,
                      blob->centroid.X,
                      blob->centroid.Y,
                      blob->centroid.Z,
                      blob->pixels
                     );
      }
    }
    // Send the blobs values with the OSC bundle
    // SLIPSerial.beginPacket();
    // bndl.send(SLIPSerial);  // Send the bytes to the SLIP stream
    // SLIPSerial.endPacket();    // Mark the end of the OSC packet
    bndl.empty();              // Empty the bundle to free room for a new one

    if (DEBUG_BLOB) Serial.printf(F("\n DEBUG_BLOB / %d Blobs from **blobsToUpdate_ptr** saved to **freeBlobList** linked list"), blobsToUpdate_ptr->index);
    llist_save_blobs(freeBlobs_ptr, blobsToUpdate_ptr);

    llist_raz(blobsToUpdate_ptr);
    llist_raz(blobsToAdd_ptr);

  } else {
    llist_save_blobs(freeBlobs_ptr, blob_ptr);
  }
  if (DEBUG_BLOB) Serial.printf(F("\n DEBUG_BLOB / END OFF BLOB FONCTION"));
}



////////////////////////////// Bitmap //////////////////////////////

char bitmap_bit_get(char* bitmap_ptr, uint16_t index) {
  return (bitmap_ptr[index >> CHAR_SHIFT] >> (index & CHAR_MASK)) & 1;
}

void bitmap_bit_set(char* bitmap_ptr, uint16_t index) {
  bitmap_ptr[index >> CHAR_SHIFT] |= 1 << (index & CHAR_MASK);
}

void bitmap_clear(char* bitmap_ptr, const uint16_t arraySize) {
  memset(bitmap_ptr, 0, arraySize * sizeof(char));
}

void raz_blob(blob_t* node) {

  node->UID = -1; // RAZ UID
  node->centroid.X = 0;
  node->centroid.Y = 0;
  node->centroid.Z = 0;
  node->pixels = 0;
  node->isDead = false;
}

////////////////////////////// linked list  //////////////////////////////

void llist_raz(llist_t* ptr) {
  ptr->tail_ptr = ptr->head_ptr = NULL;
  ptr->index = -1;
}

void llist_init(llist_t* dst, blob_t* nodesArray, const uint8_t max_nodes) {

  dst->max_nodes = max_nodes;

  dst->head_ptr = dst->tail_ptr = &nodesArray[0];
  if (DEBUG_LIST || DEBUG_CCL) Serial.printf(F("\n DEBUG_LIST / llist_init: %d: %p"), 0, &nodesArray[0]);
  dst->index++;

  for (int i = 1; i < dst->max_nodes; i++) {
    nodesArray[i - 1].next_ptr = &nodesArray[i];
    nodesArray[i].next_ptr = NULL;
    dst->tail_ptr = &nodesArray[i];
    if (DEBUG_LIST || DEBUG_CCL) Serial.printf(F("\n DEBUG_LIST / llist_init: %d: %p"), i, &nodesArray[i]);
    dst->index++;
  }
}

blob_t* llist_pop_front(llist_t* src) {

  if (src->index > -1) {
    blob_t* node = src->head_ptr;
    if (src->index > 0) {
      src->head_ptr = src->head_ptr->next_ptr;
    } else {
      src->head_ptr = src->tail_ptr = NULL;
    }
    node->next_ptr = NULL;
    src->index--;
    return node;
  } else {
    Serial.printf(F("\n DEBUG_LIST / list_pop_front / ERROR : SRC list is umpty!"));
    return NULL;
  }
}

void llist_push_back(llist_t* dst, blob_t* node) {

  if (dst->index > -1) {
    dst->tail_ptr->next_ptr = node;
    dst->tail_ptr = node;
  } else {
    dst->head_ptr = dst->tail_ptr = node;
  }
  node->next_ptr = NULL;
  dst->index++;
}

// Remove a blob in a linked list
void llist_remove_blob(llist_t* src, blob_t* blobSuppr) {

  blob_t* prevBlob = NULL;
  if (DEBUG_LIST) Serial.printf(F("\n DEBUG_LIST / list_remove_blob / Blob to remove: %p"), blobSuppr);

  for (blob_t* blob = iterator_start_from_head(src); blob != NULL; blob = iterator_next(blob)) {

    if (blob == blobSuppr) {
      if (DEBUG_LIST) Serial.printf(F("\n DEBUG_LIST / list_remove_blob / Blob: %p is found"), blob);

      if (src->index == 0) {
        if (DEBUG_LIST) Serial.printf(F("\n DEBUG_LIST / list_remove_blob / The blob: %p is the first & last in the linked list"), blobSuppr);
        src->head_ptr = src->tail_ptr = NULL;
        src->index--;
        return;
      }
      else if (blob->next_ptr == NULL) {
        if (DEBUG_LIST) Serial.printf(F("\n DEBUG_LIST / list_remove_blob / The blob: %p is the tail of the linked list"), blobSuppr);
        prevBlob->next_ptr = NULL;
        src->tail_ptr = prevBlob;
        src->index--;
        return;
      }
      else if (blob == src->head_ptr) {
        if (DEBUG_LIST) Serial.printf(F("\n DEBUG_LIST / list_remove_blob / The blob: %p is the hard of the linked list"), blobSuppr);
        src->head_ptr = src->head_ptr->next_ptr;
        src->index--;
        return;
      }
      else {
        if (DEBUG_LIST) Serial.printf(F("\n DEBUG_LIST / list_remove_blob / The blob: %p is somewear else in the linked list"), blobSuppr);
        prevBlob->next_ptr = blob->next_ptr;
        src->index--;
        return;
      }
    }
    prevBlob = blob;
  }
  if (DEBUG_LIST) Serial.printf(F("\n DEBUG_LIST / list_remove_blob / ERROR / Blob not found"));
}

void llist_save_blobs(llist_t* dst, llist_t* src) {

  if (DEBUG_LIST) Serial.printf(F("\n DEBUG_LIST / list_save_blobs / START"));
  blob_t* blob = NULL;

  while (src->index > -1) {
    // SRC pop front
    blob = src->head_ptr;
    if (src->index > 0) {
      if (DEBUG_LIST) Serial.printf(F("\n DEBUG_LIST / list_save_blobs / SRC pop a blob in the list: %p"), blob);
      src->head_ptr = src->head_ptr->next_ptr;
      if (DEBUG_LIST) Serial.printf(F("\n DEBUG_LIST / list_save_blobs / SRC Move the list hed to next_ptr: %p"), src->head_ptr);
    } else { // src->index == 0
      src->tail_ptr = src->head_ptr = NULL;
      if (DEBUG_LIST) Serial.printf(F("\n DEBUG_LIST / list_save_blobs / SRC pop the last blob in the list: %p"), blob);
    }
    src->index--;
    if (DEBUG_LIST) Serial.printf(F("\n DEBUG_LIST / list_save_blobs / SRC set index to: %d"), src->index);

    // DST push back
    if (dst->index > -1) {
      dst->tail_ptr->next_ptr = blob;
      if (DEBUG_LIST) Serial.printf(F("\n DEBUG_LIST / list_save_blobs / DST add the blob to the list: %p"), blob);
      dst->tail_ptr = blob;
    } else { // dst->index == -1
      dst->head_ptr = dst->tail_ptr = blob;
      if (DEBUG_LIST) Serial.printf(F("\n DEBUG_LIST / list_save_blobs / DST add the first blob to the list"));
    }
    dst->tail_ptr->next_ptr = NULL; // Same than blob->next_ptr = NULL;
    dst->index++;
    if (DEBUG_LIST) Serial.printf(F("\n DEBUG_LIST / list_save_blobs / DST set index to: %d"), dst->index);
  }
  if (DEBUG_LIST) Serial.printf(F("\n DEBUG_LIST / list_save_blobs / SRC linked list is umpty!"));
}

void llist_update_blob(blob_t* dst, blob_t* src) {

  //memcpy(dst, src, blobSize); // Can't do that because a blob have an next_ptr element that must not be changed!

  // dst->UID = src->UID;
  dst->centroid.X = src->centroid.X;
  dst->centroid.Y = src->centroid.Y;
  dst->centroid.Z = src->centroid.Z;
  dst->pixels = src->pixels;
  //dst->isDead = src->isDead;
}

////////////////////////////// Linked list iterators //////////////////////////////

blob_t* iterator_start_from_head(llist_t* src) {
  return src->head_ptr;
}

blob_t* iterator_next(blob_t* src) {
  return src->next_ptr;
}
int8_t llist_size(llist_t* ptr) {
  return ptr->index;
}
