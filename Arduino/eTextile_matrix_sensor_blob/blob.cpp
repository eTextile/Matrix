/*
   This file is part of the OpenMV project - https://github.com/openmv/openmv
   Copyright (c) 2013-2017 Ibrahim Abdelkader <iabdalkader@openmv.io> & Kwabena W. Agyeman <kwagyeman@openmv.io>
   This work is licensed under the MIT license, see the file LICENSE for details.

   This file has been modified to fit the eTextile matrix sensor needs
   eTextile matrix sensor - http://matrix.eTextile.org
*/

#include <Arduino.h>
#include "collections.h"
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
  
  llist_raz(blob_ptr);

  for (uint8_t posY = 0; posY < rows; posY++) {
    uint8_t* row_ptr_A = ROW_PTR(inFrame_ptr, posY);     // Return inFrame_ptr curent row pointer
    uint16_t row_index_A = ROW_INDEX(inFrame_ptr, posY); // Return inFrame_ptr curent row index (1D array) 0, 64, 128,... 4032
    for (uint8_t posX = 0; posX < cols; posX++) {
      // if (DEBUG_BITMAP) Serial.printf(F("%d "), GET_PIXEL(row_ptr, posX)); // Print INPUT_VALUES
      // if (DEBUG_BITMAP) Serial.printf(F("%d "), PIXEL_THRESHOLD(GET_PIXEL(row_ptr_A, posX), pixelThreshold));
      // if (DEBUG_BITMAP) Serial.printf(F("%d "), BITMAP_INDEX(row_index, posX));
      // if (DEBUG_BITMAP) Serial.printf(F("%d "), !bitmap_bit_get(bitmap_ptr, BITMAP_INDEX(row_index_A, posX)));

      if (!bitmap_bit_get(bitmap_ptr, BITMAP_INDEX(row_index_A, posX)) && PIXEL_THRESHOLD(GET_PIXEL(row_ptr_A, posX), pixelThreshold)) {
        if (DEBUG_CCL) Serial.printf(F("\n DEBUG_CCL / Found active pixel in row:%d\tcolumn:%d"), posY, posX);

        uint8_t oldX = posX;
        uint8_t oldY = posY;

        blob_t* blob = llist_pop_front(freeBlobs_ptr);
        if (DEBUG_CCL) Serial.printf(F("\n DEBUG_CCL / Get a blob from **freeBlobs** linked list: %p"), blob);

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

        if ((blob->pixels > minBlobPix) && (blob->pixels < maxBlobPix) && (llist_size(freeBlobs_ptr) > -1)) {

          blob->centroid.X = blob_x2 - ((blob_x2 - blob_x1) / 2); // x centroid position
          blob->centroid.Y = blob_y2 - ((blob_y1 - blob_y2) / 2); // y centroid position
          // uint8_t* row_ptr = ROW_PTR(inFrame_ptr, blob->centroid.Y); // DO NOT WORK!?
          // blob->centroid.Z = GET_PIXEL(row_ptr, blob->centroid.X);   // DO NOT WORK!?
          if (DEBUG_CENTER) Serial.printf(F("\n DEBUG_CENTER / blob_cx: %d\tblob_cy: %d\tblob_cz: %d"), blob->centroid.X, blob->centroid.Y, blob->centroid.Z);

          llist_push_back(blob_ptr, blob);
          if (DEBUG_CCL) Serial.printf(F("\n DEBUG_CCL / Blob added to the **blobs** linked list: %p"), blob);
        } else {
          llist_push_back(freeBlobs_ptr, blob);
          if (DEBUG_CCL) Serial.printf(F("\n DEBUG_CCL / saved blob Not valide  **freeBlobList** linked list"));
        }
        posX = oldX;
        posY = oldY;
      }
    }
    if (DEBUG_BITMAP) Serial.println();
  }
  if (DEBUG_BITMAP) Serial.println();

  bitmap_clear(bitmap_ptr);
  if (DEBUG_BLOB) Serial.printf(F("\n DEBUG_CCL / Cleared bitmap"));
  if (DEBUG_CCL) Serial.printf(F("\n DEBUG_CCL / Input **blobs** linked list index: %d"), blob_ptr->index);
  if (DEBUG_CCL) Serial.printf(F("\n DEBUG_CCL / **freeBlobs** linked list index: %d"), freeBlobs_ptr->index);
  if (DEBUG_CCL) Serial.print(F("\n DEBUG_CCL / END of scanline flood fill algorithm <<<<<<<<<<<<<<<<<<<<<<<<"));

  ///////////////////////////////////////////////////////////////////////////////////////// Percistant blobs ID
  if (PERSISTANT_ID) {

    // Look for the nearest blob between curent blob position (blobs linked list) and last blob position (outputBlobs linked list)
    for (blob_t* blobA = iterator_start_from_head(blob_ptr); blobA != NULL; blobA = iterator_next(blobA)) {
      float minDist = 127;
      blob_t* nearestBlob = NULL;
      for (blob_t* blobB = iterator_start_from_head(outputBlobs_ptr); blobB != NULL; blobB = iterator_next(blobB)) {
        uint8_t xa = blobA->centroid.X;
        uint8_t ya = blobA->centroid.Y;
        uint8_t xb = blobB->centroid.X;
        uint8_t yb = blobB->centroid.Y;
        float dist = sqrt(pow(xa - xb, 2) + pow(ya - yb, 2)); // fast_sqrt? & fast_pow? // arm_sqrt_f32(); ?

        if (dist < minDist) {
          minDist = dist;
          if (DEBUG_BLOB) Serial.printf(F("\n DEBUG_BLOB / Found nearest blob %p at: %f"), blobB, minDist);
          nearestBlob = blobB;
        }
      }

      // If the distance between curent blob and last blob position is less than minDist:
      // We take the ID of the nearestBlob in outputBlobs linked list and give it to the curent input blob.
      // We move the curent blob to the blobsToUpdate linked list.
      if (minDist < 5) {
        if (DEBUG_BLOB) Serial.printf(F("\n DEBUG_BLOB / Found corresponding blob: %p in the **outputBlobs** linked list"), nearestBlob);
        blobA->UID = nearestBlob->UID;
        if (DEBUG_BLOB) Serial.printf(F("\n DEBUG_BLOB / Copy the corresponding **outputBlobs** ID: %d to the incoming blob ID"), nearestBlob->UID);
        llist_push_back(blobsToUpdate_ptr, blobA);
        if (DEBUG_BLOB) Serial.printf(F("\n DEBUG_BLOB / BlobA: %p pushed back to the **blobsToUpdate** linked list"), blobA);
      } else {
        // Found a new blob! we nead to give it an ID.
        // We look for the minimum unused ID through the outputBlobs linked list &
        // We add the new blob to the nodesToAdd linked list.
        if (DEBUG_BLOB) Serial.printf(F("\n DEBUG_BLOB / Found new blob without ID"));
        boolean isFree = false;
        uint8_t minID = 0;
        while (!isFree) {
          isFree = true;
          for (blob_t* blob = iterator_start_from_head(outputBlobs_ptr); blob != NULL; blob = iterator_next(blob)) {
            if (blob->UID == minID) {
              minID++;
              isFree = false;
              break;
            }
          }
        }
        blobA->UID = minID;
        if (DEBUG_BLOB) Serial.printf(F("\n DEBUG_BLOB / ID: %d seted to the new incoming blob: %p"), minID, blobA);
        llist_push_back(blobsToAdd_ptr, blobA);
        if (DEBUG_BLOB) Serial.printf(F("\n DEBUG_BLOB / New incoming blob: %p pushed back to the **blobsToAdd** linked list"), blobA);
      }
    }

    // Update outputBlobs linked list with blobsToUpdate linked list.
    // If a blob in the outputBlobs linked list is not in the blobsToUpdate linked list, set it to dead.
    for (blob_t* blobA = iterator_start_from_head(outputBlobs_ptr); blobA != NULL; blobA = iterator_next(blobA)) {
      boolean found = false;
      for (blob_t* blobB = iterator_start_from_head(blobsToUpdate_ptr); blobB != NULL; blobB = iterator_next(blobB)) {
        // if (blobA->UID == blobB->UID) {
        if (blobA->UID == blobB->UID) { // && blobA != blobB
          llist_update_blob(blobA, blobB);
          if (DEBUG_BLOB) Serial.printf(F("\n DEBUG_BLOB / BlobA: %p updated with blobB: %p"), blobA, blobB);
          blobB->UID = -1; // RAZ blobB UID
          llist_push_back(freeBlobs_ptr, blobB);
          if (DEBUG_BLOB) Serial.printf(F("\n DEBUG_BLOB / BlobB: %p saved to **freeBlobList** linked list"), blobB);
          // Add the blobs values to an OSC bundle
          //message.addIntArg(blobA->UID);
          //message.addIntArg(blobA->centroid.X);
          //message.addIntArg(blobA->centroid.Y);
          //message.addIntArg(blobA->centroid.Z);
          //message.addIntArg(blobA->pixels);
          //bundle.addMessage(message);
          if (DEBUG_OSC) {
            Serial.printf(F("\n DEBUG_OSC / UID:%d\tX:%d\tY:%d\tZ:%d\tPIX:%d"),
                          blobA->UID,
                          blobA->centroid.X,
                          blobA->centroid.Y,
                          blobA->centroid.Z,
                          blobA->pixels
                         );
          }
          found = true;
        }
        break;
      }
      if (!found) {
        blobA->isDead = true;
        if (DEBUG_BLOB) Serial.printf(F("\n DEBUG_BLOB / Found dead blob: %p in the **outputBlobs** linked list"), blobA);
      }
    }

    // Suppress dead blobs from the outputBlobs linked list
    while (1) {
      blob_t* deadBlob = NULL;
      for (blob_t* blob = iterator_start_from_head(outputBlobs_ptr); blob != NULL; blob = iterator_next(blob)) {
        if (blob->isDead) {
          deadBlob = blob;
          if (DEBUG_BLOB) Serial.printf(F("\n DEBUG_BLOB / Blob: %p from **outputBlobs** linked list - FLAG_FLAG"), deadBlob);
          break;
        }
      }
      if (deadBlob != NULL) {
        llist_remove_blob(outputBlobs_ptr, deadBlob);
        if (DEBUG_BLOB) Serial.printf(F("\n DEBUG_BLOB / Blob: %p removed from **outputBlobs** linked list - FLAG_B"), deadBlob);
        // deadBlob->UID = -1; // RAZ deadBlob UID
        llist_push_back(freeBlobs_ptr, deadBlob);
        if (DEBUG_BLOB) Serial.printf(F("\n DEBUG_BLOB / Blob: %p saved to **freeBlobList** linked list"), deadBlob);
        // Add the blobs values to an OSC bundle
        // message.addIntArg(deadBlob.UID);
        // message.addIntArg(-1);
        // message.addIntArg(-1);
        // message.addIntArg(-1);
        // message.addIntArg(-1);
        // bundle.addMessage(message);
        if (DEBUG_OSC) {
          Serial.printf(F("\n DEBUG_OSC / UID:%d\tX:%d\tY:%d\tZ:%d\tPIX:%d"), deadBlob->UID, -1, -1, -1, -1);
        }
      } else {
        break;
      }
    }

    // Add the new blobs to the outputBlobs linked list
    for (blob_t* blob = iterator_start_from_head(blobsToAdd_ptr); blob != NULL; blob = iterator_next(blob)) {
      llist_push_back(outputBlobs_ptr, blob);
      if (DEBUG_BLOB) Serial.printf(F("\n DEBUG_BLOB / Blob: %p added to **outputBlobs** linked list"), blob);
      // Add the blobs values to an OSC bundle
      //message.addIntArg(blob->UID);
      //message.addIntArg(blob->centroid.X);
      //message.addIntArg(blob->centroid.Y);
      //message.addIntArg(blob->centroid.Z);
      //message.addIntArg(blob->pixels);
      //bundle.addMessage(message);
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

    // Send the blobs values with an OSC bundle
    //sender.sendBundle(bundle);

    llist_raz(blobsToUpdate_ptr);
    llist_raz(blobsToAdd_ptr);

  } else {
    llist_save_blobs(freeBlobs_ptr, blob_ptr);
  }
  if (DEBUG_BLOB) Serial.printf(F("\n DEBUG_BLOB / END OFF BLOB FONCTION"));
}
