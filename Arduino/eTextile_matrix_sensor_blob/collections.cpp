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

////////////// Bitmap //////////////

void bitmap_bit_set(char* array_ptr, int index) {

  array_ptr[index >> CHAR_SHIFT] |= 1 << (index & CHAR_MASK);
}

char bitmap_bit_get(char* array_ptr, int index) {

  return (array_ptr[index >> CHAR_SHIFT] >> (index & CHAR_MASK)) & 1;
}

void bitmap_clear(char* array_ptr) {
  memset(array_ptr, 0, NEW_FRAME * sizeof(char));
}

////////////////////////////// Lifo //////////////////////////////

void lifo_raz(lifo_t *ptr) {
  ptr->head_ptr = NULL;
  ptr->index = -1;
}

void lifo_init(lifo_t* dst, xylf_t* nodesArray, uint16_t max_nodes) {

  dst->head_ptr = &nodesArray[0];
  nodesArray[0].prev_ptr = NULL;
  if (DEBUG_LIFO) Serial.printf(F("\n DEBUG_LIFO / lifo_init\t%d: %p"), 0, &nodesArray[0]);
  dst->index++;
  for (int i = 1; i < max_nodes; i++) {
    nodesArray[i].prev_ptr = &nodesArray[i - 1];
    dst->head_ptr = &nodesArray[i];
    if (DEBUG_LIFO) Serial.printf(F("\n DEBUG_LIFO / lifo_init\t%d: %p"), i, &nodesArray[i]);
    dst->index++;
  }
}

// Pull last data element in the list
xylf_t* lifo_dequeue(lifo_t* src) {

  if (src->index > -1) {
    xylf_t* node = src->head_ptr;

    if (src->index > 0) {
      if (DEBUG_LIFO) Serial.printf(F("\n DEBUG_LIFO / lifo_dequeue / Take a node: %p"), node);
      src->head_ptr = src->head_ptr->prev_ptr;
    } else {
      src->head_ptr = NULL;
      if (DEBUG_LIFO) Serial.printf(F("\n DEBUG_LIFO / lifo_dequeue / Take the last node: %p"), node);
    }
    node->prev_ptr = NULL;
    src->index--;
    if (DEBUG_LIFO) Serial.printf(F("\n DEBUG_LIFO / lifo_dequeue / SRC index is now: %d"), src->index);
    return node;
  } else {
    if (DEBUG_LIFO) Serial.printf(F("\n DEBUG_LIFO / lifo_dequeue / ERROR : SRC list is umpty!"));
    return NULL;
  }
}

// Add a data element at the end of the list
void lifo_enqueue(lifo_t* dst, xylf_t* node) {

  if (dst->index > -1) {
    if (DEBUG_LIFO) Serial.printf(F("\n DEBUG_LIFO / lifo_enqueue / Add the node: %p"), node);
    node->prev_ptr = dst->head_ptr;
    dst->head_ptr = node;
  } else {
    if (DEBUG_LIFO) Serial.printf(F("\n DEBUG_LIFO / lifo_enqueue / Add the first node: %p"), node);
    dst->head_ptr = node;
    node->prev_ptr = NULL;
  }
  dst->index++;
  if (DEBUG_LIFO) Serial.printf(F("\n DEBUG_LIFO / lifo_enqueue / DST index is now: %d"), dst->index);
}

int16_t lifo_size(lifo_t* ptr) {
  return ptr->index;
}


////////////////////////////// linked list  //////////////////////////////

void llist_raz(llist_t *ptr) {
  ptr->tail_ptr = ptr->head_ptr = NULL;
  ptr->index = -1;
}

void llist_init(llist_t* dst, blob_t* nodesArray, uint8_t max_nodes) { // uint8_t buffer_size

  dst->head_ptr = dst->tail_ptr = &nodesArray[0];
  if (DEBUG_LIST || DEBUG_CCL) Serial.printf(F("\n DEBUG_LIST / llist_init: %d: %p"), 0, &nodesArray[0]);
  dst->index++;

  for (int i = 1; i < max_nodes; i++) {
    nodesArray[i - 1].next_ptr = &nodesArray[i];
    nodesArray[i].next_ptr = NULL;
    dst->tail_ptr = &nodesArray[i];
    if (DEBUG_LIST || DEBUG_CCL) Serial.printf(F("\n DEBUG_LIST / llist_init: %d: %p"), i, &nodesArray[i]);
    dst->index++;
  }
}

blob_t* llist_pop_front(llist_t* src) {

  if (src->index > -1) {
    blob_t* node = src->head_ptr;
    if (src->index > 0) {
      src->head_ptr = src->head_ptr->next_ptr;
    } else {
      src->tail_ptr = src->head_ptr = NULL;
    }
    node->next_ptr = NULL;
    src->index--;
    return node;
  } else {
    if (DEBUG_LIST) Serial.printf(F("\n DEBUG_LIST / list_pop_front / ERROR : SRC list is umpty!"));
    return NULL;
  }
}

