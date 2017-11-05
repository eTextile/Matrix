/* This file is part of the OpenMV project.
   Copyright (c) 2013-2017 Ibrahim Abdelkader <iabdalkader@openmv.io> & Kwabena W. Agyeman <kwagyeman@openmv.io>
   This work is licensed under the MIT license, see the file LICENSE for details.
*/

#include <Arduino.h>
#include "blob.h"
#include "collections.h"
#include "config.h"

void find_blobs(
  const image_t*      input_ptr,
  char*               bitmap_ptr,
  const int           rows,
  const int           cols,
  const int           pixelThreshold,
  const unsigned int  minBlobPix,
  const unsigned int  maxBlobPix,
  list_t*             freeNodeList_ptr,
  list_t*             nodes_ptr,
  list_t*             oldNodesToUpdate_ptr,
  list_t*             nodesToUpdate_ptr,
  list_t*             nodesToAdd_ptr,
  list_t*             outputNodes_ptr
) {

  if (DEBUG_BLOB) Serial.printf(F("\n>>>>>>>> STARTING BLOB SCANNING"));

  lifo_t lifo;
  size_t lifoLen;

  lifo_alloc_all(&lifo, &lifoLen, sizeof(xylf_t));
  int count = 0;

  // list_copy(freeNodeList_ptr, nodes_ptr);            // Remove & save node from the nodes_ptr linked list

  if (DEBUG_BLOB) Serial.printf(F("\n>>>>>>>> A-Lifo len: %d"), lifoLen);
  if (DEBUG_BLOB) Serial.printf(F("\n>>>>>>>> A-Lifo size: %d"), lifo_size(&lifo));

  for (int posY = 0; posY < rows; posY++) {

    uint8_t* row_ptr = FRAME_ROW_PTR(input_ptr, posY);       // Return pointer to image curent row
    size_t row_index = BITMAP_ROW_INDEX(input_ptr, posY);    // Return bitmap curent row

    for (int posX = 0; posX < cols; posX++) {
      if (DEBUG_INPUT) Serial.printf("%d ", GET_FRAME_PIXEL(row_ptr, posX)); // <<<< INPUT VALUES!!

      if (!bitmap_bit_get(bitmap_ptr, BITMAP_INDEX(row_index, posX)) &&
          PIXEL_THRESHOLD(GET_FRAME_PIXEL(row_ptr, posX), pixelThreshold)) {

        if (DEBUG_INPUT) Serial.printf(F("\n>>>>>>>> INIT BLOB!\n"));

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

        int16_t cx = blob_cx / blob_pixels; // x centroid
        int16_t cy = blob_cy / blob_pixels; // y centroid

        row_ptr = FRAME_ROW_PTR(input_ptr, cy); // Return pointer to image curent row
        int16_t cz = GET_FRAME_PIXEL(row_ptr, cx);  //

        blob_t blob;

        blob.pixels = blob_pixels;
        blob.centroid.x = cx;
        blob.centroid.y = cy;
        blob.centroid.z = cz - THRESHOLD;
        blob.isDead = false;

        if (DEBUG_BLOB) Serial.printf(F("\n>>>>>>>> Add values to the blob"));

        if ((blob.pixels <= maxBlobPix) && (blob.pixels >= minBlobPix)) {
          blob.UID = count;
          if (DEBUG_BLOB) Serial.printf(F("\n>>>>>>>> Blob ID: %d"), count);
          count++;
          list_push_back(nodes_ptr, list_get_freeNode(freeNodeList_ptr), &blob);
          if (DEBUG_BLOB) Serial.printf(F("\n>>>>>>>> Added node to the nodes_ptr linked list: %d"), list_size(nodes_ptr));
        }
        posX = old_x;
        posY = old_y;
      }
    }
  }

  ///////////////////////////////////////////////////////////////////////////////////////// Percistant blobs/nodes

  // Look for the nearest blob between curent blob position (nodes_ptr) and last blob position (outputNodes_ptr)
  for (node_t* nodeA = iterator_start_from_head(nodes_ptr); nodeA != NULL; nodeA = iterator_next(nodeA)) {

    float minDist = 1000;
    int nearestBlobIndex = -1;
    int index = 0;

    blob_t blobA;
    iterator_get(nodes_ptr, nodeA, &blobA);

    for (node_t* nodeB = iterator_start_from_head(outputNodes_ptr); nodeB != NULL; nodeB = iterator_next(nodeB)) {

      blob_t blobB;
      iterator_get(outputNodes_ptr, nodeB, &blobB);

      int16_t xa = blobA.centroid.x;
      int16_t ya = blobA.centroid.y;
      int16_t xb = blobB.centroid.x;
      int16_t yb = blobB.centroid.y;

      int dist = (int) sqrt(pow(xa - xb, 2) + pow(ya - yb, 2)); // fast_sqrt? & fast_pow?

      if (dist < minDist) {
        minDist = dist;
        nearestBlobIndex = index;
      }
      index++;
    }

    // Compare the distance between current blobs (nodes_ptr) and last blobs (outputNodes_ptr)
    // If the distance is less than a cetain threshold select the blob
    if (minDist < 10) {

      list_push_back(nodesToUpdate_ptr, list_get_freeNode(freeNodeList_ptr), &blobA); // From nodes_ptr

      node_t nearestNode;
      list_get_node(outputNodes_ptr, nearestBlobIndex, &nearestNode); // Do not remove the node from the SRC linked list
      list_push_back(oldNodesToUpdate_ptr, list_get_freeNode(freeNodeList_ptr), nearestNode.data); // From outputNodes_ptr
      
    } else {
      // Found a new blob! we nead to geave it an ID.
      // We look through the outputNodes linked list to get the minimum unused ID.

      bool reachEnd = false;
      int minID = 0;

      while (!reachEnd) {
        reachEnd = true;
        for (node_t* node = iterator_start_from_head(outputNodes_ptr); node != NULL; node = iterator_next(node)) {
          blob_t blob;
          iterator_get(outputNodes_ptr, node, &blob);
          if (blob.UID == minID) {
            minID ++;
            reachEnd = false;
            break;
          }
        }
      }
      blobA.UID = minID;
      list_push_back(nodesToAdd_ptr, list_get_freeNode(freeNodeList_ptr), &blobA);
    }

  }

  // Update outputNodes linked list with nodesToUpdate linked list
  for (node_t* nodeA = iterator_start_from_head(outputNodes_ptr); nodeA != NULL; nodeA = iterator_next(nodeA)) {
    bool found = false;
    blob_t blobA;
    iterator_get(outputNodes_ptr, nodeA, &blobA);

    for (node_t* nodeB = iterator_start_from_head(nodesToUpdate_ptr); nodeB != NULL; nodeB = iterator_next(nodeB)) {
      blob_t blobB;
      iterator_get(nodesToUpdate_ptr, nodeB, &blobB);

      blob_t oldBlob;
      iterator_get(oldNodesToUpdate_ptr, nodeB, &oldBlob);

      if (oldBlob.UID == blobB.UID) {
        found = true;
        blobA.centroid.x = blobB.centroid.x;
        blobA.centroid.y = blobB.centroid.y;
        blobA.centroid.z = blobB.centroid.z;
        blobA.pixels = blobB.pixels;
      }
    }
    if (!found) {
      // message.addIntArg(blobA.UID);
      // message.addIntArg(-1);
      // message.addIntArg(-1);
      // message.addIntArg(-1);
      // message.addIntArg(-1);
      // bundle.addMessage(message);
      blobA.isDead = true;
    }
  }

  // Suppres dead blobs from the outputNodes linked list
  bool deadExists = true;
  while (deadExists) {
    int index = 0;
    int deadBlob = -1;

    for (node_t* node = iterator_start_from_head(outputNodes_ptr); node != NULL; node = iterator_next(node)) {
      blob_t blob;
      iterator_get(outputNodes_ptr, node, &blob);

      if (blob.isDead) {
        deadBlob = index;
        break;
      }
      index++;
    }
    if (deadBlob != -1) {
      node_t node;
      list_remove_node(outputNodes_ptr, &node, deadBlob); // Remove a node from the outputNodes linked list
      list_save_node(freeNodeList_ptr, &node); // Save this node to the freeNodeList linked list
    } else {
      deadExists = false;
    }
  }

  for (node_t* node = iterator_start_from_head(nodesToAdd_ptr); node != NULL; node = iterator_next(node)) {
    list_push_back(outputNodes_ptr, list_get_freeNode(freeNodeList_ptr), node->data); // Is it good!?
  }

  for (node_t* node = iterator_start_from_head(outputNodes_ptr); node != NULL; node = iterator_next(node)) {
    blob_t blob;
    iterator_get(outputNodes_ptr, node, &blob);

    if (!blob.isDead) {
      // message.addIntArg(blob.UID);
      // message.addIntArg(blob.centroid.x);
      // message.addIntArg(blob.centroid.y);
      // message.addIntArg(blob.centroid.z);
      // message.addIntArg(blob.pixels);
      // bundle.addMessage(message);
      if (DEBUG_OSC) {
        Serial.printf(F("\n>>>> UID: %d\tX: %d\tY: %d\tZ: %d\tPIX: %d"),
                      blob.UID,
                      blob.centroid.x,
                      blob.centroid.y,
                      blob.centroid.z,
                      blob.pixels
                     );
      }
    }
  }
  // sender.sendBundle(bundle);

  lifo_free(&lifo);
  if (DEBUG_BITMAP) bitmap_print(bitmap_ptr);
  bitmap_clear(bitmap_ptr); // TODO: optimizing!
  if (DEBUG_BLOB) Serial.printf(F("\n>>>>>>>> Cleared bitmap"));
  if (DEBUG_BLOB) Serial.printf(F("\n>>>>>>>> END OFF BLOB FONCTION"));

  list_copy(freeNodeList_ptr, nodes_ptr);            // Remove & save node from the nodes_ptr linked list
  list_copy(freeNodeList_ptr, oldNodesToUpdate_ptr); // Remove & save node from the oldNodesToUpdate_ptr linked list
  list_copy(freeNodeList_ptr, nodesToUpdate_ptr);    // Remove & save node from the nodesToUpdate_ptr linked list
  list_copy(freeNodeList_ptr, nodesToAdd_ptr);       // Remove & save node from the nodesToAdd_ptr linked list
}
