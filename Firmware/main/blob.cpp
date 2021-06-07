/*
  FORKED FROM https://github.com/openmv/openmv/blob/master/src/omv/imlib/blob.c
    - This file is part of the OpenMV project.
    - Copyright (c) 2013- Ibrahim Abdelkader <iabdalkader@openmv.io> & Kwabena W. Agyeman <kwagyeman@openmv.io>
    - This work is licensed under the MIT license, see the file LICENSE for details.

  Added custom blob détection algorithm to keep track of the blobs ID's
    - This patch is part of the eTextile-Synthesizer project - http://synth.eTextile.org
    - Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
    - This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "blob.h"

#define MAX_BLOBS           32            // [1:64] Set how many blobs can be tracked at the same time
#define LIFO_NODES          512           // Set the maximum nodes number
#define X_STRIDE            3             // Speed up the scanning X
#define Y_STRIDE            3             // Speed up the scanning Y
#define MIN_BLOB_PIX        5             // Set the minimum blob pixels
#define DEBOUNCE_TIME       20            // Avioding undesired bouncing effect when taping on the sensor

#define CENTER_X            (NEW_COLS / 2)
#define CENTER_Y            (NEW_ROWS / 2)

uint8_t bitmapFrame[NEW_FRAME] = {0};     // 1D Array to store (64*64) binary values
xylr_t lifoArray[LIFO_NODES] = {0};       // 1D Array to store lifo nodes
blob_t blobArray[MAX_BLOBS] = {0};        // 1D Array to store blobs

velocity_t blobVelocity[MAX_SYNTH] = {0}; // 1D Array to store XY & Z blobs velocity
polar_t polarCoord[MAX_SYNTH] = {0};      // 1D Array of struct polar_t to store blobs polar coordinates

llist_t llist_context_stack;              // Free nodes stack
llist_t llist_context;                    // Used nodes
llist_t llist_blobs_stack;                // Free nodes stack
llist_t llist_blobs;                      // Intermediate blobs linked list

void blob_llist_init(llist_t* llist_ptr, blob_t* nodesArray_ptr) {
  llist_raz(llist_ptr);
  for (int i = 0; i < MAX_BLOBS; i++) {
    llist_push_front(llist_ptr, &nodesArray_ptr[i]);
  }
}

void lifo_llist_init(llist_t* llist_ptr, xylr_t* nodesArray_ptr) {
  llist_raz(llist_ptr);
  for (int i = 0; i < LIFO_NODES; i++) {
    llist_push_front(llist_ptr, &nodesArray_ptr[i]);
  }
}

void BLOB_SETUP(llist_t* blobs_ptr) {
  lifo_llist_init(&llist_context_stack, &lifoArray[0]); // Add X nodes to the llist_context_stack
  blob_llist_init(&llist_blobs_stack, &blobArray[0]); // Add X nodes to the llist_blobs_stack linked list
  llist_raz(&llist_context);
  llist_raz(&llist_blobs);
  llist_raz(blobs_ptr);
}

/////////////////////////////// Scanline flood fill algorithm / SFF
/////////////////////////////// Connected-component labeling / CCL
void find_blobs(uint8_t zThreshold, image_t* inputFrame_ptr, llist_t* outputBlobs_ptr) {

  memset((uint8_t*)bitmapFrame, 0, SIZEOF_FRAME);

  for (uint8_t posY = 0; posY < NEW_ROWS; posY += Y_STRIDE) {

    uint8_t* row_ptr_A = COMPUTE_IMAGE_ROW_PTR(inputFrame_ptr, posY);
    uint8_t* bmp_row_ptr_A = COMPUTE_BINARY_IMAGE_ROW_PTR(&bitmapFrame[0], posY);

    for (uint8_t posX = (posY % X_STRIDE); posX < NEW_COLS; posX += X_STRIDE) {
      if (!IMAGE_GET_BINARY_PIXEL_FAST(bmp_row_ptr_A, posX)
          && PIXEL_THRESHOLD(IMAGE_GET_PIXEL_FAST(row_ptr_A, posX), zThreshold)) {

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

          uint8_t* row_ptr_B = COMPUTE_IMAGE_ROW_PTR(inputFrame_ptr, posY);
          uint8_t* bmp_row_ptr_B = COMPUTE_BINARY_IMAGE_ROW_PTR (&bitmapFrame[0], posY);

          while ((left > 0)
                 && (!IMAGE_GET_BINARY_PIXEL_FAST(bmp_row_ptr_B, left - 1))
                 && PIXEL_THRESHOLD(IMAGE_GET_PIXEL_FAST(row_ptr_B, left - 1), zThreshold)) {
            left--;
          }

          while (right < (NEW_COLS - 1)
                 && (!IMAGE_GET_BINARY_PIXEL_FAST(bmp_row_ptr_B, right + 1))
                 && PIXEL_THRESHOLD(IMAGE_GET_PIXEL_FAST(row_ptr_B, right + 1), zThreshold)) {
            right++;
          }

          blob_x1 = MIN(blob_x1, left);
          blob_x2 = MAX(blob_x2, right);

          for (uint8_t i = left; i <= right; i++) {
            IMAGE_SET_BINARY_PIXEL_FAST(bmp_row_ptr_B, i);
            blob_depth = MAX(blob_depth, IMAGE_GET_PIXEL_FAST(row_ptr_B, i));
          }

          uint16_t sum = ((right * (right + 1)) - (left * (left - 1))) / 2;

          uint8_t pixels = right - left + 1;
          blob_pixels += pixels;
          blob_cx += sum;
          blob_cy += posY * pixels;

          uint8_t top_left = left;
          uint8_t bot_left = left;

          boolean break_out = false;

          while (1) { // while_B

            if (posY > 0) {
              row_ptr_B = COMPUTE_IMAGE_ROW_PTR(inputFrame_ptr, posY - 1);
              bmp_row_ptr_B = COMPUTE_BINARY_IMAGE_ROW_PTR(&bitmapFrame[0], posY - 1);

              boolean recurse = false;
              for (uint8_t i = top_left; i <= right; i++) {

                if ((!IMAGE_GET_BINARY_PIXEL_FAST(bmp_row_ptr_B, i))
                    && (PIXEL_THRESHOLD(IMAGE_GET_PIXEL_FAST(row_ptr_B, i), zThreshold))) {

                  xylr_t* context = (xylr_t*)llist_pop_front(&llist_context_stack);
                  //Serial.printf("\nDEBUG_LIFO / A / llist_context_stack / llist_pop_front: %p", (lnode_t*)context);

                  context->x = posX;
                  context->y = posY;
                  context->l = left;
                  context->r = right;
                  context->t_l = i++; // Don't test the same pixel again
                  context->b_l = bot_left;

                  llist_push_front(&llist_context, context);
                  //Serial.printf("\nDEBUG_LIFO / A / llist_context / llist_push_front: %p", (lnode_t*)context);

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

            row_ptr_B = COMPUTE_IMAGE_ROW_PTR(inputFrame_ptr, posY + 1);
            bmp_row_ptr_B = COMPUTE_BINARY_IMAGE_ROW_PTR(&bitmapFrame[0], posY + 1);

            boolean recurse = false;
            for (uint8_t i = bot_left; i <= right; i++) {

              if (!IMAGE_GET_BINARY_PIXEL_FAST(bmp_row_ptr_B, i)
                  && PIXEL_THRESHOLD(IMAGE_GET_PIXEL_FAST(row_ptr_B, i), zThreshold)) {

                xylr_t* context = (xylr_t*)llist_pop_front(&llist_context_stack);
                //Serial.printf("\nDEBUG_LIFO / B / llist_context_stack / llist_pop_front: %p", (lnode_t*)context);

                context->x = posX;
                context->y = posY;
                context->l = left;
                context->r = right;
                context->t_l = top_left;
                context->b_l = i++; // Don't test the same pixel again

                llist_push_front(&llist_context, context);
                //Serial.printf("\nDEBUG_LIFO / B / llist_context / llist_push_front: %p", (lnode_t*)context);

                posX = i;
                posY++;
                recurse = true;
                break;
              }
            }

            if (recurse) {
              break;
            }
            if (llist_context.head_ptr == NULL) {
              break_out = true;
              break;
            }

            xylr_t* context = (xylr_t*)llist_pop_front(&llist_context);
            //Serial.printf("\nDEBUG_LIFO / C / llist_context / llist_pop_front: %p", (lnode_t*)context);

            posX = context->x;
            posY = context->y;
            left = context->l;
            right = context->r;
            top_left = context->t_l;
            bot_left = context->b_l;

            llist_push_front(&llist_context_stack, context);
            //Serial.printf("\nDEBUG_LIFO / C / llist_context_stack / llist_push_front: %p", (lnode_t*)context);

            blob_height++;

          } // END while_B

          if (break_out) {
            break;
          }
        } // END while_A

        if (blob_pixels > MIN_BLOB_PIX) {

          blob_t* blob = (blob_t*)llist_pop_front(&llist_blobs_stack);

          blob->timeTag = millis();
          blob->centroid.X = blob_cx / (float)blob_pixels;
          blob->centroid.Y = blob_cy / (float)blob_pixels;
          blob->box.W = (blob_x2 - blob_x1);
          blob->box.H = blob_height;
          blob->box.D = blob_depth - zThreshold; // NEED TO ADD A LIMIT?
          llist_push_front(&llist_blobs, blob);
        }
        posX = oldX;
        posY = oldY;
      }
    }
  }

  /////////////////////////////// PERSISTANT BLOB ID

  // Suppress DEAD blobs from the outputBlobs linked list
  while (1) {
    boolean deadFound = false;
    blob_t* blob_ptr = (blob_t*)ITERATOR_START_FROM_HEAD(outputBlobs_ptr);
    if (blob_ptr != NULL && blob_ptr->status == TO_REMOVE) {
      deadFound = true;
      blob_ptr = (blob_t*)llist_pop_front(outputBlobs_ptr);
      blob_ptr->status = FREE;
      llist_push_front(&llist_blobs_stack, blob_ptr);
      //Serial.printf("\nDEBUG_FIND_BLOBS / Blob: %p removed from **outputBlobs** linked list", (lnode_t*)blob_ptr);
    }
    if (!deadFound) {
      break;
    }
  }

  // NEW BLOBS MANAGMENT
  // Look for corresponding blobs into the **inputBlobs** and **outputBlobs** linked list
  for (blob_t* blobIn = (blob_t*)ITERATOR_START_FROM_HEAD(&llist_blobs); blobIn != NULL; blobIn = (blob_t*)ITERATOR_NEXT(blobIn)) {
    float minDist = 255.0f;
    blob_t* nearestBlob = NULL;
    for (blob_t* blobOut = (blob_t*)ITERATOR_START_FROM_HEAD(outputBlobs_ptr); blobOut != NULL; blobOut = (blob_t*)ITERATOR_NEXT(blobOut)) {
      float dist = sqrtf(pow(blobIn->centroid.X - blobOut->centroid.X, 2) + pow(blobIn->centroid.Y - blobOut->centroid.Y, 2));
#if DEBUG_FIND_BLOBS
      Serial.printf("\nDEBUG_FIND_BLOBS / Distance between input & output blobs positions: %f ", dist);
#endif
      if (dist < minDist) {
        minDist = dist;
        nearestBlob = blobOut;
      }
    }
    // If the distance between curent blob and last blob position is less than minDist:
    // Give the nearestBlob UID to the input blob.
    if (minDist < 2) {
#if DEBUG_FIND_BLOBS
      Serial.printf("\nDEBUG_FIND_BLOBS / Found corresponding blob: %p in the **outputBlobs** linked list", (lnode_t*)nearestBlob);
#endif
      blobIn->UID = nearestBlob->UID;
      blobIn->lastState = true;
      blobIn->state = true;
    }
    // Found a new blob! We nead to give it a UID
    else {
#if DEBUG_FIND_BLOBS
      Serial.print("\nDEBUG_FIND_BLOBS / Found new blob without ID");
#endif
      // Find the smallest missing UID in the outputBlobs linked list
      uint8_t minID = 0;
      while (1) {
        boolean isFree = true;
        for (blob_t* blob_ptr = (blob_t*)ITERATOR_START_FROM_HEAD(outputBlobs_ptr); blob_ptr != NULL; blob_ptr = (blob_t*)ITERATOR_NEXT(blob_ptr)) {
          if (blob_ptr->UID == minID) {
            isFree = false;
            minID++;
            break;
          }
        }
        if (isFree) {
          blobIn->UID = minID;
          blobIn->lastState = false;
          blobIn->state = true;
          break;
        }
      } // while_end / The blob have a new ID
    }
  }

  // DEAD BLOBS MANAGMENT
  // Look for dead blobs in the outputBlobs linked list
  // If found flag it TO_REMOVE
  while (1) {
    boolean allDone = true;
    blob_t* prevBlob_ptr = NULL;
    for (blob_t* blobOut = (blob_t*)ITERATOR_START_FROM_HEAD(outputBlobs_ptr); blobOut != NULL; blobOut = (blob_t*)ITERATOR_NEXT(blobOut)) {
      boolean found = false;
      for (blob_t* blobIn = (blob_t*)ITERATOR_START_FROM_HEAD(&llist_blobs); blobIn != NULL; blobIn = (blob_t*)ITERATOR_NEXT(blobIn)) {
        if (blobOut->UID == blobIn->UID) {
          found = true;
          break;
        }
      }
      if (!found) {
        allDone = false;
        llist_extract_node(outputBlobs_ptr, prevBlob_ptr, blobOut);
        blobOut->status = NOT_FOUND;
        //Serial.printf("\nDEBUG_FIND_BLOBS / Blob: %p in the **outputBlobs** linked list is NOT_FOUND", (lnode_t*)blobOut);
        if ((millis() - blobOut->timeTag) > DEBOUNCE_TIME) {
          blobOut->state = false;
          blobOut->status = TO_REMOVE;
          //Serial.printf("\nDEBUG_FIND_BLOBS / Blob: %p in the **outputBlobs** linked list taged TO_REMOVE", (lnode_t*)blobOut);
        }
        llist_push_front(&llist_blobs, blobOut);
        break;
      }
      prevBlob_ptr = blobOut;
    }
    if (allDone) {
      break;
    }
  }

  llist_swap_llist(outputBlobs_ptr, &llist_blobs);     // Swap inputBlobs with outputBlobs linked list
  llist_save_nodes(&llist_blobs_stack, &llist_blobs);  // Rescure all dead blobs Linked list nodes

#if DEBUG_BLOBS
  for (blob_t* blob = (blob_t*)ITERATOR_START_FROM_HEAD(outputBlobs_ptr); blob != NULL; blob = (blob_t*)ITERATOR_NEXT(blob)) {
    Serial.printf("\nDEBUG_FIND_BLOBS:%d\tLS:%d\tS:%d\tX:%f\tY:%f\tW:%d\tH:%d\tD:%d\t",
                  blob->UID,
                  blob->lastState,
                  blob->state,
                  blob->centroid.X,
                  blob->centroid.Y,
                  blob->box.W,
                  blob->box.H,
                  blob->box.D
                 );
  }
#endif

#if DEBUG_BITMAP
  for (uint8_t posY = 0; posY < NEW_ROWS; posY++) {
    uint8_t* row_ptr = COMPUTE_BINARY_IMAGE_ROW_PTR(&bitmapFrame[0], posY);
    for (uint8_t posX = 0; posX < NEW_COLS; posX++) {
      IMAGE_GET_BINARY_PIXEL_FAST(row_ptr, posX) == 0 ? Serial.printf(".") : Serial.printf("o");
    }
    Serial.printf("\n");
  }
  Serial.printf("\n");
#endif
}

void getBlobsVelocity(llist_t* blobs_ptr) {
  for (blob_t* blob_ptr = (blob_t*)ITERATOR_START_FROM_HEAD(blobs_ptr); blob_ptr != NULL; blob_ptr = (blob_t*)ITERATOR_NEXT(blob_ptr)) {
    if (blob_ptr->UID < MAX_SYNTH) {
      float vx = blob_ptr->centroid.X - blobVelocity[blob_ptr->UID].lastPos.X;
      float vy = blob_ptr->centroid.Y - blobVelocity[blob_ptr->UID].lastPos.Y;

      blobVelocity[blob_ptr->UID].vxy = sqrt(vx * vx + vy * vy); //pow(vx, 2) + pow(vy, 2)
      blobVelocity[blob_ptr->UID].vz = blob_ptr->box.D - blobVelocity[blob_ptr->UID].lvz;

      blobVelocity[blob_ptr->UID].lastPos.X = blob_ptr->centroid.X;
      blobVelocity[blob_ptr->UID].lastPos.Y = blob_ptr->centroid.Y;
      blobVelocity[blob_ptr->UID].lvz = blob_ptr->box.D;

#if DEBUG_MAPPING
      Serial.printf("\nDEBUG_VELOCITY:\tvxy:%f\tvz:%f",
                    blobVelocity[blob_ptr->UID].vxy,
                    blobVelocity[blob_ptr->UID].vz
                   );
#endif
    };
  };
};

void getPolarCoordinates(llist_t* blobs_ptr) {
  for (blob_t* blob_ptr = (blob_t*)ITERATOR_START_FROM_HEAD(blobs_ptr); blob_ptr != NULL; blob_ptr = (blob_t*)ITERATOR_NEXT(blob_ptr)) {
    float posX = blob_ptr->centroid.X - CENTER_X;
    float posY = blob_ptr->centroid.Y - CENTER_Y;
    if (posX == 0 && posY == 0 ) {
      polarCoord[blob_ptr->UID].r = 0;
      polarCoord[blob_ptr->UID].phi = 0;
    }
    else {
      polarCoord[blob_ptr->UID].r = sqrt(posX * posX + posY * posY);
      if (posX == 0 && 0 < posY) {
        polarCoord[blob_ptr->UID].phi = PI / 2;
      } else if (posX == 0 && posY < 0) {
        polarCoord[blob_ptr->UID].phi = PI * 3 / 2;
      } else if (posX < 0) {
        polarCoord[blob_ptr->UID].phi = atan(posY / posX) + PI;
      } else if (posY < 0) {
        polarCoord[blob_ptr->UID].phi = atan(posY / posX) + 2 * PI;
      } else {
        polarCoord[blob_ptr->UID].phi = atan(posY / posX);
      }
    }
#if DEBUG_MAPPING
    Serial.printf("\nDEBUG_POLAR:\tR:%f\tPHY:%f", polarCoord[blob_ptr->UID].r, polarCoord[blob_ptr->UID].phi);
#endif
  }
}
