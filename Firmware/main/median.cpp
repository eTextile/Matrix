/*
  median filter (c) 2019 by RainerU
  This code is under the MIT License
  https://github.com/RainerU/medianfilter

  Added multy blob filtering capability
  Copyright (c) 2021 Maurin Donneaud <maurin@etextile.org>
*/

#include "median.h"

median_t blobMedian[MAX_SYNTH] = {0};       // 1D ...

void median(llist_t* blobs_ptr) {

  for (blob_t* blob_ptr = (blob_t*)ITERATOR_START_FROM_HEAD(blobs_ptr); blob_ptr != NULL; blob_ptr = (blob_t*)ITERATOR_NEXT(blob_ptr)) {

    float inputVal = blob_ptr->box.D; // The new value
    float outputVal = inputVal;       // The new value could be the median

    // Initialize all arrays
    if (blob_ptr->state && !blob_ptr->lastState) {
      memset(blobMedian[blob_ptr->UID].zVal, blob_ptr->box.D, MEDIAN_WINDOW * sizeof(uint8_t));
      memset(blobMedian[blob_ptr->UID].zSort, blob_ptr->box.D, MEDIAN_WINDOW * sizeof(uint8_t));
      blobMedian[blob_ptr->UID].index = 0;
    }

    else { // Store new value
      if (++blobMedian[blob_ptr->UID].index >= MEDIAN_WINDOW) blobMedian[blob_ptr->UID].index = 0; // One step forward in ring storage

      float lastZ = blobMedian[blob_ptr->UID].zVal[blobMedian[blob_ptr->UID].index];       // Save last value
      uint8_t lastOrdZ = blobMedian[blob_ptr->UID].zSort[blobMedian[blob_ptr->UID].index]; // Save last order

      blobMedian[blob_ptr->UID].zVal[blobMedian[blob_ptr->UID].index] = blob_ptr->box.D;   // Store new value
      blobMedian[blob_ptr->UID].zSort[blobMedian[blob_ptr->UID].index] = 0;                // Reset order number for new value

      uint8_t index = blobMedian[blob_ptr->UID].index;                                     // Get index
      if (++index >= MEDIAN_WINDOW) index = 0;                                             // loop through array storage

      do {
        if (blob_ptr->box.D <= blobMedian[blob_ptr->UID].zVal[index] &&
            lastOrdZ > blobMedian[blob_ptr->UID].zSort[index]) {
          blobMedian[blob_ptr->UID].zSort[index]++;     // Remove bigger value, add smaller value
        }
        else if (blob_ptr->box.D > blobMedian[blob_ptr->UID].zVal[index] &&
                 lastOrdZ < blobMedian[blob_ptr->UID].zSort[index]) {
          blobMedian[blob_ptr->UID].zSort[index]--;                           // Remove smaller value, add bigger value
        }
        if (blobMedian[blob_ptr->UID].zSort[index] == MEDIAN_POS) {
          outputVal = blobMedian[blob_ptr->UID].zVal[index];                  // blobMedian found
        }
        if (blob_ptr->box.D > blobMedian[blob_ptr->UID].zVal[index]) {
          blobMedian[blob_ptr->UID].zSort[blobMedian[blob_ptr->UID].index]++; // Compute new value order
        }
        if (++index >= MEDIAN_WINDOW) index = 0;                              // Go one step in ring storage
      } while (index != blobMedian[blob_ptr->UID].index);                     // Stop at index position
    }
    blob_ptr->box.D = outputVal;                                              // Replace the value with the computed blobMedian value
    Serial.printf("\n%f ; %f", inputVal, outputVal);
  }
}
