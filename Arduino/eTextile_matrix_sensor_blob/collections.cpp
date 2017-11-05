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
/*
    Linked List fonctions
    The linked list is a sigel direction (from head to tail)
    (head) node_3 > node_2 > node_1 > node_0 > (tail)
*/
/*
    eTextile.org / list_init();
    Initialise the linked list
*/
void list_init(list_t* ptr, size_t data_len) {
  ptr->head_ptr = NULL;
  ptr->data_len = data_len;
  ptr->index = 0;
  if (DEBUG_LIST) Serial.printf(F("\n>>>> list_init(); / init done"));
}
/*
    eTextile.org / list_alloc_all();
    Linked list - push front
*/
void list_alloc_all(list_t* dst, size_t data_len) {
  if (DEBUG_LIST) Serial.printf(F("\n>>>>>>>> list_alloc_all / Start"));

  for (int i = 0; i < MAX_BLOBS; i++) {
    node_t* node = (node_t*) malloc(data_len);
    if (DEBUG_LIST) Serial.printf(F("\n>>>> list_alloc_all / Create and allocate the node: %p"), node);
    if (node == NULL) {
      if (DEBUG_LIST) Serial.printf(F("\n>>>> list_alloc_all / _ALLOC_FAILURE_"));
    } else {
      memcpy(node->data, 0, data_len);
      if (dst->head_ptr != NULL) {
        node->next_ptr = dst->head_ptr;
        dst->head_ptr = node;
        if (DEBUG_LIST) Serial.printf(F("\n>>>> list_alloc_all / Add a node to the linked list: %d"), dst->index);
        dst->index++;
      } else {
        node->next_ptr = NULL;
        dst->head_ptr = node;
        if (DEBUG_LIST) Serial.printf(F("\n>>>> list_alloc_all / Add the first node to the linked list: %d"), dst->index);
        dst->index++;
      }
    }
  }
}
/*
    eTextile.org / list_get_freeNode();
    Return the adress/pointer of the first node in the freeNode linked list
*/
node_t* list_get_freeNode(list_t* src) {
  if (DEBUG_LIST) Serial.printf(F("\n>>>>>>>> list_get_freeNode / Start"));
  node_t* node;

  if (src->head_ptr != NULL) {
    node = src->head_ptr;
    if (DEBUG_LIST) Serial.printf(F("\n>>>> list_get_freeNode / Get the first node address in the linked list: %p"), src->head_ptr);
    if (node->next_ptr != NULL) {
      src->head_ptr = node->next_ptr;
      if (DEBUG_LIST) Serial.printf(F("\n>>>> list_get_freeNode / Move the head to the next node: %p"), src->head_ptr);
      src->index--;
      if (DEBUG_LIST) Serial.printf(F("\n>>>> list_get_freeNode / Exit"));
      return node;
    } else {
      src->head_ptr = NULL;
      if (DEBUG_LIST) Serial.printf(F("\n>>>> list_get_freeNode / Set the liked list head pointer to NULL: %p"), src->head_ptr);
      src->index--;
      if (DEBUG_LIST) Serial.printf(F("\n>>>> list_get_freeNode / Exit"));
      return node;
    }
  } else {
    if (DEBUG_LIST) Serial.printf(F("\n>>>> list_get_freeNode / Linked list is umpty"));
    return NULL;
  }
}
/*
    eTextile.org / list_get_node(); / V0.1 ///////////////////////////////////////////// FIXME!
    Return the adress/pointer of a linked list node
    Do not remove the node from the SRC linked list
*/
void list_get_node(list_t* src, size_t index, void* data) {
  if (DEBUG_LIST) Serial.printf(F("\n>>>>>>>> list_get_node / Start"));

  node_t* tmpNode;
  
  if (src->head_ptr != NULL) {
    tmpNode = src->head_ptr;
    if (DEBUG_LIST) Serial.printf(F("\n>>>> list_get_node / Get the linked list head adress: %p"), src->head_ptr);
    for (int i = 0; i < index; i++) {
      if (tmpNode->next_ptr != NULL) {
        tmpNode = tmpNode->next_ptr;
        if (DEBUG_LIST) Serial.printf(F("\n>>>> list_get_node / Get the next node address: %p"), src->head_ptr);
      }
    }
    memcpy(data, tmpNode->data, src->data_len);
    if (DEBUG_LIST) Serial.printf(F("\n>>>> list_get_node / Return the node data values to the extern node"));
  } // src->head_ptr == NULL
}
/*
    eTextile.org / list_remove_node(); / V0.1
    Remove a node from a linked list
    Return the adress/pointer of the removed linked list node
*/
void list_remove_node(list_t* src, node_t* node, size_t index) {
  if (DEBUG_LIST) Serial.printf(F("\n>>>>>>>> list_remove_node / Start with node: %d"), index);

  node_t* prevNode = NULL;

  if (src->head_ptr != NULL) {
    node = src->head_ptr;
    if (DEBUG_LIST) Serial.printf(F("\n>>>> list_remove_node / Get the head adress of the linked list: %p"), src->head_ptr);
    
    for (int i = 0; i <= index; i++) {
      if (node->next_ptr != NULL) {
        prevNode = node;
        if (DEBUG_LIST) Serial.printf(F("\n>>>> list_remove_node / Save the previus node address: %p"), prevNode);
        
        node = node->next_ptr;
        if (DEBUG_LIST) Serial.printf(F("\n>>>> list_remove_node / Get the next node address: %p"), node);
        
      } else { // node->next_ptr == NULL
        src->head_ptr = NULL;
        if (DEBUG_LIST) Serial.printf(F("\n>>>> list_remove_node / Set SRC linked list head adress is umpty: %p"), src->head_ptr);
      }
    }
    prevNode->next_ptr = node->next_ptr; // FIXME!! si ce'est le dernier nede, pas possible de connecter au suivant!
    if (DEBUG_LIST) Serial.printf(F("\n>>>> list_remove_node / Connect the next & previous nodes of the removed node: %p -> %p"), prevNode->next_ptr, node->next_ptr);
    src->index--;
    if (DEBUG_LIST) Serial.printf(F("\n>>>> list_remove_node / Change the index of the SRC linked list: %d"), src->index);
  }
}
/*
    eTextile.org / list_save_node(); / V1.0
    Save the address/ponter of a node to a linked list of nodes
*/
void list_save_node(list_t* dst, node_t* node) {
  if (DEBUG_LIST) Serial.printf(F("\n>>>>>>>> list_save_node / Start"));

  if (dst->head_ptr != NULL) {
    node->next_ptr = dst->head_ptr;
    if (DEBUG_LIST) Serial.printf(F("\n>>>> list_save_node / Set the node->next_ptr to point the DST linked list head address: %p"), dst->head_ptr);
    dst->head_ptr = node;
    if (DEBUG_LIST) Serial.printf(F("\n>>>> list_save_node / Move the DST linked list head address to the new node: %p"), dst->head_ptr);
    dst->index++;
    if (DEBUG_LIST) Serial.printf(F("\n>>>> list_save_node / Exit"));
  } else {
    if (DEBUG_LIST) Serial.printf(F("\n>>>> list_save_node / Linked list is umpty"));
    node->next_ptr = NULL;
    dst->head_ptr = node;
    if (DEBUG_LIST) Serial.printf(F("\n>>>> list_save_node / Set the node as head of the linked list"));
    if (DEBUG_LIST) Serial.printf(F("\n>>>> list_save_node / Exit"));
  }
}
/*
    eTextile.org / list_push_back(); / V1.1
    Add a node addres/pointer to the end of a linked list
*/
void list_push_back(list_t* dst, node_t* node, void* data) {
  if (DEBUG_LIST) Serial.printf(F("\n>>>>>>>> list_push_back / Start"));

  node_t* tmpNode = node;
  if (DEBUG_LIST) Serial.printf(F("\n>>>> list_push_back / A-create a tmpNode to hold the node address: %p"), tmpNode);
  memcpy(tmpNode->data, data, dst->data_len);
  if (DEBUG_LIST) Serial.printf(F("\n>>>> list_push_back / A-copy the blob values to the tmpNode: %p"), tmpNode);
  tmpNode->next_ptr = NULL;
  if (DEBUG_LIST) Serial.printf(F("\n>>>> list_push_back / A-set the tmpNode next_ptr to NULL: %p"), tmpNode->next_ptr);
  if (dst->head_ptr != NULL) {
    node = dst->head_ptr;
    if (DEBUG_LIST) Serial.printf(F("\n>>>> list_push_back / Get the DST linked list head address"));
    while (node->next_ptr != NULL) {
      node = node->next_ptr;
      if (DEBUG_LIST) Serial.printf(F("\n>>>> list_push_back / Look for the last node: %p"), node->next_ptr);
    } // node->next_ptr == NULL
    if (DEBUG_LIST) Serial.printf(F("\n>>>> list_push_back / Reach the end of the linked list"));
    node->next_ptr = tmpNode;
    if (DEBUG_LIST) Serial.printf(F("\n>>>> list_push_back / Add the node to the end of the linked list: %p"), tmpNode);
    dst->index++;
    return;
  } else { // dst->head_ptr == NULL
    dst->head_ptr = tmpNode;
    if (DEBUG_LIST) Serial.printf(F("\n>>>> list_push_back / Set the node as head of the linked list: %p"), tmpNode);
    dst->index++;
    if (DEBUG_LIST) Serial.printf(F("\n>>>> list_push_back / Exit"));
    return;
  }
}
/*
    eTextile.org / list_pop_front(); / V1.1
    Read & remove the first node of a linked list
*/
void list_pop_front(list_t* src, void* data, node_t* node ) {
  if (DEBUG_LIST) Serial.printf(F("\n>>>>>>>> list_pop_front / Start"));

  if (src->head_ptr != NULL) {
    node = src->head_ptr;
    if (DEBUG_LIST) Serial.printf(F("\n>>>> list_pop_front / Use a node to point the linked list head address"));
    memcpy(node->data, data, src->data_len);
    if (DEBUG_LIST) Serial.printf(F("\n>>>> list_pop_front / Copy the head node values to the node"));
    if (node->next_ptr != NULL) {
      src->head_ptr = node->next_ptr;
      if (DEBUG_LIST) Serial.printf(F("\n>>>> list_pop_front / Change the head node of the linked list to: %d"), src->index);
      src->index--;
      if (DEBUG_LIST) Serial.printf(F("\n>>>> list_pop_front / Exit"));
      // return node;
    } else { // src->head_ptr == NULL
      src->head_ptr = NULL;
      if (DEBUG_LIST) Serial.printf(F("\n>>>> list_pop_front / Change the head node of the linked list to: %d"), src->index);
      src->index--;
      if (DEBUG_LIST) Serial.printf(F("\n>>>> list_pop_front / Exit"));
      // return node;
    }
  }
  if (DEBUG_LIST) Serial.printf(F("\n>>>> list_pop_front / Linked list is umpty"));
  // return NULL;
}

