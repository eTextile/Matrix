/* This file is part of the OpenMV project.
   Copyright (c) 2013-2017 Ibrahim Abdelkader <iabdalkader@openmv.io> & Kwabena W. Agyeman <kwagyeman@openmv.io>
   This work is licensed under the MIT license, see the file LICENSE for details.
*/

#include <Arduino.h>
#include "fb_alloc.h"
#include "config.h"
#include "collections.h"

////////////// Bitmap //////////////

void bitmap_bit_set(char* arrayPtr, int index) {
  arrayPtr[index >> CHAR_SHIFT] |= 1 << (index & CHAR_MASK);
}

char bitmap_bit_get(char* arrayPtr, int index) {
  return (arrayPtr[index >> CHAR_SHIFT] >> (index & CHAR_MASK)) & 1;
}

void bitmap_clear(char* arrayPtr) {
  // memset(ptr->data, 0, ((ptr->size + CHAR_MASK) >> CHAR_SHIFT) * sizeof(char));
  memset(arrayPtr, 0, NEW_FRAME * sizeof(char));
}

void bitmap_print(char* arrayPtr) {

  Serial.printf(F("\n>>>> Bitmap <<<<\n"));
  for (int i = 0; i < NEW_ROWS; i++) {
    for (int j = 0; j < NEW_COLS; j++) {
      Serial.printf("%d ", bitmap_bit_get(arrayPtr, i * NEW_ROWS + j));
    }
    Serial.printf("\n");
  }
  Serial.printf("\n");
}

////////////// Lifo //////////////

void lifo_alloc_all(lifo_t* ptr, size_t* lifo_len, size_t struct_size) {

  uint32_t tmp_size;

  ptr->data = (char*) fb_alloc_all(&tmp_size);
  ptr->data_len = struct_size;
  ptr->index = tmp_size / struct_size;
  ptr->len = 0;
  *lifo_len = ptr->index;
}

void lifo_free(lifo_t* ptr) {
  if (ptr->data) {
    fb_free();
  }
}

size_t lifo_size(lifo_t* ptr) {
  return ptr->len;
}

// Add data at the end of the lifo buffer
void lifo_enqueue(lifo_t* ptr, void* data) {
  memcpy(ptr->data + (ptr->len * ptr->data_len), data, ptr->data_len);
  ptr->len++;
}

// Cpoy the lifo data into data, exept the last element
void lifo_dequeue(lifo_t* ptr, void* data) {
  if (data) {
    memcpy(data, ptr->data + ((ptr->len - 1) * ptr->data_len), ptr->data_len);
  }
  ptr->len--;
}

////////////////////////////// linked list  //////////////////////////////

void list_init(list_t *ptr) {
  ptr->tail_ptr = ptr->head_ptr = NULL;
  ptr->index = -1;
}

void list_alloc_all(list_t* dst, blob_t* blobs) {

  dst->head_ptr = dst->tail_ptr = &blobs[0];
  if (DEBUG_LIST) Serial.printf(("\n>>>> list_alloc_all\t%d: %p"), 0, &blobs[0]);
  dst->index++;

  for (int i = 1; i < MAX_NODES; i++) {
    blobs[i - 1].next_ptr = &blobs[i];
    dst->tail_ptr = &blobs[i];
    blobs[i].UID = -1;
    if (DEBUG_LIST) Serial.printf(("\n>>>> list_alloc_all\t%d: %p"), i, &blobs[i]);
    dst->index++;
  }
  dst->tail_ptr->next_ptr = NULL;
}

blob_t* list_pop_front(list_t* src) {

  if (src->index > -1) {
    blob_t* blob = src->head_ptr;
    if (src->index == 0) {
      src->tail_ptr = src->head_ptr = NULL;
      src->index--;
      return blob;
    } else {
      src->head_ptr = src->head_ptr->next_ptr;
      src->index--;
      return blob;
    }
  } else {
    if (DEBUG_LIST) Serial.printf(F("\n>>>>>>>>> list_pop_front / ERROR : SRC list is umpty!"));
    return NULL;
  }
}

