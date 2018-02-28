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

void bitmap_print(char* array_ptr) {

  Serial.printf(F("\n>>>> Bitmap <<<<\n"));
  for (int i = 0; i < NEW_ROWS; i++) {
    for (int j = 0; j < NEW_COLS; j++) {
      Serial.printf("%d ", bitmap_bit_get(array_ptr, i * NEW_ROWS + j));
    }
    Serial.printf("\n");
  }
  Serial.printf("\n");
}

////////////////////////////// Lifo //////////////////////////////

void lifo_alloc(lifo_t* ptr, xylf_t* array_ptr, size_t struct_size) {
  ptr->data_ptr = (char*) &array_ptr[0];
  ptr->bloc_size = struct_size;
  ptr->index = 0;
}

size_t lifo_size(lifo_t* ptr) {
  return ptr->index;
}

// Add a data element at the end of the list
void lifo_enqueue(lifo_t* ptr, void *data) {
  memcpy(ptr->data_ptr + (ptr->index * ptr->bloc_size), data, ptr->bloc_size);
  ptr->index++;
}

// Pull out last data element in the list
void lifo_dequeue(lifo_t* ptr, void* data) {
  if (data) {
    memcpy(data, ptr->data_ptr + ((ptr->index - 1) * ptr->bloc_size), ptr->bloc_size);
  }
  ptr->index--;
}

void lifo_init(lifo_t* ptr) {
  ptr->index = 0;
}

////////////////////////////// linked list  //////////////////////////////

void list_init(list_t *ptr) {
  ptr->tail_ptr = ptr->head_ptr = NULL;
  ptr->index = -1;
}

void list_alloc_all(list_t* dst, blob_t* blobs) {

  dst->head_ptr = dst->tail_ptr = &blobs[0];
  if (DEBUG_LIST || DEBUG_CCL) Serial.printf(("\n>>>> list_alloc_all\t%d: %p"), 0, &blobs[0]);
  dst->index++;

  for (int i = 1; i < MAX_NODES; i++) {
    blobs[i - 1].next_ptr = &blobs[i];
    blobs[i].UID = -1;
    blobs[i].next_ptr = NULL;
    dst->tail_ptr = &blobs[i];
    if (DEBUG_LIST || DEBUG_CCL) Serial.printf(("\n>>>> list_alloc_all\t%d: %p"), i, &blobs[i]);
    dst->index++;
  }
}

blob_t* list_pop_front(list_t* src) {

  if (src->index > -1) {
    blob_t* blob = src->head_ptr;

    if (src->index > 0) {
      src->head_ptr = src->head_ptr->next_ptr;
    } else {
      src->tail_ptr = src->head_ptr = NULL;
    }
    // blob->next_ptr = NULL;
    src->index--;
    return blob;
  } else { // SRC list is umpty!
    if (DEBUG_LIST) Serial.printf(F("\n>>>>>>>>> list_pop_front / ERROR : SRC list is umpty!"));
    return NULL;
  }
}

void list_push_back(list_t* dst, blob_t* blob) {

  if (dst->index > -1) {
    dst->tail_ptr->next_ptr = blob;
    dst->tail_ptr = blob;
  } else {
    dst->head_ptr = dst->tail_ptr = blob;
  }
  dst->tail_ptr->next_ptr = NULL; // Do not work if we do it in the list_pop_front();
  dst->index++;
}

blob_t* list_read_blob(list_t* src, uint8_t index) {

  blob_t* blob;

  if ((src->index > -1) && (index <= src->index)) {
    blob = src->head_ptr;
    for (int i = 0; i < index; i++) {
      blob = blob->next_ptr;
    }
    return blob;
  } else {
    if (DEBUG_LIST) Serial.printf(F("\n>>>>>>>>> list_read_blob / ERROR : blob is not in the list"));
    return NULL;
  }
}

