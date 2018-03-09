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
  lifo_t*             freeNodes_ptr,
  lifo_t*             lifo_ptr,
  llist_t*            freeBlobs_ptr,
  llist_t*            blob_ptr,
  llist_t*            blobsToUpdate_ptr,
  llist_t*            blobsToAdd_ptr,
  llist_t*            outputBlobs_ptr
) {
  if (DEBUG_BLOB || DEBUG_CCL) Serial.printf(F("\n>>>>>>>>>>>>>>>>>>>> DEBUG / START\n"));

  for (uint8_t posY = 0; posY < rows; posY++) {
    uint8_t* row_ptr = ROW_PTR(inFrame_ptr, posY);     // Return inFrame_ptr curent row pointer
    uint16_t row_index = ROW_INDEX(inFrame_ptr, posY); // Return inFrame_ptr curent row index (1D array) 0, 64, 128,... 4032

    // if (DEBUG_CCL) Serial.printf(F("\n DEBUG_CCL / row_ptr:%p \t row_index:%d\n"), row_ptr, row_index); // Print out INPUT_VALUES
    for (uint8_t posX = 0; posX < cols; posX++) {
      // if (DEBUG_BITMAP) Serial.printf(F("%d "), GET_PIXEL(row_ptr, posX)); // Print INPUT_VALUES
      if (DEBUG_BITMAP) Serial.printf(F("%d "), PIXEL_THRESHOLD(GET_PIXEL(row_ptr, posX), pixelThreshold));
      // if (DEBUG_BITMAP) Serial.printf(F("%d "), BITMAP_INDEX(row_index, posX));

      if (!bitmap_bit_get(bitmap_ptr, BITMAP_INDEX(row_index, posX)) &&
          PIXEL_THRESHOLD(GET_PIXEL(row_ptr, posX), pixelThreshold)) {

        uint8_t old_x = posX;
        uint8_t old_y = posY;
        uint8_t blob_x1 = posX;
        uint8_t blob_y1 = posY;
        uint8_t blob_x2 = posX;
        uint8_t blob_y2 = posY;
        uint16_t blob_pixels = 0;
        uint8_t blob_cx = 0;
        uint8_t blob_cy = 0;
        uint8_t blob_cz = 0;

        ////////// Connected-component labeling / Scanline flood fill algorithm //////////

        for (;;) {

          uint8_t left = posX;
          uint8_t right = posX;

          row_ptr = ROW_PTR(inFrame_ptr, posY);     // Return inFrame_ptr curent row pointer
          row_index = ROW_INDEX(inFrame_ptr, posY); // Return inFrame_ptr curent row index (1D array) 0, 64, 128,... 4032

          while ((left > 0) &&
                 (!bitmap_bit_get(bitmap_ptr, BITMAP_INDEX(row_index, left - 1))) &&
                 PIXEL_THRESHOLD(GET_PIXEL(row_ptr, left - 1), pixelThreshold)) {
            left--;
            if (DEBUG_CCL) Serial.printf(F(" Left:%d"), left);
          }
          while (right < (cols - 1) &&
                 (!bitmap_bit_get(bitmap_ptr, BITMAP_INDEX(row_index, right + 1))) &&
                 PIXEL_THRESHOLD(GET_PIXEL(row_ptr, right + 1), pixelThreshold)) {
            right++;
            if (DEBUG_CCL) Serial.printf(F(" Right:%d"), right);
          }

          blob_x1 = IM_MIN(blob_x1, left);
          blob_y1 = IM_MIN(blob_y1, posY);
          blob_x2 = IM_MAX(blob_x2, right);
          blob_y2 = IM_MAX(blob_y2, posY);

          if (DEBUG_CENTER) Serial.printf(F("\n DEBUG_CENTER / blob_x1:%d\tblob_y1:%d\tblob_x2:%d\tblob_y2:%d"));
          if (DEBUG_CCL) Serial.printf(F("\n DEBUG_CCL / Save blob line to the bitmap array"));
          for (uint8_t i = left; i <= right; i++) {
            bitmap_bit_set(bitmap_ptr, BITMAP_INDEX(row_index, i));
            blob_pixels++;
            blob_cx += i;
            blob_cy += posY;
          }
          if (DEBUG_CCL) Serial.printf(F("\n DEBUG_CCL / Blob pixels: % d"), blob_pixels);

          bool break_out = false;
          for (;;) {
            if (lifo_size(lifo_ptr) < lifo_ptr->max_nodes) {
              if (posY > 0) {
                row_ptr = ROW_PTR(inFrame_ptr, posY - 1);
                row_index = ROW_INDEX(inFrame_ptr, posY - 1);

                bool recurse = false;
                for (uint8_t i = left; i <= right; i++) {
                  if ((!bitmap_bit_get(bitmap_ptr, BITMAP_INDEX(row_index, i))) &&
                      PIXEL_THRESHOLD(GET_PIXEL(row_ptr, i), pixelThreshold)) {

                    if (DEBUG_CCL || DEBUG_LIFO) Serial.printf(F("\n DEBUG_LIFO / Take a node from freeNodes and add it to the lifo"));
                    xylf_t* pixel = lifo_dequeue(freeNodes_ptr);
                    pixel->x = posX;
                    pixel->y = posY;
                    pixel->l = left;
                    pixel->r = right;
                    lifo_enqueue(lifo_ptr, pixel);
                    posX = i;
                    posY--;
                    recurse = true;
                    break;
                  }
                }
                if (recurse) {
                  if (DEBUG_CCL) Serial.printf(F("\n DEBUG_CCL / Break 1"));
                  break;
                }
              }

              if (posY < (rows - 1)) {
                row_ptr = ROW_PTR(inFrame_ptr, posY + 1);  // Return pointer to image curent row
                row_index = ROW_INDEX(inFrame_ptr, posY + 1);  // Return bitmap curent row

                bool recurse = false;
                for (uint8_t i = left; i <= right; i++) {
                  if ((!bitmap_bit_get(bitmap_ptr, BITMAP_INDEX(row_index, i))) &&
                      PIXEL_THRESHOLD(GET_PIXEL(row_ptr, i), pixelThreshold)) {
                    if (DEBUG_CCL || DEBUG_LIFO) Serial.printf(F("\n DEBUG_LIFO / Take a node from freeNodes and add it to the lifo"));
                    xylf_t* pixel = lifo_dequeue(freeNodes_ptr);
                    pixel->x = posX;
                    pixel->y = posY;
                    pixel->l = left;
                    pixel->r = right;
                    lifo_enqueue(lifo_ptr, pixel);
                    posX = i;
                    posY++;
                    recurse = true;
                    break;
                  }
                }
                if (recurse) {
                  if (DEBUG_CCL) Serial.printf(F("\n DEBUG_CCL / Break 2"));
                  break;
                }
              }
            }

            if (lifo_size(lifo_ptr) == -1) {
              if (DEBUG_CCL) Serial.printf(F("\n DEBUG_CCL / Lifo umpty: % d"), lifo_size(lifo_ptr));
              break_out = true;
              break;
            }

            if (DEBUG_CCL || DEBUG_LIFO) Serial.printf(F("\n DEBUG_LIFO / -- Take a node from the lifo"));
            xylf_t* pixel = lifo_dequeue(lifo_ptr);
            posX = pixel->x;
            posY = pixel->y;
            left = pixel->l;
            right = pixel->r;
            if (DEBUG_CCL || DEBUG_LIFO) Serial.printf(F("\n DEBUG_LIFO / -- Save it to freeNodes"));
            lifo_enqueue(freeNodes_ptr, pixel);
          }

          if (break_out) {
            if (DEBUG_CCL || DEBUG_LIFO) Serial.printf(F("\n DEBUG_LIFO / -- EXIT"));
            break;
          }
        }

        if ((blob_pixels > minBlobPix) && (blob_pixels < maxBlobPix) && (llist_size(freeBlobs_ptr) > -1)) {
          blob_t* blob = llist_pop_front(freeBlobs_ptr);
          if (DEBUG_CCL || DEBUG_BLOB) Serial.printf(F("\n DEBUG_BLOB / Get a blob from **freeBlobs** linked list: % p"), blob);

          int mx = blob_cx / blob_pixels; // x centroid
          int my = blob_cy / blob_pixels; // y centroid
          if (DEBUG_CENTER) Serial.printf(F("\n DEBUG_CENTER / blob_cx: % d\tblob_cy: % d\tblob_cz: % d"), mx, my, blob_cz);

          blob->UID = -1; // RAZ UID, we will give it an ID later
          blob->centroid.x = mx;
          blob->centroid.y = my;
          blob->centroid.z = blob_cz; // - THRESHOLD
          blob->pixels = blob_pixels;
          blob->isDead = false;
          llist_push_back(blob_ptr, blob);
          if (DEBUG_CCL || DEBUG_BLOB) Serial.printf(F("\n DEBUG_BLOB / Blob added to the **blobs** linked list: % p"), blob);
        }
        
        posX = old_x;
        posY = old_y;
      }
    }
    if (DEBUG_BITMAP) Serial.println();
  }
  if (DEBUG_BITMAP) Serial.println();
  
  // lifo_raz(lifo_ptr); // Do not nead!
  bitmap_clear(bitmap_ptr);

  if (DEBUG_CCL) Serial.print(F("\n DEBUG_CCL / END of scanline flood fill algorithm"));
  if (DEBUG_BLOB || DEBUG_CCL) Serial.printf(F("\n DEBUG_BLOB / Input **blobs** linked list index: % d"), blob_ptr->index);
  if (DEBUG_BLOB || DEBUG_CCL) Serial.printf(F("\n DEBUG_BLOB / **freeBlobs** linked list index : % d"), freeBlobs_ptr->index);

  ///////////////////////////////////////////////////////////////////////////////////////// Percistant blobs

#ifdef PERSISTANT

  // Look for the nearest blob between curent blob position (blobs linked list) and last blob position (outputBlobs linked list)
  for (blob_t* blobA = iterator_start_from_head(blob_ptr); blobA != NULL; blobA = iterator_next(blobA)) {
    float minDist = 100;
    blob_t* nearestBlob = NULL;
    for (blob_t* blobB = iterator_start_from_head(outputBlobs_ptr); blobB != NULL; blobB = iterator_next(blobB)) {
      int16_t xa = blobA->centroid.x;
      int16_t ya = blobA->centroid.y;
      int16_t xb = blobB->centroid.x;
      int16_t yb = blobB->centroid.y;
      float dist = sqrt(pow(xa - xb, 2) + pow(ya - yb, 2)); // fast_sqrt? & fast_pow? // arm_sqrt_f32(); ?

      if (dist < minDist) {
        minDist = dist;
        if (DEBUG_BLOB) Serial.printf(F("\n DEBUG_BLOB / Found nearest blob % p at: % f"), blobB, minDist);
        nearestBlob = blobB;
      }
    }

    // If the distance between curent blob and last blob position is less than minDist:
    // We take the ID of the nearestBlob in outputBlobs linked list and give it to the curent input blob.
    // We move the curent blob to the blobsToUpdate linked list.
    if (minDist < 4) {
      if (DEBUG_BLOB) Serial.printf(F("\n DEBUG_BLOB / Found corresponding blob: % p in the **outputBlobs** linked list"), nearestBlob);
      blobA->UID = nearestBlob->UID;
      if (DEBUG_BLOB) Serial.printf(F("\n DEBUG_BLOB / Copy the corresponding **outputBlobs** ID: % d to the incoming blob ID"), nearestBlob->UID);
      llist_push_back(blobsToUpdate_ptr, blobA);
      if (DEBUG_BLOB) Serial.printf(F("\n DEBUG_BLOB / BlobA: % p pushed back to the **blobsToUpdate** linked list"), blobA);
    } else {
      // Found a new blob! we nead to give it an ID.
      // We look for the minimum unused ID through the outputBlobs linked list &
      // We add the new blob to the nodesToAdd linked list.
      if (DEBUG_BLOB) Serial.printf(F("\n DEBUG_BLOB / Found new blob without ID"));
      bool isFree = false;
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
      if (DEBUG_BLOB) Serial.printf(F("\n DEBUG_BLOB / ID: % d seted to the new incoming blob: % p"), minID, blobA);
      llist_push_back(blobsToAdd_ptr, blobA);
      if (DEBUG_BLOB) Serial.printf(F("\n DEBUG_BLOB / New incoming blob: % p pushed back to the **blobsToAdd** linked list"), blobA);
    }
  }

  // Update outputBlobs linked list with blobsToUpdate linked list.
  // If a blob in the outputBlobs linked list is not in the blobsToUpdate linked list, set it to dead.
  for (blob_t* blobA = iterator_start_from_head(outputBlobs_ptr); blobA != NULL; blobA = iterator_next(blobA)) {
    bool found = false;
    for (blob_t* blobB = iterator_start_from_head(blobsToUpdate_ptr); blobB != NULL; blobB = iterator_next(blobB)) {
      // if (blobA->UID == blobB->UID) {
      if (blobA->UID == blobB->UID  && blobA != blobB) {
        llist_update_blob(blobA, blobB);
        if (DEBUG_BLOB) Serial.printf(F("\n DEBUG_BLOB / BlobA: % p updated with blobB: % p"), blobA, blobB);
        blobB->UID = -1; // RAZ blobB UID
        llist_push_back(freeBlobs_ptr, blobB);
        if (DEBUG_BLOB) Serial.printf(F("\n DEBUG_BLOB / BlobB: % p saved to **freeBlobList** linked list"), blobB);
        // Add the blobs values to an OSC bundle
        //message.addIntArg(blobA->UID);
        //message.addIntArg(blobA->centroid.x);
        //message.addIntArg(blobA->centroid.y);
        //message.addIntArg(blobA->centroid.z);
        //message.addIntArg(blobA->pixels);
        //bundle.addMessage(message);
        if (DEBUG_OSC) {
          Serial.printf(F("\n DEBUG_OSC / UID: % d\tX: % d\tY: % d\tZ: % d\tPIX: % d"),
                        blobA->UID,
                        blobA->centroid.x,
                        blobA->centroid.y,
                        blobA->centroid.z,
                        blobA->pixels
                       );
        }
        found = true;
      }
      break;
    }
    if (!found) {
      blobA->isDead = true;
      if (DEBUG_BLOB) Serial.printf(F("\n DEBUG_BLOB / Found dead blob: % p in the **outputBlobs** linked list"), blobA);
    }
  }

  // Suppress dead blobs from the outputBlobs linked list
  while (1) {
    blob_t* deadBlob = NULL;
    for (blob_t* blob = iterator_start_from_head(outputBlobs_ptr); blob != NULL; blob = iterator_next(blob)) {
      if (blob->isDead) {
        deadBlob = blob;
        if (DEBUG_BLOB) Serial.printf(F("\n DEBUG_BLOB / Blob: % p from **outputBlobs** linked list - FLAG_FLAG"), deadBlob);
        break;
      }
    }
    if (deadBlob != NULL) {
      llist_remove_blob(outputBlobs_ptr, deadBlob);
      if (DEBUG_BLOB) Serial.printf(F("\n DEBUG_BLOB / Blob: % p removed from **outputBlobs** linked list - FLAG_B"), deadBlob);
      // deadBlob->UID = -1; // RAZ deadBlob UID
      llist_push_back(freeBlobs_ptr, deadBlob);
      if (DEBUG_BLOB) Serial.printf(F("\n DEBUG_BLOB / Blob: % p saved to **freeBlobList** linked list"), deadBlob);
      // Add the blobs values to an OSC bundle
      // message.addIntArg(deadBlob.UID);
      // message.addIntArg(-1);
      // message.addIntArg(-1);
      // message.addIntArg(-1);
      // message.addIntArg(-1);
      // bundle.addMessage(message);
      if (DEBUG_OSC) {
        Serial.printf(F("\n DEBUG_OSC / UID: % d\tX: % d\tY: % d\tZ: % d\tPIX: % d"), deadBlob->UID, -1, -1, -1, -1);
      }
    } else {
      break;
    }
  }

  // Add the new blobs to the outputBlobs linked list
  for (blob_t* blob = iterator_start_from_head(blobsToAdd_ptr); blob != NULL; blob = iterator_next(blob)) {
    llist_push_back(outputBlobs_ptr, blob);
    if (DEBUG_BLOB) Serial.printf(F("\n DEBUG_BLOB / Blob: % p added to **outputBlobs** linked list"), blob);
    // Add the blobs values to an OSC bundle
    //message.addIntArg(blob->UID);
    //message.addIntArg(blob->centroid.x);
    //message.addIntArg(blob->centroid.y);
    //message.addIntArg(blob->centroid.z);
    //message.addIntArg(blob->pixels);
    //bundle.addMessage(message);
    if (DEBUG_OSC) {
      Serial.printf(F("\n DEBUG_OSC / UID: % d\tX: % d\tY: % d\tZ: % d\tPIX: % d"),
                    blob->UID,
                    blob->centroid.x,
                    blob->centroid.y,
                    blob->centroid.z,
                    blob->pixels
                   );
    }
  }

  // Send the blobs values with an OSC bundle
  //sender.sendBundle(bundle);

  llist_raz(blobsToUpdate_ptr);
  llist_raz(blobsToAdd_ptr);

#else
  llist_save_blobs(freeBlobs_ptr, blob_ptr);
#endif /*__PERSISTANT__*/

  llist_raz(blob_ptr);

  if (DEBUG_BLOB) Serial.printf(F("\n DEBUG_BLOB / Cleared bitmap"));
  if (DEBUG_BLOB) Serial.printf(F("\n DEBUG_BLOB / END OFF BLOB FONCTION"));
}
