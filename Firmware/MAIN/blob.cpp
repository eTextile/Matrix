/*
  FORKED FROM https://github.com/openmv/openmv/tree/master/src/omv/img
    - This file is part of the OpenMV project.
    - Copyright (c) 2013-2019 Ibrahim Abdelkader <iabdalkader@openmv.io> & Kwabena W. Agyeman <kwagyeman@openmv.io>
    - This work is licensed under the MIT license, see the file LICENSE for details.

  Added custom blob détection algorithm to keep track of the blobs ID's
    - This patch is part of the eTextile-matrix-sensor project - http://matrix.eTextile.org
    - Copyright (c) 2014-2019 Maurin Donneaud <maurin@etextile.org>
    - This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "blob.h"

void BLOB_SETUP(
  image_t* inputFrame_ptr,
  image_t* bitmap_ptr,
  uint8_t* bitmapArray_ptr,
  lifo_t*  lifo_ptr,
  lifo_t*  lifo_stack_ptr,
  xylr_t*  lifoArray_ptr,
  llist_t* blobs_ptr,
  llist_t* blobs_stack_ptr,
  blob_t*  blobArray_ptr,
  llist_t* outputBlobs_ptr
) {

  // bitmap init config (struct image_t)
  bitmap_ptr->numCols = NEW_COLS;           //
  bitmap_ptr->numRows = NEW_ROWS;           //
  bitmap_ptr->pData = &bitmapArray_ptr[0];  // uint8_t bitmapArray[RAW_FRAME];(16x16) array containing (64x64) values

  // Lifo init
  lifo_raz(lifo_stack_ptr);
  lifo_init(lifo_stack_ptr, &lifoArray_ptr[0], (uint8_t)LIFO_MAX_NODES); // Add X nodes in the lifo_stack
  lifo_raz(lifo_ptr);

  // Linked list init
  llist_raz(blobs_stack_ptr);
  llist_init(blobs_stack_ptr, &blobArray_ptr[0], (uint8_t)MAX_NODES); // Add X nodes in the blobs_stack linked list
  llist_raz(blobs_ptr);
  llist_raz(outputBlobs_ptr);
}

static int sum_m_to_n(int m, int n) {
  return ((n * (n + 1)) - (m * (m - 1))) / 2;
}

void find_blobs(
  uint8_t   Threshold,
  image_t*  inputFrame_ptr,
  image_t*  bitmap_ptr,
  lifo_t*   lifo_stack_ptr,
  lifo_t*   lifo_ptr,
  llist_t*  blobs_stack_ptr,
  llist_t*  inputBlobs_ptr,
  llist_t*  outputBlobs_ptr
) {

  /////////////////////////////// Scanline flood fill algorithm / SFF
  /////////////////////////////// Connected-component labeling / CCL
  bitmap_clear(bitmap_ptr);
  //llist_raz(inputBlobs_ptr);

  for (uint8_t posY = 0, yy = inputFrame_ptr->numRows, y_max = yy - 1; posY < yy; posY += Y_STRIDE) {

    uint8_t* row_ptr = COMPUTE_IMAGE_ROW_PTR (inputFrame_ptr, posY);
    uint8_t* bmp_row_ptr = COMPUTE_BINARY_IMAGE_ROW_PTR (bitmap_ptr, posY);

    for (uint8_t posX = (posY % X_STRIDE), xx = inputFrame_ptr->numCols, x_max = xx - 1; posX < xx; posX += X_STRIDE) {
      if (!IMAGE_GET_BINARY_PIXEL_FAST(bmp_row_ptr, posX)
          && PIXEL_THRESHOLD(IMAGE_GET_PIXEL_FAST(row_ptr, posX), Threshold)) {

        uint8_t oldX = posX;
        uint8_t oldY = posY;

        uint8_t blob_x1 = posX;
        uint8_t blob_x2 = posX;

        uint8_t blob_height = 0;
        uint8_t blob_depth = 0;

        uint16_t blob_pixels = 0;
        uint16_t blob_cx = 0;
        uint16_t blob_cy = 0;


        while (1) { // while_A
          uint8_t left = posX;
          uint8_t right = posX;

          uint8_t* row = COMPUTE_IMAGE_ROW_PTR (inputFrame_ptr, posY);
          uint8_t* bmp_row = COMPUTE_BINARY_IMAGE_ROW_PTR (bitmap_ptr, posY);

          while ((left > 0)
                 && (!IMAGE_GET_BINARY_PIXEL_FAST(bmp_row, left - 1))
                 && PIXEL_THRESHOLD(IMAGE_GET_PIXEL_FAST(row, left - 1), Threshold)) {
            left--;
          }

          while (right < (inputFrame_ptr->numCols - 1)
                 && (!IMAGE_GET_BINARY_PIXEL_FAST(bmp_row, right + 1))
                 && PIXEL_THRESHOLD(IMAGE_GET_PIXEL_FAST(row, right + 1), Threshold)) {
            right++;
          }

          blob_x1 = MIN(blob_x1, left);
          blob_x2 = MAX(blob_x2, right);

          for (uint8_t i = left; i <= right; i++) {
            IMAGE_SET_BINARY_PIXEL_FAST(bmp_row, i);
            blob_depth = MAX(blob_depth, IMAGE_GET_PIXEL_FAST(row, i));
          }

          int sum = sum_m_to_n(left, right);
          uint8_t pixels = right - left + 1;
          blob_pixels += pixels;
          blob_cx += sum;
          blob_cy += posY * pixels;

          uint8_t top_left = left;
          uint8_t bot_left = left;

          boolean break_out = false;
          while (1) { // while_B

            if (lifo_ptr->index < lifo_stack_ptr->max_nodes - 1) {

              if (posY > 0) {

                row = COMPUTE_IMAGE_ROW_PTR (inputFrame_ptr, posY - 1);
                bmp_row = COMPUTE_BINARY_IMAGE_ROW_PTR (bitmap_ptr, posY - 1);

                boolean recurse = false;
                for (uint8_t i = top_left; i <= right; i++) {

                  if ((!IMAGE_GET_BINARY_PIXEL_FAST(bmp_row, i))
                      && (PIXEL_THRESHOLD(IMAGE_GET_PIXEL_FAST(row, i), Threshold))) {

                    xylr_t* context = lifo_dequeue(lifo_stack_ptr);
                    //Serial.printf("\n DEBUG_LIFO / A / lifo_stack_ptr / Dequeue_node: %p", context);
                    context->x = posX;
                    context->y = posY;
                    context->l = left;
                    context->r = right;
                    context->t_l = i++; // Don't test the same pixel again
                    context->b_l = bot_left;
                    lifo_enqueue(lifo_ptr, context);
                    //Serial.printf("\n DEBUG_LIFO / A / lifo_ptr / Enqueue_node: %p", context);

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

              if (posY < (inputFrame_ptr->numRows - 1)) {

                row = COMPUTE_IMAGE_ROW_PTR (inputFrame_ptr, posY + 1);
                bmp_row = COMPUTE_BINARY_IMAGE_ROW_PTR (bitmap_ptr, posY + 1);

                boolean recurse = false;
                for (uint8_t i = bot_left; i <= right; i++) {

                  if (!IMAGE_GET_BINARY_PIXEL_FAST(bmp_row, i)
                      && PIXEL_THRESHOLD(IMAGE_GET_PIXEL_FAST(row, i), Threshold)) {

                    xylr_t* context = lifo_dequeue(lifo_stack_ptr);
                    //Serial.printf("\n DEBUG_LIFO / B / lifo_stack_ptr / Dequeue_node: %p", context);
                    context->x = posX;
                    context->y = posY;
                    context->l = left;
                    context->r = right;
                    context->t_l = top_left;
                    context->b_l = i++; // Don't test the same pixel again
                    lifo_enqueue(lifo_ptr, context);
                    //Serial.printf("\n DEBUG_LIFO / B / lifo_ptr / Enqueue_node: %p", context);

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
            }
            if (lifo_ptr->index < 0) {
              break_out = true;
              break;
            }

            xylr_t* context = lifo_dequeue(lifo_ptr);
            //Serial.printf("\n DEBUG_LIFO / C / lifo_ptr / Dequeue_node: %p", context);
            posX = context->x;
            posY = context->y;
            left = context->l;
            right = context->r;
            top_left = context->t_l;
            bot_left = context->b_l;
            lifo_enqueue(lifo_stack_ptr, context); // Save the node to the lifo_stack
            //Serial.printf("\n DEBUG_LIFO / C / lifo_stack_ptr / Enqueue_node: %p", context);

            blob_height++;

          } // END while_B

          if (break_out) {
            break;
          }
        } // END while_A

        if (blob_pixels > MIN_BLOB_PIX && blob_pixels < MAX_BLOB_PIX) {

          blob_t* blob = llist_pop_front(blobs_stack_ptr);
          blob_raz(blob);

          blob->centroid.X = (uint8_t)round(blob_cx / ((float) blob_pixels));
          blob->centroid.Y = (uint8_t)round(blob_cy / ((float) blob_pixels));

          blob->box.W = (blob_x2 - blob_x1);
          blob->box.H = blob_height;
          blob->box.D = blob_depth;
          /*
            Serial.printf("\n DEBUG_SFF / blob_cx:%d \tblob_cy:%d \tblob_W:%d \tblob_H:%d \tblob_D:%d",
                        blob->centroid.X,
                        blob->centroid.Y,
                        blob->box.W,
                        blob->box.H,
                        blob->box.D
                       );
          */
          llist_push_back(inputBlobs_ptr, blob);
          //Serial.printf("\n DEBUG_SFF / Blob: %p added to the **blobs** linked list", blob);
        }
        posX = oldX;
        posY = oldY;
      }
    }
  }
  /////////////////////////////// PERSISTANT BLOB ID

