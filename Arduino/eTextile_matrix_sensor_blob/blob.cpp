/* This file is part of the OpenMV project.node
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
  list_t*             freeBlobList_ptr,
  list_t*             blobs_ptr,
  list_t*             oldBlobsToUpdate_ptr,
  list_t*             blobsToUpdate_ptr,
  list_t*             blobsToAdd_ptr,
  list_t*             outputBlobs_ptr
) {
  int blob_index = 0;

  if (DEBUG_BLOB) Serial.printf(F("\n>>>>>>>> STARTING BLOB SCANNING"));

  lifo_t lifo;
  size_t lifoLen;
  lifo_alloc_all(&lifo, &lifoLen, sizeof(xylf_t));

  if (DEBUG_BLOB) Serial.printf(F("\n>>>>>>>> A-Lifo len: %d"), lifoLen);
  if (DEBUG_BLOB) Serial.printf(F("\n>>>>>>>> A-Lifo size: %d"), lifo_size(&lifo));

  for (int posY = 0; posY < rows; posY++) {

    uint8_t* row_ptr = FRAME_ROW_PTR(input_ptr, posY);       // Return pointer to image curent row
    size_t row_index = BITMAP_ROW_INDEX(input_ptr, posY);    // Return bitmap curent row

    for (int posX = 0; posX < cols; posX++) {
      if (DEBUG_INPUT) Serial.printf("%d ", GET_FRAME_PIXEL(row_ptr, posX)); // <<<< INPUT VALUES!!

      if (!bitmap_bit_get(bitmap_ptr, BITMAP_INDEX(row_index, posX)) &&
          PIXEL_THRESHOLD(GET_FRAME_PIXEL(row_ptr, posX), pixelThreshold)) {

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

        ////////// Scanline flood fill algorithm //////////

        for (;;) {

          int left = posX;
          int right = posX;

          uint8_t* row_ptr_B = FRAME_ROW_PTR(input_ptr, posY);       // Return pointer to image curent row
          size_t row_index_B = BITMAP_ROW_INDEX(input_ptr, posY);    // Return bitmap curent row

          while ((left > 0) &&
                 (!bitmap_bit_get(bitmap_ptr, BITMAP_INDEX(row_index_B, left - 1))) &&
                 PIXEL_THRESHOLD(GET_FRAME_PIXEL(row_ptr_B, left - 1), pixelThreshold)) {
            left--;
            if (DEBUG_BLOB) Serial.printf(" Left:%d", left);
          }
          while (right < (cols - 1) &&
                 (!bitmap_bit_get(bitmap_ptr, BITMAP_INDEX(row_index_B, right + 1))) &&
                 PIXEL_THRESHOLD(GET_FRAME_PIXEL(row_ptr_B, right + 1), pixelThreshold)) {
            right++;
            if (DEBUG_BLOB) Serial.printf(" Right:%d", right);
          }

          blob_x1 = IM_MIN(blob_x1, left);
          blob_y1 = IM_MIN(blob_y1, posY);
          blob_x2 = IM_MAX(blob_x2, right);
          blob_y2 = IM_MAX(blob_y2, posY);

          if (DEBUG_BLOB) Serial.printf(F("\n>>>>>>>> Bitmap bit set: "));
          for (int i = left; i <= right; i++) {
            bitmap_bit_set(bitmap_ptr, BITMAP_INDEX(row_index_B, i));
            if (DEBUG_BLOB) Serial.printf("%d ", BITMAP_INDEX(row_index_B, i));
            blob_pixels += 1;
            // if (blob_pixels > 255) blob_pixels = 255;
            blob_cx += i;
            blob_cy += posY;
          }
          if (DEBUG_BLOB) Serial.printf(F("\n>>>>>>>> Blob pixels: %d"), blob_pixels);

          boolean break_out = false;

          for (;;) {

            if (lifo_size(&lifo) < lifoLen) {

              if (DEBUG_BLOB) Serial.printf(F("\n>>>>>>>> B-Lifo len: %d"), lifoLen );
              if (DEBUG_BLOB) Serial.printf(F("\n>>>>>>>> B-Lifo size: %d"), lifo_size(&lifo) );

              if (posY > 0) {

                row_ptr_B = FRAME_ROW_PTR(input_ptr, posY - 1);
                row_index_B = BITMAP_ROW_INDEX(input_ptr, posY - 1);

                boolean recurse = false;

                for (int i = left; i <= right; i++) {
                  if ((!bitmap_bit_get(bitmap_ptr, BITMAP_INDEX(row_index_B, i))) &&
                      PIXEL_THRESHOLD(GET_FRAME_PIXEL(row_ptr_B, i), pixelThreshold)) {
                    if (DEBUG_BLOB) Serial.printf(F("\n>>>>>>>> A-Lifo add pixel"));

                    xylf_t context;
                    context.x = posX;
                    context.y = posY;
                    context.l = left;
                    context.r = right;
                    lifo_enqueue(&lifo, &context);
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
              if (DEBUG_BLOB) Serial.printf(F("\n>>>>>>>> Break 1"));

              if (posY < (rows - 1)) {

                row_ptr_B = FRAME_ROW_PTR(input_ptr, posY + 1);       // Return pointer to image curent row
                row_index_B = BITMAP_ROW_INDEX(input_ptr, posY + 1);  // Return bitmap curent row

                boolean recurse = false;

                for (int i = left; i <= right; i++) {
                  if ((!bitmap_bit_get(bitmap_ptr, BITMAP_INDEX(row_index_B, i))) &&
                      PIXEL_THRESHOLD(GET_FRAME_PIXEL(row_ptr_B, i), pixelThreshold)) {

                    xylf_t context;
                    context.x = posX;
                    context.y = posY;
                    context.l = left;
                    context.r = right;
                    if (DEBUG_BLOB) Serial.printf(F("\n>>>>>>>> B-Lifo add"));
                    lifo_enqueue(&lifo, &context);
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
              if (DEBUG_BLOB) Serial.printf(F("\n>>>>>>>> Break 2"));
            }
            if (!lifo_size(&lifo)) {
              if (DEBUG_BLOB) Serial.printf(F("\n>>>>>>>> Lifo umpty: %d"), lifo_size(&lifo));
              break_out = true;
              break;
            }

            xylf_t context;
            if (DEBUG_BLOB) Serial.printf(F("\n>>>>>>>> Lifo take pixel from the queue"));
            lifo_dequeue(&lifo, &context);
            posX = context.x;
            posY = context.y;
            left = context.l;
            right = context.r;
          }

          if (break_out) {
            break;
          }
        }
        if (DEBUG_BLOB) Serial.print(F("\n>>>>>>>> END of scanline flood fill algorithm"));

        if ((blob_pixels >= minBlobPix) && (blob_pixels <= maxBlobPix)) {

          uint16_t cx = (uint16_t) blob_cx / blob_pixels; // x centroid position
          uint16_t cy = (uint16_t) blob_cy / blob_pixels; // y centroid position
          row_ptr = FRAME_ROW_PTR(input_ptr, cy);
          uint16_t cz = GET_FRAME_PIXEL(row_ptr, cx);

          blob_t* blob = list_pop_front(freeBlobList_ptr);

          blob->pixels = blob_pixels;
          blob->centroid.x = cx;
          blob->centroid.y = cy;
          blob->centroid.z = cz; // - THRESHOLD
          blob->isDead = false;
          blob->UID = blob_index;

          list_push_back(blobs_ptr, blob);
          blob_index++;
        }

        posX = old_x;
        posY = old_y;
      }
    }
  }

  Serial.printf(F("\n>>>>>>>> freeBlobs LIST : %d"), freeBlobList_ptr->index);
  Serial.printf(F("\n>>>>>>>> blobs LIST : %d"), blobs_ptr->index);

  ///////////////////////////////////////////////////////////////////////////////////////// Percistant blobs/nodes
  if (PERCISTANT) {
    // Look for the nearest blob between curent blob position (blobs_ptr) and last blob position (outputBlobs_ptr)
    uint8_t indexA = 0;
    for (blob_t* blobA = iterator_start_from_head(blobs_ptr); blobA != NULL; blobA = iterator_next(blobA)) {
      float minDist = 1000.0;
      int nearestBlob = -1;

      uint8_t indexB = 0;
      for (blob_t* blobB = iterator_start_from_head(outputBlobs_ptr); blobB != NULL; blobB = iterator_next(blobB)) {
        int16_t xa = blobA->centroid.x;
        int16_t ya = blobA->centroid.y;
        int16_t xb = blobB->centroid.x;
        int16_t yb = blobB->centroid.y;
        float dist = sqrt(pow(xa - xb, 2) + pow(ya - yb, 2)); // fast_sqrt? & fast_pow?

        if (dist < minDist) {
          minDist = dist;
          nearestBlob = indexB;
        }
        indexB++;
      }
      if (DEBUG_BLOB) Serial.printf(F("\n>>>>>>>> BLOB / found the nearest blob: %d"), nearestBlob);

      // If the distance betwin curent blob and last blob position is less than minDist:
      // We save the curentNode from blobs_ptr to nodesToUpdate &
      // We save the old node from outputNodes to oldNodesToUpdate
      if (minDist < 10.0) {
        if (DEBUG_BLOB) Serial.printf(F("\n>>>>>>>> BLOB / blob is valid save it to a list: %d"), minDist);

        list_push_back(blobsToUpdate_ptr, list_get_blob(blobs_ptr, indexA));

        blob_t *newBlob = list_pop_front(freeBlobList_ptr);
        list_copy_blob(newBlob, list_read_blob(outputBlobs_ptr, nearestBlob), sizeof(blob_t));
        list_push_back(oldBlobsToUpdate_ptr, newBlob);

      } else {
        // Found a new blob! we nead to geave it an ID.
        // We look for the minimum unused ID through the outputNodes linked list
        // and add the new blob to the nodesToAdd linked list

        bool reachEnd = false;
        int minID = 0;

        while (!reachEnd) {
          reachEnd = true;
          for (blob_t* blob = iterator_start_from_head(outputBlobs_ptr); blob != NULL; blob = iterator_next(blob)) {
            if (blob->UID == minID) {
              minID ++;
              reachEnd = false;
              break;
            }
          }
        }
        blobA->UID = minID;
        list_push_back(blobsToAdd_ptr, blobA);
      }
      indexA++;
    }

    // Update outputBlobs linked list with blobsToUpdate linked list
    for (blob_t* blobA = iterator_start_from_head(outputBlobs_ptr); blobA != NULL; blobA = iterator_next(blobA)) {
      bool found = false;
      int i = 0;
      for (blob_t* blobB = iterator_start_from_head(blobsToUpdate_ptr); blobB != NULL; blobB = iterator_next(blobB)) {

        blob_t* oldBlob = list_read_blob(oldBlobsToUpdate_ptr, i);

        if (oldBlob->UID == blobB->UID) {
          found = true;
          blobA->centroid.x = blobB->centroid.x;
          blobA->centroid.y = blobB->centroid.y;
          blobA->centroid.z = blobB->centroid.z;
          blobA->pixels = blobB->pixels;
        }
        i++;
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

    // Suppress all dead blobs from the outputNodes linked list
    bool deadExists = true;

    while (deadExists) {
      int index = 0;
      int deadBlob = -1;

      for (blob_t* blob = iterator_start_from_head(outputBlobs_ptr); blob != NULL; blob = iterator_next(blob)) {

        if (blob->isDead) {
          deadBlob = index;
          break;
        }
        index++;
      }
      if (deadBlob != -1) {
        blob_t* blob = list_get_blob(outputBlobs_ptr, deadBlob); // Supress the blob from the outputNodes linked list
        list_push_back(freeBlobList_ptr, blob); // Save this node to the freeNodeList linked list
        Serial.printf(F("\n>>>> Remove blob: %d"), deadBlob);
      } else {
        deadExists = false;
      }
    }

    for (blob_t* blob = iterator_start_from_head(blobsToAdd_ptr); blob != NULL; blob = iterator_next(blob)) {
      list_push_back(outputBlobs_ptr, blob);
    }

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

    //list_save_blobs(freeBlobList_ptr, oldBlobsToUpdate_ptr); // Remove & save node from the oldBlobsToUpdate_ptr linked list
    // list_clear_blobs(oldBlobsToUpdate_ptr);
    //list_save_blobs(freeBlobList_ptr, blobsToUpdate_ptr);    // Remove & save node from the blobsToUpdate_ptr linked list
    // list_clear_blobs(blobsToUpdate_ptr);
    //list_save_blobs(freeBlobList_ptr, blobsToAdd_ptr);       // Remove & save node from the blobsToAdd_ptr linked list
    // list_clear_blobs(blobsToAdd_ptr);

  }

  lifo_free(&lifo);
  if (DEBUG_BITMAP) bitmap_print(bitmap_ptr);
  bitmap_clear(bitmap_ptr); // TODO: optimizing!
  if (DEBUG_BLOB) Serial.printf(F("\n>>>>>>>> Cleared bitmap"));
  // list_save_blobs(freeBlobList_ptr, blobs_ptr); // Save & remove blobs/nodes from the blobs_ptr linked list
  if (DEBUG_BLOB) Serial.printf(F("\n>>>>>>>> END OFF BLOB FONCTION"));

}
