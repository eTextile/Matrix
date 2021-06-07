/*
  This file is part of the E256 - eTextile matrix sensor project - http://matrix.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "llist.h"

void llist_raz(llist_t* llist_ptr) {
  llist_ptr->head_ptr = llist_ptr->tail_ptr = NULL;
}

void* llist_pop_front(llist_t* llist_ptr) {

  lnode_t* node = llist_ptr->head_ptr;

  if (node != NULL) {
    if (llist_ptr->head_ptr != llist_ptr->tail_ptr) {
      llist_ptr->head_ptr = llist_ptr->head_ptr->next_ptr;
      //node->next_ptr = NULL;
      return node;
    }
    else {
      llist_ptr->head_ptr = llist_ptr->tail_ptr = NULL;
      return node;
    }
  }
  else {
    return NULL;
  }
}

void llist_push_front(llist_t* llist_ptr, void* data_ptr) {
  lnode_t* node = (lnode_t*)data_ptr;
  if (llist_ptr->head_ptr != NULL) {
    node->next_ptr = llist_ptr->head_ptr;
    llist_ptr->head_ptr = node;
  }
  else {
    llist_ptr->head_ptr = llist_ptr->tail_ptr = node;
    node->next_ptr = NULL;
  }
}

// linked-list node extractor
void llist_extract_node(llist_t* llist_ptr, void* prevData_ptr, void* data_ptr) {
  lnode_t* nodeToExtract = (lnode_t*)data_ptr;
  lnode_t* prevNode_ptr = (lnode_t*)prevData_ptr;

  if (llist_ptr->head_ptr == llist_ptr->tail_ptr) {
    llist_ptr->head_ptr = llist_ptr->tail_ptr = NULL;
  }
  else {
    if (nodeToExtract == llist_ptr->head_ptr) {
      llist_ptr->head_ptr = llist_ptr->head_ptr->next_ptr;
      //nodeToExtract->next_ptr = NULL;
    }
    else if (nodeToExtract == llist_ptr->tail_ptr) {
      llist_ptr->tail_ptr = prevNode_ptr;
      //prevNode_ptr->next_ptr = NULL;
    }
    else {
      prevNode_ptr->next_ptr = nodeToExtract->next_ptr;
      //nodeToExtract->next_ptr = NULL;
    }
  }
}

void llist_swap_llist(llist_t* llistA_ptr, llist_t* llistB_ptr) {
  lnode_t* tmp_head_ptr = llistA_ptr->head_ptr;
  lnode_t* tmp_tail_ptr = llistA_ptr->tail_ptr;
  llistA_ptr->head_ptr = llistB_ptr->head_ptr;
  llistA_ptr->tail_ptr = llistB_ptr->tail_ptr;
  llistB_ptr->head_ptr = tmp_head_ptr;
  llistB_ptr->tail_ptr = tmp_tail_ptr;
}

void llist_save_nodes(llist_t* dst_ptr, llist_t* src_ptr) {
  if (src_ptr->head_ptr != NULL) {
    dst_ptr->tail_ptr->next_ptr = src_ptr->head_ptr;
    dst_ptr->tail_ptr = src_ptr->tail_ptr;
    src_ptr->tail_ptr = src_ptr->head_ptr = NULL;
  }
}

/*
  // Linked-list sort all nodes by UID
  void llist_sort(llist_t* llist_ptr) {
  lnode_t* nodeA_prev_ptr = NULL;
  while (1) {
    boolean isSorted = true;  lnode_t* node = llist_ptr->head_ptr;

    for (lnode_t* nodeA = ITERATOR_START_FROM_HEAD(llist_ptr); nodeA != NULL; nodeA = ITERATOR_NEXT(nodeA)) {
      if (nodeA->UID > nodeA->next_ptr->UID) {
        isSorted = false;
        lnode_t* node_B = node_A->next_ptr;
        //if (DEBUG_SORT) Serial.printf("\n DEBUG_SORT / llist_sort / prev_blob_A: %p", prev_blob_A);
        //if (DEBUG_SORT) Serial.printf("\n DEBUG_SORT / llist_sort / blob_A: %p", blob_A);
        //if (DEBUG_SORT) Serial.printf("\n DEBUG_SORT / llist_sort / blob_B: %p", blob_B);
        if (nodeA_prev_ptr != NULL) { // Test if prev_ptr_A is the head of linked list
          nodeA_prev_ptr->next_ptr = node_B;
          //if (DEBUG_SORT) Serial.printf("\n DEBUG_SORT / llist_sort / previous blob_A is not the HEAD : %p", prev_blob_A->next_ptr);
        }
        else { // Set curr_ptr_B as NEW HEAD
          llist_ptr->head_ptr = node_B;
          //if (DEBUG_SORT) Serial.printf("\n DEBUG_SORT / llist_sort / SET previous blob_A as HEAD : %p", src->head_ptr);
        }
        if (node_B->next_ptr != NULL) { // Test if current blob_B is not the TAIL of linked list
          node_A->next_ptr = node_B->next_ptr;
          //if (DEBUG_SORT) Serial.printf("\n DEBUG_SORT / llist_sort / blob_B is not the TAIL : %p", blob_A->next_ptr);
        }
        else { // Set current blob_A as NEW TAIL
          llist_ptr->tail_ptr = node_A;
          node_A->next_ptr = NULL;
          //if (DEBUG_SORT) Serial.printf("\n DEBUG_SORT / llist_sort / blob_B is the TAIL : %p", src->tail_ptr);
        }
        node_B->next_ptr = node_A; // Swap pointers
        //if (DEBUG_SORT) Serial.printf("\n DEBUG_SORT / llist_sort / swaped blob_A: %p", blob_B->next_ptr);
      }
      nodeA_prev_ptr = node_A;
      break;
    }
    if (isSorted) {
      return;
    }
  }
  }
*/