void list_push_back(list_t* dst, blob_t* blob) {

  if (dst->index == -1) {
    dst->head_ptr = dst->tail_ptr = blob;
  } else {
    dst->tail_ptr->next_ptr = blob;
    dst->tail_ptr = blob;
  }
  blob->next_ptr = NULL;
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

blob_t* list_get_blob(list_t* src, uint8_t index) {

  blob_t* prevBlob = NULL;
  blob_t* blob = src->head_ptr;

  if ((src->index > -1) && (index <= src->index)) {
    if (src->index == 0) { // If we whant to take the head blob
      if (src->index > 0) {
        src->head_ptr = src->head_ptr->next_ptr;
        // blob->next_ptr = NULL; //  We do it in the list_push_back()
        src->index--;
        return blob;
      } else {
        src->tail_ptr = src->head_ptr = NULL;
        // blob->next_ptr = NULL; // I do it in the list_push_back()
        src->index--;
        return blob;
      }
    } else { // If we whant to take an other blob
      for (int i = 0; i < index; i++) {
        prevBlob = blob;
        if (blob->next_ptr != NULL) {
          blob = blob->next_ptr;
        } else {
          prevBlob->next_ptr = src->tail_ptr; // We take the last blob in the list
          src->index--;
          return blob;
        }
      }
      prevBlob->next_ptr = blob->next_ptr;
      // blob->next_ptr = NULL; // I do it in the list_push_back()
      src->index--;
      return blob;
    }
  } else {
    if (DEBUG_LIST) Serial.printf(F("\n>>>>>>>>> list_get_blob / ERROR / this node is not in the list"));
    return NULL;
  }
}

void list_save_blobs(list_t* dst, list_t* src) {
  if (DEBUG_LIST) Serial.printf(F("\n>>>> list_save_blobs / START"));

  blob_t* blob;

  while (src->index != -1) {

    // SRC pop front
    if (src->index == 0) {
      blob = src->head_ptr;
      src->tail_ptr = src->head_ptr = NULL;
      if (DEBUG_LIST) Serial.printf(F("\n>>>> list_save_blobs / SRC pop the last blob in the list: %p"), blob);
    } else { // dst->index != -1
      blob = src->head_ptr;
      if (DEBUG_LIST) Serial.printf(F("\n>>>> list_save_blobs / SRC pop a blob in the list: %p"), blob);
      src->head_ptr = src->head_ptr->next_ptr;
      if (DEBUG_LIST) Serial.printf(F("\n>>>> list_save_blobs / SRC Move the list hed to next_ptr: %p"), src->head_ptr);
    }
    src->index--;
    // DST push back
    if (dst->index == -1) {
      dst->head_ptr = dst->tail_ptr = blob;
      if (DEBUG_LIST) Serial.printf(F("\n>>>> list_save_blobs / DST add the first blob to the list"));
      dst->tail_ptr->next_ptr = NULL;
    } else { // dst->index != -1
      dst->tail_ptr->next_ptr = blob;
      dst->tail_ptr = blob;
      if (DEBUG_LIST) Serial.printf(F("\n>>>> list_save_blobs / DST add a new blob to the list: %p"), blob);
      dst->tail_ptr->next_ptr = NULL;
    }
    dst->index++;
    if (DEBUG_LIST) Serial.printf(F("\n>>>> list_save_blobs / DST set index to: %d"), dst->index);
  }
  if (DEBUG_LIST) Serial.printf(F("\n>>>> list_save_blobs / SRC linked list is umpty"));
}

void list_clear_blobs(list_t* src) {
  src->tail_ptr = src->head_ptr = NULL;
}

void list_copy_blob(blob_t* blobA, blob_t* blobB, size_t blobSize) {

  // blobA->UID = blobB->UID;
  // blobA->centroid.x = blobB->centroid.x;
  // blobA->centroid.y = blobB->centroid.y;
  // blobA->centroid.z = blobB->centroid.z;
  // blobA->pixels;
  // blobA->isDead;

  memcpy(blobA, blobB, blobSize);
}


////////////// Linked list iterators //////////////

blob_t* iterator_start_from_head(list_t* src) {
  return src->head_ptr;
}
blob_t* iterator_next(blob_t* src) {
  return src->next_ptr;
}
int8_t list_size(list_t* src) {
  return src->index;
}
