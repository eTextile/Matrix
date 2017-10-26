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

  Serial.printf(F("\n>>>> Bitmap <<<<\n"));
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
     eTextile.org / list_init();
     Initialise the linked list
*/
void list_init(list_t* ptr, size_t data_len) {
  ptr->head_ptr = NULL;
  ptr->data_len = data_len;
  ptr->index = 0;
  if (DEBUG_LIST || DEBUG_OUTPUT) Serial.printf(F("\n>>>> list_init(); / init done"));
}
/*
    eTextile.org / list_alloc_all();
    (head) node_3 > node_2 > node_1 > node_0 > (next_ptr = NULL)
*/
void list_alloc_all(list_t* ptr, size_t data_len) {
  // Linked list push front
  for (int i = 0; i < MAX_BLOBS; i++) {
    node_t* node = (node_t*) malloc(data_len);
    if (DEBUG_LIST) Serial.printf(F("\n>>>> list_alloc_all / Create a node and allocate it with the blob size: %p"), node);
    if (node == NULL) {
      if (DEBUG_LIST) Serial.printf(F("\n>>>> list_alloc_all / _ALLOC_FAILURE_"));
      return;
    } else {
      memcpy(node->data, 0, ptr->data_len);
      if (ptr->head_ptr != NULL) {
        node->next_ptr = ptr->head_ptr;
        ptr->head_ptr = node;
        if (DEBUG_LIST) Serial.printf(F("\n>>>> list_alloc_all / Add a node to the linked list: %d"), ptr->index);
        ptr->index++;
      } else {
        ptr->head_ptr = node;
        ptr->head_ptr->next_ptr = NULL;
        if (DEBUG_LIST) Serial.printf(F("\n>>>> list_alloc_all / Add the first node to the linked list: %d"), ptr->index);
        ptr->index++;
      }
    }
  }
  return;
}
/*
    eTextile.org / list_get_node_ptr(); / ok!
    (head) node_3 > node_2 > node_1 > node_0 > (next_ptr = NULL)
*/
node_t* list_get_freeNode(list_t* ptr) {
  if (DEBUG_LIST) Serial.printf(F("\n>>>> list_get_freeNode / Start"));
  node_t* node;

  if (ptr->head_ptr != NULL) {
    node = ptr->head_ptr;
    if (DEBUG_LIST) Serial.printf(F("\n>>>> list_get_freeNode / Get the first node address in the linked list: %p"), ptr->head_ptr);
    if (node->next_ptr != NULL) {
      ptr->head_ptr = node->next_ptr;
      if (DEBUG_LIST) Serial.printf(F("\n>>>> list_get_freeNode / Move the head to the next node: %p"), ptr->head_ptr);
    } else {
    }
    ptr->index--;
    if (DEBUG_LIST) Serial.printf(F("\n>>>> list_get_freeNode / Exit"));
    return node;
  } else {
    if (DEBUG_LIST) Serial.printf(F("\n>>>> list_get_freeNode / Linked list is umpty"));
    return NULL;
  }
}
/*
    eTextile.org / list_set_node_ptr(); / ok!
    Copy a node address to a list of nodes address
*/
void list_save_node(list_t* ptr, node_t* node) {

  if (DEBUG_LIST) Serial.printf(F("\n>>>> list_save_node / Start"));

  if (ptr->head_ptr != NULL) {
    node = ptr->head_ptr;
    if (DEBUG_LIST) Serial.printf(F("\n>>>> list_save_node / Use a node to point the linked list head address"));
    ptr->head_ptr = node;
    ptr->index++;
    if (DEBUG_LIST) Serial.printf(F("\n>>>> list_save_node / Use this node as the new linked list head: %d"), ptr->index);
    if (DEBUG_LIST) Serial.printf(F("\n>>>> list_save_node / Exit"));
    return;
  } else {
    if (DEBUG_LIST) Serial.printf(F("\n>>>> list_save_node / Linked list is umpty"));
    node->next_ptr = NULL;
    ptr->head_ptr = node;
    if (DEBUG_LIST) Serial.printf(F("\n>>>> list_save_node / Set the node as head of the linked list"));
    if (DEBUG_LIST) Serial.printf(F("\n>>>> list_save_node / Exit"));
    return;
  }
}

