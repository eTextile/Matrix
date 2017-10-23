/* This file is part of the OpenMV project.
   Copyright (c) 2013-2017 Ibrahim Abdelkader <iabdalkader@openmv.io> & Kwabena W. Agyeman <kwagyeman@openmv.io>
   This work is licensed under the MIT license, see the file LICENSE for details.
*/

#include <Arduino.h>
#include "collections.h"
#include "fb_alloc.h"
#include "config.h"

////////////// Bitmap //////////////

void bitmap_bit_set(char* arrayPtr, int index) {
  arrayPtr[index >> CHAR_SHIFT] |= 1 << (index & CHAR_MASK);
}

char bitmap_bit_get(char* arrayPtr, int index) {
  return (arrayPtr[index >> CHAR_SHIFT] >> (index & CHAR_MASK)) & 1;
}

void bitmap_clear(char* arrayPtr) {
  memset(arrayPtr, 0, ((NEW_FRAME + CHAR_MASK) >> CHAR_SHIFT) * sizeof(char));
}

void bitmap_print(char* arrayPtr) {

  Serial.printf(F("\n>>>> Bitmap print"));
  Serial.printf("\n");
  for (int i = 0; i < NEW_ROWS; i++) {
    for (int j = 0; j < NEW_COLS; j++) {
      Serial.printf( "%d " , bitmap_bit_get(arrayPtr, i * NEW_ROWS + j));
    }
    Serial.printf("\n");
  }
  Serial.printf("\n");
}

////////////// Lifo //////////////

void lifo_alloc_all(lifo_t* ptr, size_t* _size, size_t struct_size) {

  uint32_t tmp_size;

  ptr->data = (char*) fb_alloc_all(&tmp_size);
  ptr->data_len = struct_size;
  ptr->index = tmp_size / struct_size;
  ptr->len = 0;
  _size = &ptr->index;
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
  ptr->len += 1;
}

// Cpoy the lifo data into data, exept the last element
void lifo_dequeue(lifo_t* ptr, void* data) {
  if (data) {
    memcpy(data, ptr->data + ((ptr->len - 1) * ptr->data_len), ptr->data_len);
  }
  ptr->len -= 1;
}

////////////// List //////////////

void list_init(list_t* ptr, size_t data_len) {

  ptr->head_ptr = NULL;
  ptr->tail_ptr = NULL;
  ptr->index = 0;
  ptr->data_len = data_len;
}

void list_copy(list_t* dst, list_t* src) {
  memcpy(dst, src, sizeof(list_t));
}

size_t list_size(list_t* ptr) {
  return ptr->index;
}

/////////// list_push_back() ///////////
// Adds a new element at the end of the list container
// The content of data is added to the new element
//
// struct list_t:  node_t* head_ptr;
//                 node_t* tail_ptr;
//                 size_t  data_len;
//                 size_t  index;
// struct node_t:  struct node* next_ptr;
//                 struct node* prev_ptr;
//                 char data[];
//
// (head)            (tail)
// node_0 < node_1 < node_2

void list_push_back(void* data, list_t* ptr, node_t* tmpNodePtr) {

  if (DEBUG_BLOB) Serial.printf(F("\n>>>> list_push_back / Start"));

  // tmpNodePtr = ptr->tail_ptr; // Get the address of the last node in the tmpList
  // if (DEBUG_BLOB) Serial.printf(F("\n>>>> list_push_back / Get the address of the last node in the list: %p"), &tmpNodePtr);

  memcpy(tmpNodePtr->data, data, ptr->data_len); // Copy the input data to a temporary node
  if (DEBUG_BLOB) Serial.printf(F("\n>>>> list_push_back / Data copyed to tmpNode "));

  if (ptr->index++) {
    tmpNodePtr->next_ptr = NULL;
    tmpNodePtr->prev_ptr = ptr->tail_ptr;
    ptr->tail_ptr->next_ptr = tmpNodePtr;
    ptr->tail_ptr = tmpNodePtr;
    if (DEBUG_BLOB) Serial.printf(F("\n>>>> list_push_back / Moved pointers"));
  } else {
    tmpNodePtr->next_ptr = NULL;
    tmpNodePtr->prev_ptr = NULL;
    ptr->head_ptr = tmpNodePtr;
    ptr->tail_ptr = tmpNodePtr;
  }
  if (DEBUG_BLOB) Serial.printf(F("\n>>>> list_push_back / Exit"));
}

/////////// list_pop_front() ///////////
// Take the first blob element of the blob list container
// Copy this blob in to the external tmp blob (*data)
//
// struct list_t:  node_t* head_ptr;
//                 node_t* tail_ptr;
//                 size_t  index;
//                 size_t  data_len;
// struct node_t:  struct node* next_ptr;
//                 struct node* prev_ptr;
//                 char data[];
//
// (head)            (tail)
// node_0 < node_1 < node_2

void list_pop_front(void* data, list_t* ptr, node_t* tmpNodePtr) { // TODO change node_t* tmpNodePtr TO list_t* tmpListPtr

  tmpNodePtr = ptr->head_ptr; // Get the address of the first node in the list
  if (DEBUG_BLOB) Serial.printf(F("\n>>>> list_pop_front / tmpNodePtr is pointing to the head of the list"));

  if (data) {
    memcpy(data, tmpNodePtr->data, ptr->data_len);
    if (DEBUG_BLOB) Serial.printf(F("\n>>>> list_pop_front / Copy done!"));
  }
  // tmpNodePtr->next_ptr->prev_ptr = NULL; // DO NOT WORK !?
  tmpNodePtr->next_ptr = tmpNodePtr->prev_ptr;
  // tmpNodePtr->prev_ptr = NULL;

  if (DEBUG_BLOB) Serial.printf(F("\n>>>> list_pop_front / Clear tmpNode pointers"));

  ptr->head_ptr = tmpNodePtr->next_ptr;
  if (DEBUG_BLOB) Serial.printf(F("\n>>>> list_pop_front / Updated head of the list"));

  ptr->index--;
  if (DEBUG_BLOB) Serial.printf(F("\n>>>> list_pop_front / Updated size of the list: %d"), ptr->index);

  if (DEBUG_BLOB) Serial.printf(F("\n>>>> Exiting list pop front"));
}

////////////// Iterators //////////////

node_t* iterator_start_from_head(list_t* ptr) {
  return ptr->head_ptr;
}

node_t* iterator_next(node_t* lnk) {
  return lnk->next_ptr;
}

// void *memcpy(void *str1, const void *str2, size_t n)
// str1 − This is pointer to the destination array where the content is to be copied, type-casted to a pointer of type void*.
// str2 − This is pointer to the source of data to be copied, type-casted to a pointer of type void*.
// n − This is the number of bytes to be copied.
void iterator_get(void* data, node_t* lnk, list_t* ptr) {
  memcpy(data, lnk->data, ptr->data_len);
}

