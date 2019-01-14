/*
  This file is part of the eTextile-matrix-sensor project - http://matrix.eTextile.org
  Copyright (c) 2014-2018 Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "blob.h"

void find_blobs(
  image_t*              inFrame_ptr,
  char*                 bitmap_ptr,
  const int             rows,
  const int             cols,
  uint8_t               E256_threshold,
  const unsigned int    minBlobPix,
  const unsigned int    maxBlobPix,
  llist_t*              freeBlobs_ptr,
  llist_t*              blob_ptr,
  llist_t*              outputBlobs_ptr,
  OSCBundle*            bundleOut_ptr
) {

  /////////////////////////////// Scanline flood fill algorithm
  /////////////////////////////// Connected-component labeling / CCL
  ///////////////////////////////

  //if (DEBUG_CCL) Serial.println("\n DEBUG_CCL / START <<<<<<<<<<<<<<<<<<<<<<<<");

  bitmap_clear(bitmap_ptr, NEW_FRAME);
  //if (DEBUG_CCL) Serial.println("DEBUG_CCL / Bitmap cleared");

  llist_raz(blob_ptr);

  for (uint8_t posY = 0; posY < rows; posY++) {
    uint8_t* row_ptr_A = ROW_PTR(inFrame_ptr, posY);     // Return inFrame_ptr curent row pointer
    uint16_t row_index_A = ROW_INDEX(inFrame_ptr, posY); // Return inFrame_ptr curent row index (1D array) 0, 64, 128,... 4032
    for (uint8_t posX = 0; posX < cols; posX++) {
      //if (DEBUG_BITMAP) Serial.printf("%d ", bitmap_bit_get(bitmap_ptr, BITMAP_INDEX(row_index_A, posX)));
      //if (DEBUG_BITMAP) Serial.printf("%d ", GET_PIXEL(row_ptr_A, posX));

      if (!bitmap_bit_get(bitmap_ptr, BITMAP_INDEX(row_index_A, posX)) && PIXEL_THRESHOLD(GET_PIXEL(row_ptr_A, posX), E256_threshold)) {
        //if (DEBUG_CCL) Serial.printf("\n DEBUG_CCL / Found active pixel in row:%d\tcolumn:%d", posY, posX);

        uint8_t oldX = posX;
        uint8_t oldY = posY;

        blob_t* blob = llist_pop_front(freeBlobs_ptr);
        //if (DEBUG_CCL) Serial.printf("\n DEBUG_CCL / Get a blob from **freeBlobs** linked list: %p", blob);
        blob_raz(blob);

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
                 PIXEL_THRESHOLD(GET_PIXEL(row_ptr, left - 1), E256_threshold)) {
            left--;
          }
          //if (DEBUG_CCL) Serial.printf("\n DEBUG_CCL / The minimum activated left pixel ID is: %d", left);

          while (right < (cols - 1) &&
                 (!bitmap_bit_get(bitmap_ptr, BITMAP_INDEX(row_index, right + 1))) &&
                 PIXEL_THRESHOLD(GET_PIXEL(row_ptr, right + 1), E256_threshold)) {
            right++;
          }
          //if (DEBUG_CCL) Serial.printf("\n DEBUG_CCL / The maximum activated right pixel ID is: %d", right);

          blob_x1 = MIN(blob_x1, left);
          blob_y1 = MIN(blob_y1, posY);
          blob_x2 = MAX(blob_x2, right);
          blob_y2 = MAX(blob_y2, posY);

          //if (DEBUG_CCL) Serial.println("DEBUG_CCL / Save this activated pixels line to the bitmap array");
          for (uint8_t x = left; x <= right; x++) {
            bitmap_bit_set(bitmap_ptr, BITMAP_INDEX(row_index, x));
            blob->box.D = MAX(blob->box.D, GET_PIXEL(row_ptr, x));
            blob->pixels++;
          }

          boolean break_out = true;
          if (posY < (rows - 1)) {
            row_ptr = ROW_PTR(inFrame_ptr, posY + 1);     // Return inFrame_ptr curent row pointer
            row_index = ROW_INDEX(inFrame_ptr, posY + 1); // Return inFrame_ptr curent row index (1D array) 0, 64, 128,... 4032
            for (uint8_t x = left; x <= right; x++) {
              if ((!bitmap_bit_get(bitmap_ptr, BITMAP_INDEX(row_index, x))) && PIXEL_THRESHOLD(GET_PIXEL(row_ptr, x), E256_threshold)) {
                //if (DEBUG_CCL) Serial.printf("\n DEBUG_CCL / Found an active pixel in row: %d", posY + 1);
                posX = x;
                posY++;
                break_out = false;
                break;
              }
            }
          }
          if (break_out) {
            break;
          }
        }
        //if (DEBUG_CCL) Serial.println("DEBUG_CCL / BLOB COMPLEAT!");

        if ((blob->pixels > minBlobPix) && (blob->pixels < maxBlobPix)) {
          blob->box.W = blob_x2 - blob_x1;
          blob->box.H = blob_y2 - blob_y1;

          blob->centroid.X = (uint8_t) (blob_x2 - ((blob_x2 - blob_x1) / 2)); // x centroid position
          blob->centroid.Y = (uint8_t) (blob_y2 - ((blob_y1 - blob_y2) / 2)); // y centroid position
          // uint8_t* row_ptr = ROW_PTR(inFrame_ptr, blob->centroid.Y); // DO NOT WORK!?
          // blob->box.Z = GET_PIXEL(row_ptr, blob->centroid.X);   // DO NOT WORK!?
          //if (DEBUG_CENTER) Serial.printf("\n DEBUG_CENTER / blob_cx: %d\tblob_cy: %d\tblob_cz: %d", blob->centroid.X, blob->centroid.Y, blob->centroid.Z);

          llist_push_back(blob_ptr, blob);
          //if (DEBUG_CCL) Serial.printf("\n DEBUG_CCL / Blob: %p added to the **blobs** linked list", blob);
        } else {
          llist_push_back(freeBlobs_ptr, blob);
          //if (DEBUG_CCL) Serial.printf("\n DEBUG_CCL / Blob %p saved to **freeBlobList** linked list", blob);
        }
        posX = oldX;
        posY = oldY;
      }
    }
    //if (DEBUG_BITMAP) Serial.println();
  }
  //if (DEBUG_BITMAP) Serial.println();

  //if (DEBUG_CCL) Serial.printf("\n DEBUG_CCL / **blobs** linked list index: %d", blob_ptr->index);
  //if (DEBUG_CCL) Serial.printf("\n DEBUG_CCL / **freeBlobs** linked list index: %d", freeBlobs_ptr->index);
  //if (DEBUG_CCL) Serial.println("DEBUG_CCL / END of scanline flood fill algorithm <<<<<<<<<<<<<<<<<<<<<<<<");

  ///////////////////////////////
  /////////////////////////////// PERSISTANT BLOB ID
  ///////////////////////////////

  //if (DEBUG_BLOBS) Serial.printf("\n DEBUG_BLOBS / **blobs** linked list index: %d", blob_ptr->index);
  //if (DEBUG_BLOBS) Serial.printf("\n DEBUG_BLOBS / **freeBlobs** linked list index: %d", freeBlobs_ptr->index);
  //if (DEBUG_BLOBS) Serial.printf("\n DEBUG_BLOBS / **outputBlobs** linked list index: %d", outputBlobs_ptr->index);

  // Look for the nearest blob between curent blob position (blobs linked list) and last blob position (outputBlobs linked list)
  for (blob_t* blobA = iterator_start_from_head(blob_ptr); blobA != NULL; blobA = iterator_next(blobA)) {
    float minDist = 255.0f;
    blob_t* nearestBlob = NULL;
    //if (DEBUG_BLOBS) Serial.printf("\n DEBUG_BLOBS / Is input blob: %p have a coresponding blob in **outputBlobs**", blobA);

    for (blob_t* blobB = iterator_start_from_head(outputBlobs_ptr); blobB != NULL; blobB = iterator_next(blobB)) {
      uint8_t xa = blobA->centroid.X;
      uint8_t ya = blobA->centroid.Y;
      uint8_t xb = blobB->centroid.X;
      uint8_t yb = blobB->centroid.Y;
      float dist = sqrt(pow(xa - xb, 2) + pow(ya - yb, 2)); // fast_sqrt? & fast_pow? // arm_sqrt_f32(); ?
      //if (DEBUG_BLOBS) Serial.printf("\n DEBUG_BLOBS / Distance between input & output blobs positions: %f ", dist);

      if (dist < minDist) {
        minDist = dist;
        nearestBlob = blobB;
      }
    }
    // If the distance between curent blob and last blob position is less than minDist:
    // Take the ID of the nearestBlob in outputBlobs linked list and give it to the curent input blob.
    // Move the curent blob to the blobsToUpdate linked list.
    if (minDist < 10) {
      //if (DEBUG_BLOBS) Serial.printf("\n DEBUG_BLOBS / Found corresponding blob: %p in the **outputBlobs** linked list", nearestBlob);
      blobA->UID = nearestBlob->UID;
      blobA->state = UPDATE;
    } else {
      // Found a new blob! we nead to give it an ID.
      // We look for the minimum unused ID through the outputBlobs linked list &
      // We add the new blob to the nodesToAdd linked list.
      //if (DEBUG_BLOBS) Serial.print("\n DEBUG_BLOBS / Found new blob without ID");
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
          blobA->state = TOADD;
          break;
        }
      }
    }
  }
  //if (DEBUG_BLOBS) Serial.printf(F("\n DEBUG_BLOBS / EXIT <<<<<<<<<<<<<<<<<"));

  // Update outputBlobs linked list with blobsToUpdate linked list.
  // If a blob in the outputBlobs linked list is not in the blobsToUpdate linked list, set it to dead.
  for (blob_t* blobA = iterator_start_from_head(outputBlobs_ptr); blobA != NULL; blobA = iterator_next(blobA)) {
    boolean found = false;
    for (blob_t* blobB = iterator_start_from_head(blob_ptr); blobB != NULL; blobB = iterator_next(blobB)) {
      if (blobB->UID == blobA->UID && blobB->state == UPDATE) {
        found = true;
        blob_copy(blobA, blobB);
        blobB->state = FREE;
        // Add the blob values to the OSC bundle
        OSCMessage OSCmsg("/blob");
        OSCmsg.add(blobA->UID).add(blobA->centroid.X).add(blobA->centroid.Y).add(blobA->box.W).add(blobA->box.H).add(blobA->box.D);
        bundleOut_ptr->add(OSCmsg);
#ifdef DEBUG_BLOBS_OSC
        Serial.printf("\n DEBUG_BLOBS_OSC / Update outputBlobs / UID:%d\tX:%d\tY:%d\tW:%d\tH:%d\tD:%d",
                      blobA->UID,
                      blobA->centroid.X,
                      blobA->centroid.Y,
                      blobA->box.W,
                      blobA->box.H,
                      blobA->box.D
                     );
#endif /*__DEBUG_BLOBS_OSC__*/
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
        //if (DEBUG_BLOBS) Serial.printf("\n DEBUG_BLOBS / Blob: %p removed from **outputBlobs** linked list", blob);
        llist_push_back(freeBlobs_ptr, blob);
        //if (DEBUG_BLOBS) Serial.printf("\n DEBUG_BLOBS / Blob: %p saved to **freeBlobList** linked list", blob);
        // Add the blob values to the OSC bundle
        OSCMessage OSCmsg("/blob");
        OSCmsg.add(blob->UID).add(-1).add(-1).add(-1).add(-1).add(-1);
        bundleOut_ptr->add(OSCmsg);