/*
    eTextile.org / list_save_nodes(list_t* dst, list_t* src); / FIXME!!!!!!!!!!!!!!!!!!!!
    Copy all nodes address from src to dst
*/
void list_save_nodes(list_t* dst, list_t* src) {

  node_t* node;

  if (DEBUG_LIST) Serial.printf(F("\n>>>> list_save_nodes / Start"));
  while (src->head_ptr != NULL) {
    node = src->head_ptr;
    if (DEBUG_LIST) Serial.printf(F("\n>>>> list_save_nodes / Get the SRC linked list head address: %p"), src->head_ptr);

    if (dst->head_ptr != NULL) {
      node->next_ptr = dst->head_ptr;
      dst->head_ptr = node;
      if (DEBUG_LIST) Serial.printf(F("\n>>>> list_save_nodes / Add the node to the DST linked list: %p"), dst->head_ptr);
      // ptr->index--;
      // dst->index++;
    } else {
      dst->head_ptr = node;
      dst->head_ptr->next_ptr = NULL;
      if (DEBUG_LIST) Serial.printf(F("\n>>>> list_save_nodes / Add the first node to the DST linked list index: %d"), dst->index);
      // ptr->index--;
      // dst->index++;
    }
    src->head_ptr = node->next_ptr;
    src->head_ptr = NULL;
    if (DEBUG_LIST) Serial.printf(F("\n>>>> list_save_nodes / Move the head to the next node of the SRC linked list"));
  }
  if (DEBUG_LIST) Serial.printf(F("\n>>>> list_save_nodes / Exit"));
}
/*
    eTextile.org / list_push_back();
    (head) node_3 > node_2 > node_1 > node_0 > (next_ptr = NULL)
*/
void list_push_back(list_t* ptr, void* data, node_t* node) {
  if (DEBUG_LIST) Serial.printf(F("\n>>>> list_push_back / Start whis node: %p"), node);
  if (ptr->head_ptr != NULL) {
    node = ptr->head_ptr;
    if (DEBUG_LIST) Serial.printf(F("\n>>>> list_push_back / Get the linked list head address"));
    memcpy(node->data, data, ptr->data_len);
    if (DEBUG_LIST) Serial.printf(F("\n>>>> list_push_back / Blob values copyed to the node: %p"), node);
    while (node->next_ptr != NULL) {
      node = node->next_ptr;
      if (DEBUG_LIST) Serial.printf(F("\n>>>> list_push_back / Look for the last node"));
    }
    if (DEBUG_LIST) Serial.printf(F("\n>>>> list_push_back / Reach the end of the linked list"));
    node->next_ptr = node;
    if (DEBUG_LIST) Serial.printf(F("\n>>>> list_push_back / Add the node to the end of the linked list"));
    node = node->next_ptr;
    if (DEBUG_LIST) Serial.printf(F("\n>>>> list_push_back / Move to the last node"));
    node->next_ptr = NULL;
    if (DEBUG_LIST) Serial.printf(F("\n>>>> list_push_back / Set the next pointer to NULL"));
    ptr->index++;
    return;
  } else {
    memcpy(node->data, data, ptr->data_len);
    if (DEBUG_LIST) Serial.printf(F("\n>>>> list_push_back / Blob values copyed to the node: %d"), ptr->index);
    ptr->head_ptr = node;
    if (DEBUG_LIST) Serial.printf(F("\n>>>> list_push_back / Set the node as head of the linked list"));
    ptr->head_ptr->next_ptr = NULL;
    if (DEBUG_LIST) Serial.printf(F("\n>>>> list_push_back / Exit"));
    ptr->index++;
    return;
  }
}
/*
    eTextile.org / list_pop_front();
    (head) node_3 > node_2 > node_1 > node_0 > (next_ptr = NULL)
*/
void list_pop_front(list_t* ptr, void* data, node_t* node) {

  if (DEBUG_LIST) Serial.printf(F("\n>>>> list_pop_front / Start"));
  if (ptr->head_ptr != NULL) {
    node = ptr->head_ptr;
    if (DEBUG_LIST) Serial.printf(F("\n>>>> list_pop_front / Use a node to point the linked list head address"));
    memcpy(node->data, data, ptr->data_len);
    if (DEBUG_LIST) Serial.printf(F("\n>>>> list_pop_front / Copy the head node values to the node"));
    ptr->head_ptr = node;
    ptr->index--;
    if (DEBUG_LIST) Serial.printf(F("\n>>>> list_pop_front / Change the head node of the linked list to: %d"), ptr->index);
    if (DEBUG_LIST) Serial.printf(F("\n>>>> list_pop_front / Exit"));
    return;
  } else {
    if (DEBUG_LIST) Serial.printf(F("\n>>>> list_pop_front / Linked list is umpty"));
    return;
  }
}

// TODO update it to collect unused nodes
void list_copy(list_t* dst, list_t* src) {
  memcpy(dst, src, sizeof(list_t));
}

size_t list_size(list_t* ptr) {
  return ptr->index;
}

////////////// Iterators //////////////

node_t* iterator_start_from_head(list_t* ptr) {
  return ptr->head_ptr;
}

node_t* iterator_next(node_t* lnk) {
  return lnk->next_ptr;
}

void iterator_get(node_t* lnk, void* data, list_t* ptr) {
  memcpy(data, lnk->data, ptr->data_len);
}

