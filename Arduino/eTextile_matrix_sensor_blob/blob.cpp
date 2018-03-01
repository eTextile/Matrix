/*
   This file is part of the OpenMV project - https://github.com/openmv/openmv
   Copyright (c) 2013-2017 Ibrahim Abdelkader <iabdalkader@openmv.io> & Kwabena W. Agyeman <kwagyeman@openmv.io>
   This work is licensed under the MIT license, see the file LICENSE for details.

   This file has been modified to fit the eTextile matrix sensor needs
   eTextile matrix sensor - http://matrix.eTextile.org
*/

#include <Arduino.h>
#include "config.h"
#include "collections.h"

void find_blobs(
  image_t*            input_ptr,
  char*               bitmap_ptr,
  const int           rows,
  const int           cols,
  const int           pixelThreshold,
  const unsigned int  minBlobPix,
  const unsigned int  maxBlobPix,
  lifo_t*             lifo_ptr,
  list_t*             freeBlobs_ptr,
  list_t*             blobs_ptr,
  list_t*             blobsToUpdate_ptr,
  list_t*             blobsToAdd_ptr,
  list_t*             outputBlobs_ptr
) {
  if (DEBUG_BLOB) Serial.printf(F("\n>>>>>>>>>>>>>>>>>>>> DEBUG_CCL / START"));

  for (uint8_t posY = 0; posY < rows; posY++) {
    uint8_t* row_ptr_A = ROW_PTR(input_ptr, posY);  // Return pointer to image curent row
    size_t row_index_A = ROW_INDEX(input_ptr, posY); // Return bitmap curent row

    if (DEBUG_CCL_INPUT) Serial.println();
    for (uint8_t posX = 0; posX < cols; posX++) {
      if (DEBUG_CCL_INPUT) Serial.printf("%d ", GET_PIXEL(row_ptr_A, posX)); // Print out INPUT_VALUES

      if (!bitmap_bit_get(bitmap_ptr, BITMAP_INDEX(row_index_A, posX)) && PIXEL_THRESHOLD(GET_PIXEL(row_ptr_A, posX), pixelThreshold)) {

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

          uint8_t* row_ptr_B = ROW_PTR(input_ptr, posY);  // Return pointer to image curent row
          size_t row_index_B = ROW_INDEX(input_ptr, posY);  // Return bitmap curent row

          while ((left > 0) &&
                 (!bitmap_bit_get(bitmap_ptr, BITMAP_INDEX(row_index_B, left - 1))) &&
                 PIXEL_THRESHOLD(GET_PIXEL(row_ptr_B, left - 1), pixelThreshold)) {
            left--;
            // if (DEBUG_CCL) Serial.printf(" Left:%d", left);
          }
          while (right < (cols - 1) &&
                 (!bitmap_bit_get(bitmap_ptr, BITMAP_INDEX(row_index_B, right + 1))) &&
                 PIXEL_THRESHOLD(GET_PIXEL(row_ptr_B, right + 1), pixelThreshold)) {
            right++;
            // if (DEBUG_CCL) Serial.printf(" Right:%d", right);
          }

          blob_x1 = IM_MIN(blob_x1, left);
          blob_y1 = IM_MIN(blob_y1, posY);
          blob_x2 = IM_MAX(blob_x2, right);
          blob_y2 = IM_MAX(blob_y2, posY);

          if (DEBUG_CCL) Serial.printf(F("\n>>>>>>>> DEBUG_CCL / Bitmap bit set: "));
          for (int i = left; i <= right; i++) {
            bitmap_bit_set(bitmap_ptr, BITMAP_INDEX(row_index_B, i));
            if (DEBUG_CCL) Serial.printf("%d ", BITMAP_INDEX(row_index_B, i));
            blob_pixels++;
            blob_cx += i;
            blob_cy += posY;
          }
          if (DEBUG_CCL) Serial.printf(F("\n>>>>>>>> DEBUG_CCL / Blob pixels: %d"), blob_pixels);

          boolean break_out = false;

          for (;;) {

            if (lifo_size(lifo_ptr) < NEW_FRAME) {

              if (posY > 0) {

                row_ptr_B = ROW_PTR(input_ptr, posY - 1);
                row_index_B = ROW_INDEX(input_ptr, posY - 1);

                boolean recurse = false;

                for (int i = left; i <= right; i++) {
                  if ((!bitmap_bit_get(bitmap_ptr, BITMAP_INDEX(row_index_B, i))) &&
                      PIXEL_THRESHOLD(GET_PIXEL(row_ptr_B, i), pixelThreshold)) {
                    if (DEBUG_CCL) Serial.printf(F("\n>>>>>>>> DEBUG_CCL / A-Lifo add pixel"));

                    xylf_t pixel;

                    pixel.x = posX;
                    pixel.y = posY;
                    pixel.l = left;
                    pixel.r = right;
                    lifo_enqueue(lifo_ptr, &pixel);
                    posX = i;
                    posY--;
                    recurse = true;
                    break;
                  }
                }
                if (recurse) {
                  break;
                }
              }
              if (DEBUG_CCL) Serial.printf(F("\n>>>>>>>> DEBUG_CCL / Break 1"));

              if (posY < (rows - 1)) {

                row_ptr_B = ROW_PTR(input_ptr, posY + 1);  // Return pointer to image curent row
                row_index_B = ROW_INDEX(input_ptr, posY + 1);  // Return bitmap curent row

                boolean recurse = false;

                for (int i = left; i <= right; i++) {
                  if ((!bitmap_bit_get(bitmap_ptr, BITMAP_INDEX(row_index_B, i))) &&
                      PIXEL_THRESHOLD(GET_PIXEL(row_ptr_B, i), pixelThreshold)) {

                    xylf_t pixel;
                    pixel.x = posX;
                    pixel.y = posY;
                    pixel.l = left;
                    pixel.r = right;
                    if (DEBUG_CCL) Serial.printf(F("\n>>>>>>>> DEBUG_CCL / B-Lifo add pixel"));
                    lifo_enqueue(lifo_ptr, &pixel);
                    posX = i;
                    posY++;
                    recurse = true;
                    break;
                  }
                }
                if (recurse) {
                  break;
                }
              }
              if (DEBUG_CCL) Serial.printf(F("\n>>>>>>>> DEBUG_CCL / Break 2"));
            }

            if (lifo_size(lifo_ptr) == 0) {
              if (DEBUG_CCL) Serial.printf(F("\n>>>>>>>> DEBUG_CCL / Lifo umpty: %d"), lifo_size(lifo_ptr));
              break_out = true;
              break;
            }

            xylf_t pixel;
            if (DEBUG_CCL) Serial.printf(F("\n>>>>>>>> DEBUG_CCL / Lifo take a pixel from the queue"));
            lifo_dequeue(lifo_ptr, &pixel);
            posX = pixel.x;
            posY = pixel.y;
            left = pixel.l;
            right = pixel.r;
          }

          if (break_out) {
            break;
          }
        }

        if ((blob_pixels >= minBlobPix) && (blob_pixels <= maxBlobPix)) {

          uint8_t cx = (uint8_t) blob_cx / blob_pixels; // x centroid position
          uint8_t cy = (uint8_t) blob_cy / blob_pixels; // y centroid position
          uint8_t* row_ptr = ROW_PTR(input_ptr, cy);
          uint8_t cz = GET_PIXEL(row_ptr, cx);

          blob_t* blob = list_pop_front(freeBlobs_ptr);
          if (DEBUG_CCL || DEBUG_BLOB) Serial.printf(F("\n DEBUG_BLOB / Get a blob from **freeBlobs** linked list: %p"), blob);

          // blob->UID = -1; // RAZ UID, we will give it an ID later
          blob->centroid.x = cx;
          blob->centroid.y = cy;
          blob->centroid.z = cz; // - THRESHOLD
          blob->pixels = blob_pixels;
          blob->isDead = false;

          list_push_back(blobs_ptr, blob);
          if (DEBUG_CCL || DEBUG_BLOB) Serial.printf(F("\n DEBUG_BLOB / Blob added to the **blobs** linked list: %p"), blob);
        }

        posX = old_x;
        posY = old_y;
      }
    }
  }

  lifo_init(lifo_ptr);
  if (DEBUG_BITMAP) bitmap_print(bitmap_ptr);
  bitmap_clear(bitmap_ptr); // TODO: optimizing!

  if (DEBUG_CCL_INPUT) Serial.println();
  if (DEBUG_CCL) Serial.print(F("\n DEBUG_CCL / END of scanline flood fill algorithm"));
  if (DEBUG_BLOB || DEBUG_CCL) Serial.printf(F("\n DEBUG_BLOB / Input **blobs** linked list index: %d"), blobs_ptr->index);
  if (DEBUG_BLOB || DEBUG_CCL) Serial.printf(F("\n DEBUG_BLOB / **freeBlobs** linked list index : %d"), freeBlobs_ptr->index);

  ///////////////////////////////////////////////////////////////////////////////////////// Percistant blobs

  if (PERCISTANT) {

    // Look for the nearest blob between curent blob position (blobs linked list) and last blob position (outputBlobs linked list)
    for (blob_t* blobA = iterator_start_from_head(blobs_ptr); blobA != NULL; blobA = iterator_next(blobA)) {
      float minDist = 1000;
      blob_t* nearestBlob = NULL;
      for (blob_t* blobB = iterator_start_from_head(outputBlobs_ptr); blobB != NULL; blobB = iterator_next(blobB)) {
        int16_t xa = blobA->centroid.x;
        int16_t ya = blobA->centroid.y;
        int16_t xb = blobB->centroid.x;
        int16_t yb = blobB->centroid.y;
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
        if (DEBUG_BLOB) Serial.printf(F("\n DEBUG_BLOB / Found corresponding blob in the **outputBlobs** linked list: %p"), nearestBlob);
        blobA->UID = nearestBlob->UID;
        if (DEBUG_BLOB) Serial.printf(F("\n DEBUG_BLOB / Copy the corresponding **outputBlobs** ID: %d to the incoming blob ID"), nearestBlob->UID);
        list_push_back(blobsToUpdate_ptr, blobA);
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
        list_push_back(blobsToAdd_ptr, blobA);
        if (DEBUG_BLOB) Serial.printf(F("\n DEBUG_BLOB / New incoming blob: %p pushed back to the **blobsToAdd** linked list"), blobA); // STOP if two BLOBs are in the list!
      }
    }

    // Update outputBlobs linked list with blobsToUpdate linked list.
    // If a blob in the outputBlobs linked list is not in the blobsToUpdate linked list, set it to dead.
    for (blob_t* blobA = iterator_start_from_head(outputBlobs_ptr); blobA != NULL; blobA = iterator_next(blobA)) {
      boolean found = false;
      for (blob_t* blobB = iterator_start_from_head(blobsToUpdate_ptr); blobB != NULL; blobB = iterator_next(blobB)) {
        // if (blobA->UID == blobB->UID  && blobA != blobB) {
        if (blobA->UID == blobB->UID) {
          list_copy_blob(blobA, blobB);
          if (DEBUG_BLOB) Serial.printf(F("\n DEBUG_BLOB / BlobA: %p updated with blobB: %p"), blobA, blobB);
          blobB->UID = -1; // RAZ blobB UID
          list_push_back(freeBlobs_ptr, blobB);
          if (DEBUG_BLOB) Serial.printf(F("\n DEBUG_BLOB / BlobB: %p saved to **freeBlobList** linked list - FLAG_A"), blobB);
          // Add the blobs values to an OSC bundle
          //message.addIntArg(blobA->UID);
          //message.addIntArg(blobA->centroid.x);
          //message.addIntArg(blobA->centroid.y);
          //message.addIntArg(blobA->centroid.z);
          //message.addIntArg(blobA->pixels);
          //bundle.addMessage(message);
          if (DEBUG_OSC) {
            Serial.printf(F("\n DEBUG_OSC / UID: %d\tX: %d\tY: %d\tZ: %d\tPIX: %d"),
                          blobA->UID,
                          blobA->centroid.x,
                          blobA->centroid.y,
                          blobA->centroid.z,
                          blobA->pixels
                         );
          }
          found = true;
          break;
        }
      }
      if (!found) {
        blobA->isDead = true;
      }
    }
    
    // Suppress dead blobs from the outputBlobs linked list
    // boolean deadExists = true;
    while (1) {
      blob_t* deadBlob = NULL;
      for (blob_t* blob = iterator_start_from_head(outputBlobs_ptr); blob != NULL; blob = iterator_next(blob)) {
        if (blob->isDead) {
          deadBlob = blob;
          break;
        }
      }
      if (NULL != deadBlob) {
        list_remove_blob(outputBlobs_ptr, deadBlob);
        if (DEBUG_BLOB) Serial.printf(F("\n DEBUG_BLOB / Blob: %p removed from **outputBlobs** linked list - FLAG_B"), deadBlob);
        // deadBlob->UID = -1; // RAZ deadBlob UID
        list_push_back(freeBlobs_ptr, deadBlob);
        if (DEBUG_BLOB) Serial.printf(F("\n DEBUG_BLOB / Blob: %p saved to **freeBlobList** linked list"), deadBlob);
        // Add the blobs values to an OSC bundle
        // message.addIntArg(deadBlob.UID);
        // message.addIntArg(-1);
        // message.addIntArg(-1);
        // message.addIntArg(-1);
        // message.addIntArg(-1);
        // bundle.addMessage(message);
        if (DEBUG_OSC) {
          Serial.printf(F("\n DEBUG_OSC / UID: %d\tX: %d\tY: %d\tZ: %d\tPIX: %d"), deadBlob->UID, -1, -1, -1, -1);
        }
      } else {
        // deadExists = false;
        break;
      }
    }

    // Add the new blobs to the outputBlobs linked list
    for (blob_t* blob = iterator_start_from_head(blobsToAdd_ptr); blob != NULL; blob = iterator_next(blob)) {
      list_push_back(outputBlobs_ptr, blob);
      if (DEBUG_BLOB) Serial.printf(F("\n DEBUG_BLOB / Blob: %p added to **outputBlobs** linked list"), blob);
      // Add the blobs values to an OSC bundle
      //message.addIntArg(blob->UID);
      //message.addIntArg(blob->centroid.x);
      //message.addIntArg(blob->centroid.y);
      //message.addIntArg(blob->centroid.z);
      //message.addIntArg(blob->pixels);
      //bundle.addMessage(message);
      if (DEBUG_OSC) {
        Serial.printf(F("\n DEBUG_OSC / UID: %d\tX: %d\tY: %d\tZ: %d\tPIX: %d"),
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

  } else {
    list_save_blobs(freeBlobs_ptr, blobs_ptr);
  }

  list_init(blobs_ptr);
  list_init(blobsToUpdate_ptr);
  list_init(blobsToAdd_ptr);

  if (DEBUG_BLOB) Serial.printf(F("\n DEBUG_BLOB / Cleared bitmap"));
  if (DEBUG_BLOB) Serial.printf(F("\n DEBUG_BLOB / END OFF BLOB FONCTION"));

}