#ifdef DEBUG_BLOBS_ID
  Serial.printf("\n DEBUG_BLOBS_ID / **inputBlobs** linked list index: %d", inputBlobs_ptr->index);
  Serial.printf("\n DEBUG_BLOBS_ID / **blobs_stack** linked list index: %d", blobs_stack_ptr->index);
  Serial.printf("\n DEBUG_BLOBS_ID / **outputBlobs** linked list index: %d", outputBlobs_ptr->index);
#endif /*__DEBUG_BLOBS_ID__*/

  // Suppress blobs from the outputBlobs linked list
  while (1) {
    boolean found = false;
    for (blob_t* blob = ITERATOR_START_FROM_HEAD(outputBlobs_ptr); blob != NULL; blob = ITERATOR_NEXT(blob)) {
      if (blob->state == TO_REMOVE) {
        found = true;
        llist_remove_blob(outputBlobs_ptr, blob);
        llist_push_back(blobs_stack_ptr, blob);
#ifdef DEBUG_BLOBS_ID
        Serial.printf("\n DEBUG_BLOBS_ID / Blob: %p removed from **outputBlobs** linked list", blob);
        Serial.printf("\n DEBUG_BLOBS_ID / Blob: %p saved to **freeBlobList** linked list", blob);
#endif /*__DEBUG_BLOBS_ID__*/
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

#ifdef DEBUG_BLOBS_ID
    Serial.printf("\n DEBUG_BLOBS_ID / Is input blob: %p have a coresponding blob in **outputBlobs**", blobA);
#endif /*__DEBUG_BLOBS_ID__*/
    for (blob_t* blobB = ITERATOR_START_FROM_HEAD(outputBlobs_ptr); blobB != NULL; blobB = ITERATOR_NEXT(blobB)) {

      float dist = distance(blobA, blobB);

#ifdef DEBUG_BLOBS_ID
      Serial.printf("\n DEBUG_BLOBS_ID / Distance between input & output blobs positions: %f ", dist);
#endif /*__DEBUG_BLOBS_ID__*/
      if (dist < minDist) {
        minDist = dist;
        nearestBlob = blobB;
      }
    }
    // If the distance between curent blob and last blob position is less than minDist:
    // Copy the ID of the nearestBlob found in outputBlobs linked list and give it to the curent input blob.
    // Set the curent input blob state to TO_UPDATE.
    if (minDist < 5.0f) { // TODO: set it as global variable
#ifdef DEBUG_BLOBS_ID
      Serial.printf("\n DEBUG_BLOBS_ID / Found corresponding blob: %p in the **outputBlobs** linked list", nearestBlob);
#endif /*__DEBUG_BLOBS_ID__*/
      blobA->UID = nearestBlob->UID;
      blobA->state = TO_UPDATE;
    }
    // Found a new blob! We nead to give it an ID
    else {
#ifdef DEBUG_BLOBS_ID
      Serial.print("\n DEBUG_BLOBS_ID / Found new blob without ID");
#endif /*__DEBUG_BLOBS_ID__*/

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
#ifdef DEBUG_BLOBS_ID
        Serial.printf("\n DEBUG_BLOBS_ID / Copy blob: %p (inputBlobs linked list) to the blob: %p (outputBlobs linked list)", blobB, blobA);
#endif /*__DEBUG_BLOBS_ID__*/
        break;
      }
    }
    if (!found) {
      blobA->state = TO_REMOVE;
      blobA->alive = 0;
    }
  }

  // Add the new blobs to the outputBlobs linked list
  for (blob_t* blob = ITERATOR_START_FROM_HEAD(inputBlobs_ptr); blob != NULL; blob = ITERATOR_NEXT(blob)) {
    if (blob->state == TO_ADD) {
      blob_t* newBlob = llist_pop_front(blobs_stack_ptr);
      blob_copy(newBlob, blob);
      newBlob->alive = 1;
      llist_push_back(outputBlobs_ptr, newBlob);
#ifdef DEBUG_BLOBS_ID
      Serial.printf("\n DEBUG_BLOBS_ID / Blob: %p added to **outputBlobs** linked list", blob);
#endif /*__DEBUG_BLOBS_ID__*/
    }
  }

  // Clear the input blobs Linked list
  llist_save_blobs(blobs_stack_ptr, inputBlobs_ptr); // TODO : can we live without it ?

#ifdef DEBUG_BLOBS_ID
  Serial.println("\n DEBUG_BLOBS_ID / END OFF BLOB FONCTION");
#endif /*__DEBUG_BLOBS_ID__*/
}

////////////////////////////////

void bitmap_clear(image_t* bitmap_ptr) {
  memset(bitmap_ptr->pData, 0, NEW_FRAME * sizeof(uint8_t));
}

/*
  void bitmap_clear(image_t* bitmap_ptr){
    memset(bitmap_ptr->pData, 0, ((NEW_FRAME + CHAR_MASK) >> CHAR_SHIFT) * sizeof(uint8_t));
  }
*/

float distance(blob_t* blobA, blob_t* blobB) {
  float sum = 0.0f;
  sum += (blobA->centroid.X - blobB->centroid.X) * (blobA->centroid.X - blobB->centroid.X);
  sum += (blobA->centroid.Y - blobB->centroid.Y) * (blobA->centroid.Y - blobB->centroid.Y);
  return sqrtf(sum);;
}

/* DO NOT WORK!
  void blob_copy(blob_t* dst, blob_t* src) {
  memcpy(dst, src, sizeof(blob_t));
  }
*/

// FIXME: can be optimized
void blob_copy(blob_t* dst, blob_t* src) {
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

// FIXME: can be optimized
void blob_raz(blob_t* node) {
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