void list_memcpy(list_t* dst, list_t* src) {
  memcpy(dst, src, sizeof(list_t));
}
/*
    eTextile.org / list_save_nodes(); / V1.1
    Copy all nodes address from src to dst
*/
void list_copy(list_t* dst, list_t* src) {
  if (DEBUG_LIST) Serial.printf(F("\n>>>>>>>> list_copy / Start DST: %d <- SRC: %d"), dst->index, src->index);

  node_t* node;

  while (src->head_ptr != NULL) {
    node = src->head_ptr;
    if (DEBUG_LIST) Serial.printf(F("\n>>>> list_copy / SRC: get the linked list head address: %p"), src->head_ptr);
    if (dst->head_ptr != NULL) {
      if (node->next_ptr != NULL) {
        src->head_ptr = node->next_ptr ;
        if (DEBUG_LIST) Serial.printf(F("\n>>>> list_copy / SRC-A: change the head node of the linked list to: %p"), src->head_ptr);
        node->next_ptr = dst->head_ptr;
        if (DEBUG_LIST) Serial.printf(F("\n>>>> list_copy / DST-A: plug the node next_ptr to the linked list: %p"), node->next_ptr);
        dst->head_ptr = node;
        if (DEBUG_LIST) Serial.printf(F("\n>>>> list_copy / DST-A: change the head node of the linked list to: %p"), dst->head_ptr);
        dst->index++;
        src->index--;
      } else { // node->next_ptr == NULL
        node->next_ptr = dst->head_ptr;
        if (DEBUG_LIST) Serial.printf(F("\n>>>> list_copy / DST-B: plug the node next_ptr to the linked list: %p"), node->next_ptr);
        dst->head_ptr = node;
        if (DEBUG_LIST) Serial.printf(F("\n>>>> list_copy / DST-B: change the head node of the linked list to: %p"), dst->head_ptr);
        src->head_ptr = NULL;
        if (DEBUG_LIST) Serial.printf(F("\n>>>> list_copy / SRC-B: clear the head pointer: %p"), src->head_ptr);
        dst->index++;
        src->index--;
        if (DEBUG_LIST) Serial.printf(F("\n>>>> list_copy / Exit"));
        return;
      }
    } else {
      src->head_ptr = node->next_ptr ;
      if (DEBUG_LIST) Serial.printf(F("\n>>>> list_copy / SRC-C: change the head node of the linked list to: %p"), src->head_ptr);
      node->next_ptr = NULL;
      if (DEBUG_LIST) Serial.printf(F("\n>>>> list_copy / DST-C: set the next_ptr of the node to NULL: %p"), node->next_ptr);
      dst->head_ptr = node;
      if (DEBUG_LIST) Serial.printf(F("\n>>>> list_copy / DST-C: add the first node to the linked list: %p"), dst->head_ptr);
      src->index--;
      dst->index++;
    }
  } // src->head_ptr == NULL
  if (DEBUG_LIST) Serial.printf(F("\n>>>> list_copy / SRC: no more node in the linked list & Exit"));
  // return;
}

////////////// Linked list iterators //////////////
node_t* iterator_start_from_head(list_t* src) {
  return src->head_ptr;
}
void iterator_get(list_t* src, node_t* node, void* data) { // Can be optimised !?
  memcpy(data, node->data, src->data_len);
}
node_t* iterator_next(node_t* src) {
  return src->next_ptr;
}
size_t list_size(list_t* ptr) {
  return ptr->index;
}