void llist_push_back(llist_t* dst, blob_t* node) {

  if (dst->index > -1) {
    dst->tail_ptr->next_ptr = node;
    dst->tail_ptr = node;
  } else {
    dst->head_ptr = dst->tail_ptr = node;
  }
  dst->index++;
}

// Remove a blob in a linked list
void llist_remove_blob(llist_t* src, blob_t* blobSuppr) {

  blob_t* prevBlob = NULL;
  if (DEBUG_LIST) Serial.printf(F("\n DEBUG_LIST / list_remove_blob / Blob to remove: %p"), blobSuppr);

  for (blob_t* blob = iterator_start_from_head(src); blob != NULL; blob = iterator_next(blob)) {

    if (blob == blobSuppr) {
      if (DEBUG_LIST) Serial.printf(F("\n DEBUG_LIST / list_remove_blob / Blob: %p is found"), blob);

      if (src->index == 0) {
        if (DEBUG_LIST) Serial.printf(F("\n DEBUG_LIST / list_remove_blob / The blob: %p is the first & last in the linked list"), blobSuppr);
        src->head_ptr = src->tail_ptr = NULL;
        src->index--;
        return;
      }
      else if (blob->next_ptr == NULL) {
        if (DEBUG_LIST) Serial.printf(F("\n DEBUG_LIST / list_remove_blob / The blob: %p is the tail of the linked list"), blobSuppr);
        prevBlob->next_ptr = NULL;
        src->tail_ptr = prevBlob;
        src->index--;
        return;
      }
      else if (blob == src->head_ptr) {
        if (DEBUG_LIST) Serial.printf(F("\n DEBUG_LIST / list_remove_blob / The blob: %p is the hard of the linked list"), blobSuppr);
        src->head_ptr = src->head_ptr->next_ptr;
        src->index--;
        return;
      }
      else {
        if (DEBUG_LIST) Serial.printf(F("\n DEBUG_LIST / list_remove_blob / The blob: %p is somewear else in the linked list"), blobSuppr);
        prevBlob->next_ptr = blob->next_ptr;
        src->index--;
        return;
      }
    }
    prevBlob = blob;
  }
  if (DEBUG_LIST) Serial.printf(F("\n DEBUG_LIST / list_remove_blob / ERROR / Blob not found"));
}

void llist_save_blobs(llist_t* dst, llist_t* src) {

  if (DEBUG_LIST) Serial.printf(F("\n DEBUG_LIST / list_save_blobs / START"));
  blob_t* blob = NULL;

  while (src->index > -1) {
    // SRC pop front
    blob = src->head_ptr;
    if (src->index > 0) {
      if (DEBUG_LIST) Serial.printf(F("\n DEBUG_LIST / list_save_blobs / SRC pop a blob in the list: %p"), blob);
      src->head_ptr = src->head_ptr->next_ptr;
      if (DEBUG_LIST) Serial.printf(F("\n DEBUG_LIST / list_save_blobs / SRC Move the list hed to next_ptr: %p"), src->head_ptr);
    } else { // src->index == 0
      src->tail_ptr = src->head_ptr = NULL;
      if (DEBUG_LIST) Serial.printf(F("\n DEBUG_LIST / list_save_blobs / SRC pop the last blob in the list: %p"), blob);
    }
    src->index--;
    if (DEBUG_LIST) Serial.printf(F("\n DEBUG_LIST / list_save_blobs / SRC set index to: %d"), src->index);

    // DST push back
    if (dst->index > -1) {
      dst->tail_ptr->next_ptr = blob;
      if (DEBUG_LIST) Serial.printf(F("\n DEBUG_LIST / list_save_blobs / DST add the blob to the list: %p"), blob);
      dst->tail_ptr = blob;
    } else { // dst->index == -1
      dst->head_ptr = dst->tail_ptr = blob;
      if (DEBUG_LIST) Serial.printf(F("\n DEBUG_LIST / list_save_blobs / DST add the first blob to the list"));
    }
    dst->tail_ptr->next_ptr = NULL; // Same than blob->next_ptr = NULL;
    dst->index++;
    if (DEBUG_LIST) Serial.printf(F("\n DEBUG_LIST / list_save_blobs / DST set index to: %d"), dst->index);
  }
  if (DEBUG_LIST) Serial.printf(F("\n DEBUG_LIST / list_save_blobs / SRC linked list is umpty!"));
}

void llist_copy_blob(blob_t* dst, blob_t* src) {

  //memcpy(dst, src, blobSize); // Can't do that because a blob have an next_ptr element that must not be changed!

  // dst->UID = src->UID;
  dst->centroid.x = src->centroid.x;
  dst->centroid.y = src->centroid.y;
  dst->centroid.z = src->centroid.z;
  dst->pixels = src->pixels;
  //dst->isDead = src->isDead;
}

////////////// Linked list iterators //////////////

blob_t* iterator_start_from_head(llist_t* src) {

  return src->head_ptr;
}

blob_t* iterator_next(blob_t* src) {

  return src->next_ptr;
}
