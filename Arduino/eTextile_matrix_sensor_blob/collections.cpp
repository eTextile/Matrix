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

void lifo_alloc_all(lifo_t* ptr, size_t* lifo_len, size_t struct_size) {

  uint32_t tmp_size;

  ptr->data = (char*) fb_alloc_all(&tmp_size);
  ptr->data_len = struct_size;
  ptr->index = tmp_size / struct_size;
  ptr->len = 0;
  *lifo_len = ptr->index; // Same than lifo_len = &ptr->index; ?
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

////////////// List //////////////
/*
     list_init()
     Initialise the linked list
*/
void list_init(list_t* ptr, size_t data_len) {
  ptr->head_ptr = NULL;
  ptr->index = 0;
  ptr->data_len = data_len;
  if (DEBUG_LIST) Serial.printf(F("\n>>>> linked list init"));
}

/*
    eTextile.org / list_alloc_all();
    allocate the linked list
    (head)                     (tail)
    node_3 > node_2 > node_1 > node_0
*/
void list_alloc_all(list_t* ptr, size_t data_len) {
  // Linked list push front

  for (int i = 0; i < MAX_BLOBS; i++) {
    node_t* node = (node_t*) malloc(data_len); // Initialize list size;
    if (node == NULL) {
      Serial.printf(F("\n>>>> ALLOC_FAILURE"));
      delay(2000);
      return;
    }
    if (ptr->head_ptr != NULL) {
      node->next_ptr = ptr->head_ptr;
      memcpy(node->data, 0, ptr->data_len);
      ptr->head_ptr = node;
      ptr->index++;
    } else {
      node->next_ptr = NULL; // Head of the linked list
      memcpy(node->data, 0, ptr->data_len);
      ptr->head_ptr = node;
    }
  }
}

/*
    eTextile.org / list_get_node_ptr();
    (head)                      (tail)
    node_0 > node_1 > node_2 > node_3
*/
void list_get_freeNode(list_t* ptr, node_t* node) {
  node = ptr->head_ptr;
  if (DEBUG_LIST) Serial.printf(F("\n>>>> list_get_freeNode / Start"));
  if (ptr->head_ptr != NULL) {
    if (DEBUG_LIST) Serial.printf(F("\n>>>> list_get_freeNode / Find free node: %d"), ptr->index);
    ptr->head_ptr = node->next_ptr;
    ptr->index--;
    if (DEBUG_LIST) Serial.printf(F("\n>>>> list_get_freeNode / The head node of the linked list is now: %d"), ptr->index);
    if (DEBUG_LIST) Serial.printf(F("\n>>>> list_get_freeNode / Exit"));
    return;
  } else {
    if (DEBUG_LIST) Serial.printf(F("\n>>>> list_get_freeNode / Linked list is umpty"));
    return;
  }
}

/*
    eTextile.org / list_set_node_ptr();
    (head)                      (tail)
    node_0 > node_1 > node_2 > node_3
*/
void list_save_node(list_t* ptr, node_t* node) {
  // Linked list push front
  if (DEBUG_LIST) Serial.printf(F("\n>>>> list_save_node / Start"));

  if (ptr->head_ptr != NULL) {
    node = ptr->head_ptr;
    // memcpy(node->data, 0, ptr->data_len); // Do I nead to clean the node values!?
    ptr->head_ptr = node;
    ptr->index++;
    if (DEBUG_LIST) Serial.printf(F("\n>>>> list_save_node / Add the node at the head of the linked list: %d"), ptr->index);
    if (DEBUG_LIST) Serial.printf(F("\n>>>> list_save_node / Exit"));
    return;
  } else {
    if (DEBUG_LIST) Serial.printf(F("\n>>>> list_save_node / Linked list is umpty"));
    node->next_ptr = NULL; // Head of the linked list
    if (DEBUG_LIST) Serial.printf(F("\n>>>> list_save_node / Set the node as head of the linked list"));
    // memcpy(node->data, 0, ptr->data_len); // Do I nead to clean the node values!?
    ptr->head_ptr = node;
    if (DEBUG_LIST) Serial.printf(F("\n>>>> list_save_node / Exit"));
    return;
  }
}


void list_copy(list_t* dst, list_t* src) {
  memcpy(dst, src, sizeof(list_t));
}

size_t list_size(list_t* ptr) {
  return ptr->index;
}

/*
  eTextile.org / list_push_back();
  (head)                     (tail)
  node_0 > node_1 > node_2 > node_3
  http://www.geeksforgeeks.org/linked-list-set-2-inserting-a-node/
*/
void list_push_back(list_t* ptr, void* data, node_t* node) {
  if (DEBUG_LIST) Serial.printf(F("\n>>>> list_push_back / Start"));

  if (ptr->head_ptr != NULL) {
    node = ptr->head_ptr;
    if (DEBUG_LIST) Serial.printf(F("\n>>>> list_push_back / Plug the node to the head of the linked list"));
    while (node->next_ptr != NULL) {
      node = node->next_ptr;
      if (DEBUG_LIST) Serial.printf(F("\n>>>> list_push_back / Look for the last node"));
    }
    if (DEBUG_LIST) Serial.printf(F("\n>>>> list_push_back / Reach the end of the linked list"));
    node->next_ptr = node;
    if (DEBUG_LIST) Serial.printf(F("\n>>>> list_push_back / Add the node to the end of the linked list"));
    node->next_ptr = NULL;
    memcpy(node->data, data, ptr->data_len);
    if (DEBUG_LIST) Serial.printf(F("\n>>>> list_push_back / Blob values copyed to the last node: %d"), ptr->index);
    ptr->index++;
    return;
  } else {
    if (DEBUG_LIST) Serial.printf(F("\n>>>> list_push_back / Start the copy"));
    memcpy(node->data, data, ptr->data_len);
    if (DEBUG_LIST) Serial.printf(F("\n>>>> list_push_back / Blob values copyed to the head node: %d"), ptr->index);
    ptr->head_ptr = node;
    if (DEBUG_LIST) Serial.printf(F("\n>>>> list_push_back / Set the node as head of the linked list"));
    node->next_ptr = NULL;
    if (DEBUG_LIST) Serial.printf(F("\n>>>> list_push_back / Exit"));
    return;
  }
}

/*
  list_pop_front();
  (head)                     (tail)
  node_0 > node_1 > node_2 > node_3
*/
void list_pop_front(list_t* ptr, void* data, node_t* node) {

  node = ptr->head_ptr;

  if (DEBUG_LIST) Serial.printf(F("\n>>>> list_pop_front / Start"));
  if (ptr->head_ptr != NULL) {
    if (DEBUG_LIST) Serial.printf(F("\n>>>> list_pop_front / Find the head node: %d"), ptr->index);
    memcpy(node->data, data, ptr->data_len);
    if (DEBUG_LIST) Serial.printf(F("\n>>>> list_pop_front / Copy the node values to the external node"));
    ptr->head_ptr = node->next_ptr;
    ptr->index--;
    if (DEBUG_LIST) Serial.printf(F("\n>>>> list_pop_front / Change the head node of the linked list to: %d"), ptr->index);
    if (DEBUG_LIST) Serial.printf(F("\n>>>> list_pop_front / Exit"));
    return;
  } else {
    if (DEBUG_LIST) Serial.printf(F("\n>>>> list_pop_front / Linked list is umpty"));
    return;
  }

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
void iterator_get(node_t* lnk, void* data, list_t* ptr) {
  memcpy(data, lnk->data, ptr->data_len);
}