#ifdef DEBUG_BLOBS_OSC
        Serial.printf("\n DEBUG_BLOBS_OSC / Suppress dead blob / UID:%d\tX:%d\tY:%d\tW:%d\tH:%d\tD:%d", blob->UID, -1, -1, -1, -1, -1);
#endif /*__DEBUG_BLOBS_OSC__*/
        break;
      }
    }
    if (!found) {
      break;
    }
  }

  // Add the new blobs to the outputBlobs linked list
  for (blob_t* blob = iterator_start_from_head(blob_ptr); blob != NULL; blob = iterator_next(blob)) {
    if (blob->state == TOADD) {
      blob_t* newBlob = llist_pop_front(freeBlobs_ptr);
      blob_copy(newBlob, blob);
      llist_push_back(outputBlobs_ptr, newBlob);
      //if (DEBUG_BLOBS) Serial.printf("\n DEBUG_BLOBS / Blob: %p added to **outputBlobs** linked list", blob);
      // Add the blob values to the OSC bundle
      OSCMessage OSCmsg("/blob");
      OSCmsg.add(blob->UID).add(blob->centroid.X).add(blob->centroid.Y).add(blob->box.W).add(blob->box.H).add(blob->box.D);
      bundleOut_ptr->add(OSCmsg);
#ifdef DEBUG_BLOBS_OSC
      Serial.printf("\n DEBUG_BLOBS_OSC / Add the new blob / UID:%d\tX:%d\tY:%d\tW:%d\tH:%d\tD:%d",
                    blob->UID,
                    blob->centroid.X,
                    blob->centroid.Y,
                    blob->box.W,
                    blob->box.H,
                    blob->box.D
                   );
#endif /*__DEBUG_BLOBS_OSC__*/
    }
  }

  llist_save_blobs(freeBlobs_ptr, blob_ptr);
  //if (DEBUG_BLOBS) Serial.println("\n DEBUG_BLOBS / END OFF BLOB FONCTION");
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
  dst->UID = src->UID;
  dst->centroid.X = src->centroid.X;
  dst->centroid.Y = src->centroid.Y;
  dst->box.W = src->box.W;
  dst->box.H = src->box.H;
  dst->box.D = src->box.D;
  dst->pixels = src->pixels;
}

inline void blob_raz(blob_t* node) {
  node->UID = -1;
  node->state = FREE;
  node->centroid.X = 0;
  node->centroid.Y = 0;
  node->box.W = 0;
  node->box.H = 0;
  node->box.D = 0;
  node->pixels = 0;
}