// Look for a blob in a linked list & remove it
void list_remove_blob(list_t* src, blob_t* blob) {

  blob_t* prevBlob = NULL;

  for (blob_t* srcTmpBlob = iterator_start_from_head(src); srcTmpBlob != NULL; srcTmpBlob = iterator_next(srcTmpBlob)) {
    if (DEBUG_LIST) Serial.printf(F("\n>>>> list_remove_blob / blob to remove: %p"), srcTmpBlob);

    if (srcTmpBlob == blob) {
      if (DEBUG_LIST) Serial.printf(F("\n>>>> list_remove_blob / found the blob to remove: %p"), srcTmpBlob);

      if (srcTmpBlob == src->head_ptr && srcTmpBlob == src->tail_ptr) {
        src->head_ptr = src->tail_ptr = NULL;
        src->index--;
        if (DEBUG_LIST) Serial.printf(F("\n>>>> list_remove_blob / this blob is the first & last in the linked list: %p"), blob);
        return;
      }
      else if (srcTmpBlob->next_ptr == NULL) {
        prevBlob->next_ptr == NULL;
        src->tail_ptr = prevBlob; /////////// BUBFIX!
        src->index--;
        if (DEBUG_LIST) Serial.printf(F("\n>>>> list_remove_blob / this blob is the tail of the linked list: %p"), blob);
        return;
      }
      else {
        if (DEBUG_LIST) Serial.printf(F("\n>>>> list_remove_blob / Remove: %p"), srcTmpBlob);
        prevBlob->next_ptr = srcTmpBlob->next_ptr;
        src->index--;
        if (DEBUG_LIST) Serial.printf(F("\n>>>> list_remove_blob / this blob is in the middle of the linked list: %p"), blob);
        return;
      }
    }
    prevBlob = srcTmpBlob;
  }
  if (DEBUG_LIST) Serial.printf(F("\n>>>> list_remove_blob / ERROR / Not found"));
  // return NULL;
}

void list_save_blobs(list_t* dst, list_t* src) {
  if (DEBUG_LIST) Serial.printf(F("\n>>>> list_save_blobs / START"));

  blob_t* blob = NULL;

  while (src->index > -1) {
    // SRC pop front
    blob = src->head_ptr;
    if (src->index > 0) {
      if (DEBUG_LIST) Serial.printf(F("\n>>>> list_save_blobs / SRC pop a blob in the list: %p"), blob);
      src->head_ptr = src->head_ptr->next_ptr;
      if (DEBUG_LIST) Serial.printf(F("\n>>>> list_save_blobs / SRC Move the list hed to next_ptr: %p"), src->head_ptr);
    } else { // src->index == 0
      src->tail_ptr = src->head_ptr = NULL;
      if (DEBUG_LIST) Serial.printf(F("\n>>>> list_save_blobs / SRC pop the last blob in the list: %p"), blob);
    }
    src->index--;
    if (DEBUG_LIST) Serial.printf(F("\n>>>> list_save_blobs / SRC set index to: %d"), src->index);

    // DST push back
    if (dst->index > -1) {
      dst->tail_ptr->next_ptr = blob;
      if (DEBUG_LIST) Serial.printf(F("\n>>>> list_save_blobs / DST add the blob to the list: %p"), blob);
      dst->tail_ptr = blob;
    } else { // dst->index == -1
      dst->head_ptr = dst->tail_ptr = blob;
      if (DEBUG_LIST) Serial.printf(F("\n>>>> list_save_blobs / DST add the first blob to the list"));
    }
    dst->tail_ptr->next_ptr = NULL; // Same than blob->next_ptr = NULL;
    dst->index++;
    if (DEBUG_LIST) Serial.printf(F("\n>>>> list_save_blobs / DST set index to: %d"), dst->index);
  }
  if (DEBUG_LIST) Serial.printf(F("\n>>>> list_save_blobs / SRC linked list is umpty!"));
}

void list_copy_blob(blob_t* blobA, blob_t* blobB, size_t blobSize) {
  /*
    blobA->UID = blobB->UID;
    blobA->centroid.x = blobB->centroid.x;
    blobA->centroid.y = blobB->centroid.y;
    blobA->centroid.z = blobB->centroid.z;
    blobA->pixels;
    blobA->isDead;
  */
  memcpy(blobA, blobB, blobSize);
}

////////////// Linked list iterators //////////////

blob_t* iterator_start_from_head(list_t* src) {
  return src->head_ptr;
}
blob_t* iterator_next(blob_t* src) {
  return src->next_ptr;
}
