#include "llist.h"
#include "blob.h"

#define MAX_NODES             40    // Set the maximum nodes number

//uint8_t UID;
//state_t state;
//point_t centroid;
//bbox_t box;
//struct blob* next_ptr;

blob_t blobs[5] = {
  {1, FREE, 7, 10, 10, 20, 40, 0},
  {2, FREE, 8, 50, 10, 20, 40, 0},
  {3, FREE, 9, 100, 10, 20, 40, 0},
  {4, FREE, 10, 40, 10, 20, 40, 0},
  {5, FREE, 11, 33, 10, 20, 40, 0},
};


void setup() {
  // Blobs list init
  llist_raz(&freeBlobs);
  llist_init(&freeBlobs, blobArray, MAX_NODES); // Add 40 nodes in the freeBlobs linked list
  llist_raz(&blobs);
  llist_raz(&outputBlobs);
}

void loop() {

  find_blobs(
    &freeBlobs,            // list_t
    &blobs,                // list_t
    &outputBlobs           // list_t
  );

}
