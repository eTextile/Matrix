/*
  This file is part of the eTextile-matrix-sensor project - http://matrix.eTextile.org
  Copyright (c) 2014-2019 Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "llist.h"

////////////////////////////// linked list  //////////////////////////////

void llist_raz(llist_t* src) {
  src->max_nodes = 0;
  src->tail_ptr = src->head_ptr = NULL;
  src->index = -1;
}

void llist_init(llist_t* dst, blob_t* nodesArray, uint8_t max_nodes) {

  dst->head_ptr = dst->tail_ptr = &nodesArray[0];
  //if (DEBUG_LIST || DEBUG_CCL) Serial.printf(F("\n DEBUG_LIST / llist_init: %d: %p"), 0, &nodesArray[0]);
  dst->index++;
  dst->max_nodes++;

  for (int i = 1; i < max_nodes; i++) {
    nodesArray[i - 1].next_ptr = &nodesArray[i];
    nodesArray[i].next_ptr = NULL;
    dst->tail_ptr = &nodesArray[i];
    //if (DEBUG_LIST || DEBUG_CCL) Serial.printf(F("\n DEBUG_LIST / llist_init: %d: %p"), i, &nodesArray[i]);
    dst->index++;
    dst->max_nodes++;
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

  for (blob_t* blob = ITERATOR_START_FROM_HEAD(src); blob != NULL; blob = ITERATOR_NEXT(blob)) {

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

boolean DEBUG_SORT = false;

// Sort a given linked list
void llist_sort(llist_t* src) {

  blob_t* prev_blob_A = NULL;

  while (1) {

    boolean isSorted = true;

    for (blob_t* blob_A = ITERATOR_START_FROM_HEAD(src); blob_A != NULL; blob_A = ITERATOR_NEXT(blob_A)) {
      if (blob_A->UID > blob_A->next_ptr->UID) {
        isSorted = false;

        blob_t* blob_B = blob_A->next_ptr;

        //if (DEBUG_SORT) Serial.printf("\n DEBUG_SORT / llist_sort / prev_blob_A: %p", prev_blob_A);
        //if (DEBUG_SORT) Serial.printf("\n DEBUG_SORT / llist_sort / blob_A: %p", blob_A);
        //if (DEBUG_SORT) Serial.printf("\n DEBUG_SORT / llist_sort / blob_B: %p", blob_B);

        if (prev_blob_A != NULL) { // Test if prev_ptr_A is the head of linked list
          prev_blob_A->next_ptr = blob_B;
          //if (DEBUG_SORT) Serial.printf("\n DEBUG_SORT / llist_sort / previous blob_A is not the HEAD : %p", prev_blob_A->next_ptr);
        }
        else { // Set curr_ptr_B as NEW HEAD
          src->head_ptr = blob_B;
          //if (DEBUG_SORT) Serial.printf("\n DEBUG_SORT / llist_sort / SET previous blob_A as HEAD : %p", src->head_ptr);
        }
        if (blob_B->next_ptr != NULL) { // Test if current blob_B is not the TAIL of linked list
          blob_A->next_ptr = blob_B->next_ptr;
          //if (DEBUG_SORT) Serial.printf("\n DEBUG_SORT / llist_sort / blob_B is not the TAIL : %p", blob_A->next_ptr);
        }
        else { // Set current blob_A as NEW TAIL
          src->tail_ptr = blob_A;
          blob_A->next_ptr = NULL;
          //if (DEBUG_SORT) Serial.printf("\n DEBUG_SORT / llist_sort / blob_B is the TAIL : %p", src->tail_ptr);
        }
        blob_B->next_ptr = blob_A; // Swap pointers
        //if (DEBUG_SORT) Serial.printf("\n DEBUG_SORT / llist_sort / swaped blob_A: %p", blob_B->next_ptr);
      }
      prev_blob_A = blob_A;
      break;
    }
    if (isSorted) {
      return;
    }
  }
}
