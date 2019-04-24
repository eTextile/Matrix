/*
  This file is part of the eTextile-matrix-sensor project - http://matrix.eTextile.org
  Copyright (c) 2014-2018 Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "llist.h"

////////////////////////////// linked list  //////////////////////////////

void llist_raz(llist_t* ptr) {
  ptr->tail_ptr = ptr->head_ptr = NULL;
  ptr->index = -1;
}

void llist_init(llist_t* dst, blob_t* nodesArray, const uint8_t max_nodes) {

  dst->max_nodes = max_nodes;

  dst->head_ptr = dst->tail_ptr = &nodesArray[0];
  //if (DEBUG_LIST || DEBUG_CCL) Serial.printf(F("\n DEBUG_LIST / llist_init: %d: %p"), 0, &nodesArray[0]);
  dst->index++;

  for (int i = 1; i < dst->max_nodes; i++) {
    nodesArray[i - 1].next_ptr = &nodesArray[i];
    nodesArray[i].next_ptr = NULL;
    dst->tail_ptr = &nodesArray[i];
    //if (DEBUG_LIST || DEBUG_CCL) Serial.printf(F("\n DEBUG_LIST / llist_init: %d: %p"), i, &nodesArray[i]);
    dst->index++;
  }
}

blob_t* llist_pop_front(llist_t* src) {

  if (src->index > -1) {
    blob_t* node = src->head_ptr;
    if (src->index > 0) {
      src->head_ptr = src->head_ptr->next_ptr;
    } else {
      src->head_ptr = src->tail_ptr = NULL;
    }
    node->next_ptr = NULL;
    src->index--;
    return node;
  } else {
    //if (DEBUG_LIST) Serial.printf(F("\n DEBUG_LIST / list_pop_front / ERROR : SRC list is umpty!"));
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
  node->next_ptr = NULL;
  dst->index++;
}

// Remove a blob in a linked list
void llist_remove_blob(llist_t* src, blob_t* blobSuppr) {

  blob_t* prevBlob = NULL;
  //if (DEBUG_LIST) Serial.printf(F("\n DEBUG_LIST / list_remove_blob / Blob to remove: %p"), blobSuppr);

  for (blob_t* blob = iterator_start_from_head(src); blob != NULL; blob = iterator_next(blob)) {

    if (blob == blobSuppr) {
      //if (DEBUG_LIST) Serial.printf(F("\n DEBUG_LIST / list_remove_blob / Blob: %p is found"), blob);

      if (src->index == 0) {
        //if (DEBUG_LIST) Serial.printf(F("\n DEBUG_LIST / list_remove_blob / The blob: %p is the first & last in the linked list"), blobSuppr);
        src->head_ptr = src->tail_ptr = NULL;
        src->index--;
        return;
      }
      else if (blob->next_ptr == NULL) {
        //if (DEBUG_LIST) Serial.printf(F("\n DEBUG_LIST / list_remove_blob / The blob: %p is the tail of the linked list"), blobSuppr);
        prevBlob->next_ptr = NULL;
        src->tail_ptr = prevBlob;
        src->index--;
        return;
      }
      else if (blob == src->head_ptr) {
        //if (DEBUG_LIST) Serial.printf(F("\n DEBUG_LIST / list_remove_blob / The blob: %p is the hard of the linked list"), blobSuppr);
        src->head_ptr = src->head_ptr->next_ptr;
        src->index--;
        return;
      }
      else {
        //if (DEBUG_LIST) Serial.printf(F("\n DEBUG_LIST / list_remove_blob / The blob: %p is somewear else in the linked list"), blobSuppr);
        prevBlob->next_ptr = blob->next_ptr;
        src->index--;
        return;
      }
    }
    prevBlob = blob;
  }
  //if (DEBUG_LIST) Serial.printf(F("\n DEBUG_LIST / list_remove_blob / ERROR / Blob not found"));
}

void llist_save_blobs(llist_t* dst, llist_t* src) {

  //if (DEBUG_LIST) Serial.printf(F("\n DEBUG_LIST / list_save_blobs / START"));
  blob_t* blob = NULL;

  while (src->index > -1) {
    // SRC pop front
    blob = src->head_ptr;
    if (src->index > 0) {
      //if (DEBUG_LIST) Serial.printf(F("\n DEBUG_LIST / list_save_blobs / SRC pop a blob in the list: %p"), blob);
      src->head_ptr = src->head_ptr->next_ptr;
      //if (DEBUG_LIST) Serial.printf(F("\n DEBUG_LIST / list_save_blobs / SRC Move the list hed to next_ptr: %p"), src->head_ptr);
    } else { // src->index == 0
      src->tail_ptr = src->head_ptr = NULL;
      //if (DEBUG_LIST) Serial.printf(F("\n DEBUG_LIST / list_save_blobs / SRC pop the last blob in the list: %p"), blob);
    }
    src->index--;
    //if (DEBUG_LIST) Serial.printf(F("\n DEBUG_LIST / list_save_blobs / SRC set index to: %d"), src->index);

    // DST push back
    if (dst->index > -1) {
      dst->tail_ptr->next_ptr = blob;
      //if (DEBUG_LIST) Serial.printf(F("\n DEBUG_LIST / list_save_blobs / DST add the blob to the list: %p"), blob);
      dst->tail_ptr = blob;
    } else { // dst->index == -1
      dst->head_ptr = dst->tail_ptr = blob;
      //if (DEBUG_LIST) Serial.printf(F("\n DEBUG_LIST / list_save_blobs / DST add the first blob to the list"));
    }
    dst->tail_ptr->next_ptr = NULL; // Same than blob->next_ptr = NULL;
    dst->index++;
    //if (DEBUG_LIST) Serial.printf(F("\n DEBUG_LIST / list_save_blobs / DST set index to: %d"), dst->index);
  }
  //if (DEBUG_LIST) Serial.printf(F("\n DEBUG_LIST / list_save_blobs / SRC linked list is umpty!"));
}

// Bubble sort a given linked list !!IN PROGRESS!!
void llist_bubbleSort(llist_t* src_ptr) {

  // Checking if there is more than one blob in the linked list (-1 is empty)
  if (src_ptr->index < 1) {
    return;
  }
  while (1) {
    boolean isSorted = true;
    for (blob_t* blob = iterator_start_from_head(src_ptr); blob != NULL; blob = iterator_next(blob)) {
      if (blob->UID > blob->next_ptr->UID) {
        isSorted = false;
        llist_swap(src_ptr, blob);
        break;
      }
    }
    if (isSorted) {
      break;
    }
  }
}

// Swap two nodes in a linked list (zero copy)
void llist_swap(llist_t* src_ptr, blob_t* ptr) {

  // Search for ptr_A (keep track of prev_ptr_A and curr_ptr_A)
  blob_t* prev_ptr_A = NULL;
  blob_t* curr_ptr_A = src_ptr->head_ptr;

  while (curr_ptr_A != ptr) {
    prev_ptr_A = curr_ptr_A;
    curr_ptr_A = curr_ptr_A->next_ptr;
  }

  // Set curr_ptr_B
  blob_t* curr_ptr_B = curr_ptr_A->next_ptr;

  // Test if curr_ptr_A is not the head of linked list
  if (prev_ptr_A != NULL) {
    prev_ptr_A->next_ptr = curr_ptr_B;
  }
  else { // Else make curr_ptr_B as new head
    src_ptr->head_ptr = curr_ptr_B;
  }

  // Test if curr_ptr_B is not the tail of linked list
  if (curr_ptr_B->next_ptr != NULL) {
    curr_ptr_A->next_ptr = curr_ptr_B->next_ptr;
  }
  else { // Set curr_ptr_A as new tail
    src_ptr->tail_ptr = curr_ptr_A;
    curr_ptr_A->next_ptr = NULL;
  }

  // Swap pointers
  curr_ptr_B->next_ptr = curr_ptr_A;

}


////////////////////////////// Linked list iterators //////////////////////////////

blob_t* iterator_start_from_head(llist_t* src) {
  return src->head_ptr;
}

blob_t* iterator_next(blob_t* src) {
  return src->next_ptr;
}
int8_t llist_size(llist_t* ptr) {
  return ptr->index;
}
