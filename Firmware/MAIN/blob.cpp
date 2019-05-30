/*
  FORKED FROM https://github.com/openmv/openmv/tree/master/src/omv/img
  Added custom blob détection algorithm to keep track of the blobs ID's
    This patch is part of the eTextile-matrix-sensor project - http://matrix.eTextile.org
    Copyright (c) 2014-2019 Maurin Donneaud <maurin@etextile.org>
    This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "blob.h"

#include <stdint.h>
void find_blobs(
  image_t*              inFrame_ptr,
  char*                 bitmap_ptr,
  llist_t*              freeBlobs_ptr,
  llist_t*              inputBlobs_ptr,
  llist_t*              outputBlobs_ptr
) {

  /////////////////////////////// Scanline flood fill algorithm
  /////////////////////////////// Connected-component labeling / CCL

  //if (DEBUG_CCL) Serial.println("\n DEBUG_CCL / START <<<<<<<<<<<<<<<<<<<<<<<<");

  bitmap_clear(bitmap_ptr, inFrame_ptr->numCols * inFrame_ptr->numRows);

  //if (DEBUG_CCL) Serial.println("DEBUG_CCL / Bitmap cleared");

  llist_raz(inputBlobs_ptr);

  for (uint8_t posY = 0; posY < inFrame_ptr->numRows; posY++) {
    
    uint8_t* row_ptr_A = ROW_PTR(inFrame_ptr, posY);     // Return inFrame_ptr curent row pointer
    uint16_t row_index_A = ROW_INDEX(inFrame_ptr, posY); // Return inFrame_ptr curent row index (1D array) 0, 64, 128,... 4032

    for (uint8_t posX = 0; posX < inFrame_ptr->numCols; posX++) {
      //if (DEBUG_BITMAP) Serial.printf("%d ", bitmap_bit_get(bitmap_ptr, BITMAP_INDEX(row_index_A, posX)));
      //if (DEBUG_BITMAP) Serial.printf("%d ", GET_PIXEL(row_ptr_A, posX));

      if (!IMAGE_GET_BINARY_PIXEL_FAST(bitmap_ptr, BITMAP_INDEX(row_index_A, posX)) && PIXEL_THRESHOLD(GET_PIXEL(row_ptr_A, posX), E256_threshold)) {
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
                 (!IMAGE_GET_BINARY_PIXEL_FAST(bitmap_ptr, BITMAP_INDEX(row_index, left - 1))) &&
                 PIXEL_THRESHOLD(GET_PIXEL(row_ptr, left - 1), E256_threshold)) {
            left--;
          }
          //if (DEBUG_CCL) Serial.printf("\n DEBUG_CCL / The minimum activated left pixel ID is: %d", left);

          while (right < (inFrame_ptr->numCols - 1) &&
                 (!IMAGE_GET_BINARY_PIXEL_FAST(bitmap_ptr, BITMAP_INDEX(row_index, right + 1))) &&
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
            IMAGE_SET_BINARY_PIXEL_FAST(bitmap_ptr, BITMAP_INDEX(row_index, x));
            blob->box.D = MAX(blob->box.D, GET_PIXEL(row_ptr, x));
            blob->pixels++; // uint16_t
          }

          boolean break_out = true;
          if (posY < (inFrame_ptr->numRows - 1)) {
            row_ptr = ROW_PTR(inFrame_ptr, posY + 1);     // Return inFrame_ptr curent row pointer
            row_index = ROW_INDEX(inFrame_ptr, posY + 1); // Return inFrame_ptr curent row index (1D array) 0, 64, 128,... 4032
            for (uint8_t x = left; x <= right; x++) {
              if ((!IMAGE_GET_BINARY_PIXEL_FAST(bitmap_ptr, BITMAP_INDEX(row_index, x))) && PIXEL_THRESHOLD(GET_PIXEL(row_ptr, x), E256_threshold)) {
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

        if (blob->pixels > MIN_BLOB_PIX && blob->pixels < MAX_BLOB_PIX) {

          blob->box.W = blob_x2 - blob_x1;
          blob->box.H = blob_y2 - blob_y1;

          blob->centroid.X = (uint8_t)round(blob_x2 - ((blob_x2 - blob_x1) / 2)); // x centroid position
          blob->centroid.Y = (uint8_t)round(blob_y2 - ((blob_y1 - blob_y2) / 2)); // y centroid position
          
          //uint8_t* row_ptr = ROW_PTR(inFrame_ptr, blob->centroid.Y);
          //blob->box.D = GET_PIXEL(row_ptr, blob->centroid.X);

          //if (DEBUG_CENTER) Serial.printf("\n DEBUG_CENTER / blob_cx: %d\tblob_cy: %d\tblob_cz: %d", blob->centroid.X, blob->centroid.Y, blob->centroid.Z);

          llist_push_back(inputBlobs_ptr, blob);
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

  //if (DEBUG_CCL) Serial.printf("\n DEBUG_CCL / **blobs** linked list index: %d", inputBlobs_ptr->index);
  //if (DEBUG_CCL) Serial.printf("\n DEBUG_CCL / **freeBlobs** linked list index: %d", freeBlobs_ptr->index);
  //if (DEBUG_CCL) Serial.println("DEBUG_CCL / END of scanline flood fill algorithm <<<<<<<<<<<<<<<<<<<<<<<<");

  /////////////////////////////// PERSISTANT BLOB ID

#ifdef DEBUG_BLOB_ID
  Serial.printf("\n DEBUG_BLOB_ID / **inputBlobs** linked list index: %d", inputBlobs_ptr->index);
  Serial.printf("\n DEBUG_BLOB_ID / **freeBlobs** linked list index: %d", freeBlobs_ptr->index);
  Serial.printf("\n DEBUG_BLOB_ID / **outputBlobs** linked list index: %d", outputBlobs_ptr->index);
#endif /*__DEBUG_BLOB_ID__*/

  // Suppress blobs from the outputBlobs linked list
  while (1) {
    boolean found = false;
    for (blob_t* blob = ITERATOR_START_FROM_HEAD(outputBlobs_ptr); blob != NULL; blob = ITERATOR_NEXT(blob)) {
      if (blob->state == TO_REMOVE) {
        found = true;
        llist_remove_blob(outputBlobs_ptr, blob);
        llist_push_back(freeBlobs_ptr, blob);
#ifdef DEBUG_BLOB_ID
        Serial.printf("\n DEBUG_BLOB_ID / Blob: %p removed from **outputBlobs** linked list", blob);
        Serial.printf("\n DEBUG_BLOB_ID / Blob: %p saved to **freeBlobList** linked list", blob);
#endif /*__DEBUG_BLOB_ID__*/
        break;
      }
    }
    if (!found) {
      break;
    }
  }

  // Look for the nearest blob between curent blob position (inputBlobs linked list) and last blob position (outputBlobs linked list)
  for (blob_t* blobA = ITERATOR_START_FROM_HEAD(inputBlobs_ptr); blobA != NULL; blobA = ITERATOR_NEXT(blobA)) {
    float minDist = 255.0f;
    blob_t* nearestBlob = NULL;

#ifdef DEBUG_BLOB_ID
    Serial.printf("\n DEBUG_BLOB_ID / Is input blob: %p have a coresponding blob in **outputBlobs**", blobA);
#endif /*__DEBUG_BLOB_ID__*/
    for (blob_t* blobB = ITERATOR_START_FROM_HEAD(outputBlobs_ptr); blobB != NULL; blobB = ITERATOR_NEXT(blobB)) {
      uint8_t xa = blobA->centroid.X;
      uint8_t ya = blobA->centroid.Y;
      uint8_t xb = blobB->centroid.X;
      uint8_t yb = blobB->centroid.Y;
      float dist = sqrt(pow(xa - xb, 2) + pow(ya - yb, 2)); // fast_sqrt? & fast_pow? // arm_sqrt_f32(); ?
#ifdef DEBUG_BLOB_ID
      Serial.printf("\n DEBUG_BLOB_ID / Distance between input & output blobs positions: %f ", dist);
#endif /*__DEBUG_BLOB_ID__*/
      if (dist < minDist) {
        minDist = dist;
        nearestBlob = blobB;
      }
    }
    // If the distance between curent blob and last blob position is less than minDist:
    // Copy the ID of the nearestBlob found in outputBlobs linked list and give it to the curent input blob.
    // Set the curent input blob state to TO_UPDATE.
    if (minDist < 10.0f) { // TODO: set it as global variable
#ifdef DEBUG_BLOB_ID
      Serial.printf("\n DEBUG_BLOB_ID / Found corresponding blob: %p in the **outputBlobs** linked list", nearestBlob);
#endif /*__DEBUG_BLOB_ID__*/
      blobA->UID = nearestBlob->UID;
      blobA->state = TO_UPDATE;
    }
    // Found a new blob! We nead to give it an ID
    else {
#ifdef DEBUG_BLOB_ID
      Serial.print("\n DEBUG_BLOB_ID / Found new blob without ID");
#endif /*__DEBUG_BLOB_ID__*/

      // Test if there is more than two blobs in the linked list (-1 is empty)
      if (outputBlobs_ptr->index > 0) {
        // Sorting outputBlobs linked list
        llist_sort(outputBlobs_ptr);
      }

      // Find the smallest missing UID in the sorted linked list
      uint8_t minID = 0;
      while (1) {
        boolean isFree = true;
        for (blob_t* blob = ITERATOR_START_FROM_HEAD(outputBlobs_ptr); blob != NULL; blob = ITERATOR_NEXT(blob)) {
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
  } //

  // Update outputBlobs linked list with new inputs blobs that have ben flaged TO_UPDATE.
  // If a blob in the outputBlobs linked do not have corresponding blob in the input blobs linked list (inputBlobs_ptr), flag it to TO_REMOVE.
  for (blob_t* blobA = ITERATOR_START_FROM_HEAD(outputBlobs_ptr); blobA != NULL; blobA = ITERATOR_NEXT(blobA)) {
    boolean found = false;

    for (blob_t* blobB = ITERATOR_START_FROM_HEAD(inputBlobs_ptr); blobB != NULL; blobB = ITERATOR_NEXT(blobB)) {
      if (blobB->state == TO_UPDATE && blobB->UID == blobA->UID) {
        found = true;
        blob_copy(blobA, blobB);
        blobA->alive = 1;
        blobB->state = FREE;
#ifdef DEBUG_BLOB_ID
        Serial.printf("\n DEBUG_BLOB_ID / Copy blob: %p (inputBlobs linked list) to the blob: %p (outputBlobs linked list)", blobB, blobA);
#endif /*__DEBUG_BLOB_ID__*/
        break;
      }
    }
    if (!found) {
      blobA->alive = 0;
      blobA->state = TO_REMOVE;
    }
  }

  // Add the new blobs to the outputBlobs linked list
  for (blob_t* blob = ITERATOR_START_FROM_HEAD(inputBlobs_ptr); blob != NULL; blob = ITERATOR_NEXT(blob)) {
    if (blob->state == TO_ADD) {
      blob_t* newBlob = llist_pop_front(freeBlobs_ptr);
      blob_copy(newBlob, blob);
      newBlob->alive = 1;
      llist_push_back(outputBlobs_ptr, newBlob);
#ifdef DEBUG_BLOB_ID
      Serial.printf("\n DEBUG_BLOB_ID / Blob: %p added to **outputBlobs** linked list", blob);
#endif /*__DEBUG_BLOB_ID__*/
    }
  }

  llist_save_blobs(freeBlobs_ptr, inputBlobs_ptr);
#ifdef DEBUG_BLOB_ID
  Serial.println("\n DEBUG_BLOB_ID / END OFF BLOB FONCTION");
#endif /*__DEBUG_BLOB_ID__*/
}


void bitmap_clear(char* bitmap_ptr, const uint16_t Size) {
  memset(bitmap_ptr, 0, Size * sizeof(char)); // FIXME: can be optimized
}

inline void blob_copy(blob_t* dst, blob_t* src) {
  //dst->timeTag = millis(); // TODO?
  dst->UID = src->UID;
  dst->alive = src->alive;
  dst->centroid.X = src->centroid.X;
  dst->centroid.Y = src->centroid.Y;
  dst->box.W = src->box.W;
  dst->box.H = src->box.H;
  dst->box.D = src->box.D;
  dst->pixels = src->pixels;
}

inline void blob_raz(blob_t* node) {
  //node->timeTag = 0; // TODO?
  node->UID = 0;
  node->alive = 0;
  node->state = FREE;
  node->centroid.X = 0;
  node->centroid.Y = 0;
  node->box.W = 0;
  node->box.H = 0;
  node->box.D = 0;
  node->pixels = 0;
}
