/*
   This file is part of the OpenMV project
   Copyright (c) 2013-2017 Ibrahim Abdelkader <iabdalkader@openmv.io> & Kwabena W. Agyeman <kwagyeman@openmv.io>
   This work is licensed under the MIT license, see the file LICENSE for details.
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
  list_t*             freeBlobList_ptr,
  list_t*             blobs_ptr,
  list_t*             blobsToUpdate_ptr,
  list_t*             blobsToAdd_ptr,
  list_t*             outputBlobs_ptr
) {

  for (int posY = 0; posY < rows; posY++) {

    uint8_t* row_ptr = ROW_PTR(input_ptr, posY);  // Return pointer to image curent row
    size_t row_index = ROW_INDEX(input_ptr, posY); // Return bitmap curent row

    for (int posX = 0; posX < cols; posX++) {
      if (DEBUG_INPUT) Serial.printf("%d ", GET_PIXEL(row_ptr, posX)); // <<<< INPUT VALUES!!

      if (!bitmap_bit_get(bitmap_ptr, BITMAP_INDEX(row_index, posX)) &&
          PIXEL_THRESHOLD(GET_PIXEL(row_ptr, posX), pixelThreshold)) {

        if (DEBUG_BLOB) Serial.printf(F("\n>>>>>>>> RAZ BLOB!\n"));

        int old_x = posX;
        int old_y = posY;

        int blob_x1 = posX;
        int blob_y1 = posY;
        int blob_x2 = posX;
        int blob_y2 = posY;
        int blob_pixels = 0;
        int blob_cx = 0;
        int blob_cy = 0;
        int blob_cz = 0;

        ////////// Connected-component labeling / Scanline flood fill algorithm //////////

        for (;;) {

          int left = posX;
          int right = posX;

          uint8_t* row_ptr_B = ROW_PTR(input_ptr, posY);  // Return pointer to image curent row
          size_t row_index_B = ROW_INDEX(input_ptr, posY);  // Return bitmap curent row

          while ((left > 0) &&
                 (!bitmap_bit_get(bitmap_ptr, BITMAP_INDEX(row_index_B, left - 1))) &&
                 PIXEL_THRESHOLD(GET_PIXEL(row_ptr_B, left - 1), pixelThreshold)) {
            left--;
            if (DEBUG_CCL) Serial.printf(" Left:%d", left);
          }
          while (right < (cols - 1) &&
                 (!bitmap_bit_get(bitmap_ptr, BITMAP_INDEX(row_index_B, right + 1))) &&
                 PIXEL_THRESHOLD(GET_PIXEL(row_ptr_B, right + 1), pixelThreshold)) {
            right++;
            if (DEBUG_CCL) Serial.printf(" Right:%d", right);
          }

          blob_x1 = IM_MIN(blob_x1, left);
          blob_y1 = IM_MIN(blob_y1, posY);
          blob_x2 = IM_MAX(blob_x2, right);
          blob_y2 = IM_MAX(blob_y2, posY);

          if (DEBUG_CCL) Serial.printf(F("\n>>>>>>>> Bitmap bit set: "));
          for (int i = left; i <= right; i++) {
            bitmap_bit_set(bitmap_ptr, BITMAP_INDEX(row_index_B, i));
            if (DEBUG_CCL) Serial.printf("%d ", BITMAP_INDEX(row_index_B, i));
            blob_pixels += 1;
            blob_cx += i;
            blob_cy += posY;
          }
          if (DEBUG_CCL) Serial.printf(F("\n>>>>>>>> Blob pixels: %d"), blob_pixels);

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
                    if (DEBUG_CCL) Serial.printf(F("\n>>>>>>>> A-Lifo add pixel"));

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
              if (DEBUG_CCL) Serial.printf(F("\n>>>>>>>> Break 1"));

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
                    if (DEBUG_CCL) Serial.printf(F("\n>>>>>>>> B-Lifo add pixel"));
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
              if (DEBUG_CCL) Serial.printf(F("\n>>>>>>>> Break 2"));
            }
            if (lifo_size(lifo_ptr) == -1) {
              if (DEBUG_CCL) Serial.printf(F("\n>>>>>>>> Lifo umpty: %d"), lifo_size(lifo_ptr));
              break_out = true;
              break;
            }

            xylf_t pixel;
            if (DEBUG_CCL) Serial.printf(F("\n>>>>>>>> Lifo take a pixel from the queue"));
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
        if (DEBUG_CCL) Serial.print(F("\n>>>>>>>> Scan / END of scanline flood fill algorithm"));

        if ((blob_pixels >= minBlobPix) && (blob_pixels <= maxBlobPix)) {

          uint16_t cx = (uint16_t) blob_cx / blob_pixels; // x centroid position
          uint16_t cy = (uint16_t) blob_cy / blob_pixels; // y centroid position
          row_ptr = ROW_PTR(input_ptr, cy);
          uint16_t cz = GET_PIXEL(row_ptr, cx);

          blob_t* blob = list_pop_front(freeBlobList_ptr);
          if (DEBUG_CCL) Serial.printf(F("\n>>>>>>>> Scan / Get a free blobs from freeBlobList_ptr: %p"), blob);

          blob->UID = -1; // We will give it an ID later
          blob->centroid.x = cx;
          blob->centroid.y = cy;
          blob->centroid.z = cz; // - THRESHOLD
          blob->pixels = blob_pixels;
          blob->isDead = false;

          list_push_back(blobs_ptr, blob);
          if (DEBUG_CCL) Serial.printf(F("\n>>>>>>>> Scan / add the blob to the blobs linked list: %p"), blob);
        }

        posX = old_x;
        posY = old_y;
      }
    }
  }

  if (DEBUG_BLOB) Serial.printf(F("\n>>>>>>>> freeBlobs linked list index : %d"), freeBlobList_ptr->index);
  if (DEBUG_BLOB) Serial.printf(F("\n>>>>>>>> Input blobs linked list index: %d"), blobs_ptr->index);

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
        float dist = sqrt(pow(xa - xb, 2) + pow(ya - yb, 2)); // fast_sqrt? & fast_pow?
        // arm_sqrt_f32(); ?

        if (dist < minDist) {
          minDist = dist;
          nearestBlob = blobB;
        }
      }

      // If the distance betwin curent blob and last blob position is less than minDist:
      // We take the ID of the nearestBlob in outputBlobs linked list and give it to the curent input blob.
      // We move the curent blob to the blobsToUpdate linked list.
      if (minDist < 10) {

        if (DEBUG_BLOB) Serial.printf(F("\n>>>>>>>> BLOB / we found the corresponding blob in the outputBlobs linked list: %p"), nearestBlob);
        blobA->UID = nearestBlob->UID;
        if (DEBUG_BLOB) Serial.printf(F("\n>>>>>>>> BLOB / we copy the outputBlobs ID to the new input blob: %d"), nearestBlob->UID);
        list_push_back(blobsToUpdate_ptr, blobA);      // Save the blob to the blobsToUpdate linked list
        if (DEBUG_BLOB) Serial.printf(F("\n>>>>>>>> BLOB / push back the blob to blobsToUpdate: %p"), blobA);
        // list_remove_blob(blobs_ptr, blobA);         // Remove the blob from the blobs linked list // Oups it is not a good solution hear!
        // if (DEBUG_BLOB) Serial.printf(F("\n>>>>>>>> BLOB / remove blob from input blobs linked list: %p"), blobA);
      } else {

        // Found a new blob! we nead to give it an ID.
        // We look for the minimum unused ID through the outputBlobs linked list &
        // We add the new blob to the nodesToAdd linked list.
        if (DEBUG_BLOB) Serial.printf(F("\n>>>>>>>> BLOB / we found new blob without ID!"));
        boolean isFree = false;
        uint8_t minID = 0;
        while (!isFree) {
          isFree = true;
          for (blob_t* blob = iterator_start_from_head(outputBlobs_ptr); blob != NULL; blob = iterator_next(blob)) {
            if (blob->UID == minID) {
              minID ++;
              isFree = false;
              break;
            }
          }
        }
        blobA->UID = minID;
        if (DEBUG_BLOB) Serial.printf(F("\n>>>>>>>> BLOB / the new blob ID is set to : %d"), minID);
        list_push_back(blobsToAdd_ptr, blobA);    // Save the blob to the blobsToAdd linked list
        if (DEBUG_BLOB) Serial.printf(F("\n>>>>>>>> BLOB / push back the blob to blobsToAdd: %p"), blobA); // STOP if two BLOBs are in the list!
        // list_remove_blob(blobs_ptr, blobA);       // Remove the blob from the blobs linked list // Oups it is not a good solution!
        // if (DEBUG_BLOB) Serial.printf(F("\n>>>>>>>> BLOB / remove blob from input blobs linked list: %p"), blobA);
      }
    }

    // Update outputBlobs linked list with blobsToUpdate linked list.
    // If the blob in the outputBlobs is not in the blobsToUpdate, set it to dead
    for (blob_t* blobA = iterator_start_from_head(outputBlobs_ptr); blobA != NULL; blobA = iterator_next(blobA)) {
      boolean found = false;
      for (blob_t* blobB = iterator_start_from_head(blobsToUpdate_ptr); blobB != NULL; blobB = iterator_next(blobB)) {
        if (blobA->UID == blobB->UID) {
          if (DEBUG_BLOB) Serial.printf(F("\n>>>>>>>> BLOB /  Update outputBlobs linked list with blobsToUpdate linked list: %p"), blobA);
          list_copy_blob(blobA, blobB, sizeof(blob_t)); // sizeof(blob_t) can be CONST!
          list_push_back(freeBlobList_ptr, blobB);  // Save the blob to the freeNodeList linked list
          found = true;
          break;
        }
      }
      if (!found) {
        // message.addIntArg(blobA.UID);
        // message.addIntArg(-1);
        // message.addIntArg(-1);
        // message.addIntArg(-1);
        // message.addIntArg(-1);
        // bundle.addMessage(message);
        if (DEBUG_OSC) {
          Serial.printf(F("\n>>>> UID: %d\tX: %d\tY: %d\tZ: %d\tPIX: %d"),
                        blobA->UID,
                        -1,
                        -1,
                        -1,
                        -1
                       );
        }
        blobA->isDead = true;
      }
    }

    // Suppress dead blobs from the outputBlobs linked list
    boolean deadExists = true;
    while (deadExists) {
      blob_t* deadBlob = NULL;
      for (blob_t* blob = iterator_start_from_head(outputBlobs_ptr); blob != NULL; blob = iterator_next(blob)) {
        if (blob->isDead) {
          deadBlob = blob;
          break;
        }
      }
      if (deadBlob != NULL) {
        list_push_back(freeBlobList_ptr, deadBlob);  // Save the blob to the freeNodeList linked list
        list_remove_blob(outputBlobs_ptr, deadBlob); // Remove the blob from the outputBlobs linked list
        if (DEBUG_BLOB) Serial.printf(F("\n>>>>>>>> BLOB / remove blob from outputBlobs linked list: %p"), deadBlob);
      } else {
        deadExists = false;
      }
    }

    // Add the new blobs to the outputBlobs linked list
    for (blob_t* blob = iterator_start_from_head(blobsToAdd_ptr); blob != NULL; blob = iterator_next(blob)) {
      list_push_back(outputBlobs_ptr, blob);
      if (DEBUG_BLOB) Serial.printf(F("\n>>>>>>>> BLOB / add blob to outputBlobs linked list: %p"), blob);
    }

    // Send the blobs values with an OSC bundle
    for (blob_t* blob = iterator_start_from_head(outputBlobs_ptr); blob != NULL; blob = iterator_next(blob)) {
      if (!blob->isDead) {
        // message.addIntArg(blob->UID);
        // message.addIntArg(blob->centroid.x);
        // message.addIntArg(blob->centroid.y);
        // message.addIntArg(blob->centroid.z);
        // message.addIntArg(blob->pixels);
        // bundle.addMessage(message);
        if (DEBUG_OSC) {
          Serial.printf(F("\n>>>> UID: %d\tX: %d\tY: %d\tZ: %d\tPIX: %d"),
                        blob->UID,
                        blob->centroid.x,
                        blob->centroid.y,
                        blob->centroid.z,
                        blob->pixels
                       );
        }
      }
    }
    // sender.sendBundle(bundle);

  }

  // list_save_blobs(freeBlobList_ptr, blobs_ptr); // TODO test if it work for more than one blob!

  lifo_init(lifo_ptr);

  list_init(blobs_ptr);
  list_init(blobsToUpdate_ptr);
  list_init(blobsToAdd_ptr);

  if (DEBUG_BITMAP) bitmap_print(bitmap_ptr);
  bitmap_clear(bitmap_ptr); // TODO: optimizing!
  if (DEBUG_BLOB) Serial.printf(F("\n>>>>>>>> Cleared bitmap"));
  if (DEBUG_BLOB) Serial.printf(F("\n>>>>>>>> END OFF BLOB FONCTION"));

}
