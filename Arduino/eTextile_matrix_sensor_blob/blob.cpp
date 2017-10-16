/* This file is part of the OpenMV project.
   Copyright (c) 2013-2017 Ibrahim Abdelkader <iabdalkader@openmv.io> & Kwabena W. Agyeman <kwagyeman@openmv.io>
   This work is licensed under the MIT license, see the file LICENSE for details.
*/

#include <Arduino.h>
#include "blob.h"
#include "collections.h"
#include "config.h"

////////////// Rectangle Stuff //////////////

boolean rectangle_overlap(rectangle_t *ptr0, rectangle_t *ptr1) {
  int x0 = ptr0->x;
  int y0 = ptr0->y;
  int w0 = ptr0->w;
  int h0 = ptr0->h;
  int x1 = ptr1->x;
  int y1 = ptr1->y;
  int w1 = ptr1->w;
  int h1 = ptr1->h;
  return (x0 < (x1 + w1)) && (y0 < (y1 + h1)) && (x1 < (x0 + w0)) && (y1 < (y0 + h0));
}

void rectangle_united(rectangle_t *src, rectangle_t *dst) {
  int leftX = IM_MIN(dst->x, src->x);
  int topY = IM_MIN(dst->y, src->y);
  int rightX = IM_MAX(dst->x + dst->w, src->x + src->w);
  int bottomY = IM_MAX(dst->y + dst->h, src->y + src->h);
  dst->x = leftX;
  dst->y = topY;
  dst->w = rightX - leftX;
  dst->h = bottomY - topY;
}

void find_blobs(
  image_t *input_ptr,
  list_t *output_ptr,
  node_t *tmpNode_ptr,
  char *bitmap_ptr,
  const int rows,
  const int cols,
  const int pixelThreshold,
  const int minBlobSize,
  unsigned int minBlobPix,
  boolean merge,
  int margin
) {
  size_t row_index = 0;
  size_t code = 0;

  uint8_t row;
  uint8_t *row_ptr;
  row_ptr = &row;

  lifo_t lifo;
  size_t lifoLen;

  lifo_alloc_all(&lifo, &lifoLen, sizeof(xylf_t));

  if (DEBUG_BLOB) Serial.printf("\n>>>> A-Lifo len: %d", lifoLen);
  if (DEBUG_BLOB) Serial.printf("\n>>>> A-Lifo size: %d", lifo_size(&lifo));

  for (int posY = 0; posY < rows; posY++) {

    row_ptr = FRAME_ROW_PTR(input_ptr, posY);         // Return pointer to image curent row
    row_index = BITMAP_ROW_INDEX(input_ptr, posY);    // Return bitmap curent row

    if (DEBUG_BLOB) Serial.printf("\nROW index: %d ", row_index / rows);

    for (int posX = 0; posX < cols; posX++) {
      if (DEBUG_INPUT) Serial.printf("%d ", GET_FRAME_PIXEL(row_ptr, posX)); // <<<< INPUT VALUES!!

      if (!bitmap_bit_get(bitmap_ptr, BITMAP_INDEX(row_index, posX)) &&
          PIXEL_THRESHOLD(GET_FRAME_PIXEL(row_ptr, posX), pixelThreshold)) {

        if (DEBUG_INPUT) Serial.printf("\n>>>>>>>>>>>>>>>>>>>> INIT BLOB!\n");

        int old_x = posX;
        int old_y = posY;

        int blob_x1 = posX;
        int blob_y1 = posY;
        int blob_x2 = posX;
        int blob_y2 = posY;
        int blob_pixels = 0;
        int blob_cx = 0;
        int blob_cy = 0;

        ////////// Scanline flood fill algorithm //////////

        for (;;) {

          int left = posX;
          int right = posX;

          row_ptr = FRAME_ROW_PTR(input_ptr, posY);       // Return pointer to the curent image row
          row_index = BITMAP_ROW_INDEX(input_ptr, posY);  // Return bitmap curent row

          while ((left > 0) &&
                 !bitmap_bit_get(bitmap_ptr, BITMAP_INDEX(row_index, left - 1)) &&
                 PIXEL_THRESHOLD(GET_FRAME_PIXEL(row_ptr, left - 1), pixelThreshold)) {
            left--;
            if (DEBUG_BLOB) Serial.printf("left_%d ", left);
          }
          while (right < (cols - 1) &&
                 !bitmap_bit_get(bitmap_ptr, BITMAP_INDEX(row_index, right + 1)) &&
                 PIXEL_THRESHOLD(GET_FRAME_PIXEL(row_ptr, right + 1), pixelThreshold)) {
            right++;
            if (DEBUG_BLOB) Serial.printf("right_%d ", right);
          }

          blob_x1 = IM_MIN(blob_x1, left);
          blob_y1 = IM_MIN(blob_y1, posY);
          blob_x2 = IM_MAX(blob_x2, right);
          blob_y2 = IM_MAX(blob_y2, posY);

          for (int i = left; i <= right; i++) {
            bitmap_bit_set(bitmap_ptr, BITMAP_INDEX(row_index, i));
            if (DEBUG_BLOB) Serial.printf("\n>>>> Bitmap bit set: %d", BITMAP_INDEX(row_index, i));
            blob_pixels += 1;
            blob_cx += i;
            blob_cy += posY;
          }
          // bitmap_print(bitmap_ptr);

          if (DEBUG_BLOB) Serial.printf("\n>>>> Blob pixels: %d", blob_pixels);

          boolean break_out = false;

          for (;;) {

            if (lifo_size(&lifo) < lifoLen) {

              if (DEBUG_BLOB) Serial.printf("\n>>>> B-Lifo len: %d", lifoLen );
              if (DEBUG_BLOB) Serial.printf("\n>>>> B-Lifo size: %d", lifo_size(&lifo) );

              if (posY > 0) {

                row_ptr = FRAME_ROW_PTR(input_ptr, posY + 1);
                row_index = BITMAP_ROW_INDEX(input_ptr, posY + 1);

                boolean recurse = false;

                for (int i = left; i <= right; i++) {
                  if (!bitmap_bit_get(bitmap_ptr, BITMAP_INDEX(row_index, i)) &&
                      PIXEL_THRESHOLD(GET_FRAME_PIXEL(row_ptr, i), pixelThreshold)) {

                    xylf_t context;
                    context.x = posX;
                    context.y = posY;
                    context.l = left;
                    context.r = right;
                    if (DEBUG_BLOB) Serial.printf("\n>>>> A-Lifo add");
                    lifo_enqueue(&lifo, &context);
                    posX = i;
                    posY--;
                    recurse = true;
                    break;
                  }
                }
                if (recurse) {
                  break;
                }
              }
              if (DEBUG_BLOB) Serial.printf("\n>>>> Break 1");

              if (posY < (rows - 1)) {

                row_ptr = FRAME_ROW_PTR(input_ptr, posY + 1);
                row_index = BITMAP_ROW_INDEX(input_ptr, posY + 1);

                boolean recurse = false;

                for (int i = left; i <= right; i++) {
                  if (!bitmap_bit_get(bitmap_ptr, BITMAP_INDEX(row_index, i)) &&
                      PIXEL_THRESHOLD(GET_FRAME_PIXEL(row_ptr, i), pixelThreshold)) {

                    xylf_t context;
                    context.x = posX;
                    context.y = posY;
                    context.l = left;
                    context.r = right;
                    if (DEBUG_BLOB) Serial.printf("\n>>>> B-Lifo add");
                    lifo_enqueue(&lifo, &context);
                    posX = i;
                    posY++;
                    recurse = true;
                    break;
                  }
                }
                if (recurse) {
                  break;
                }
              }
              if (DEBUG_BLOB) Serial.printf("\n>>>> Break 2");
            }
            if (!lifo_size(&lifo)) { // DO NOT GET IN THIS CONDITION !? <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
              if (DEBUG_BLOB) Serial.printf("Lifo umpty: %d -> break_out = true", lifo_size(&lifo));
              break_out = true;
              break;
            }

            xylf_t context;
            if (DEBUG_BLOB) Serial.printf("\n>>>> Lifo remove");
            lifo_dequeue(&lifo, &context);
            posX = context.x;
            posY = context.y;
            left = context.l;
            right = context.r;
          }

          if (break_out) {
            break;
          }
        }
        if (DEBUG_BLOB) Serial.print(">>>> END of scanline flood fill algorithm");

        int mx = blob_cx / blob_pixels; // x centroid
        int my = blob_cy / blob_pixels; // y centroid

        blob_t blob;

        blob.rect.x = blob_x1;
        blob.rect.y = blob_y1;
        blob.rect.w = blob_x2 - blob_x1;
        blob.rect.h = blob_y2 - blob_y1;
        blob.pixels = blob_pixels;
        blob.centroid.x = mx;
        blob.centroid.y = my;
        blob.code = 1 << code;
        blob.count = 1;

        if (((blob.rect.w * blob.rect.h) >= minBlobSize) && (blob.pixels >= minBlobPix)) {
          list_push_back(output_ptr, &blob, tmpNode_ptr);
          if (DEBUG_BLOB) Serial.printf("Adding blob to the list: %d", list_size(output_ptr));
        }

        posX = old_x;
        posY = old_y;
      }
    }
  }
  code++;
  if (DEBUG_BLOB) Serial.printf("\n>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> End of scann: %d", code);

  if (DEBUG_BLOB) Serial.printf("\n>>>> Bitmap print");
  bitmap_print(bitmap_ptr);

  if (DEBUG_BLOB) Serial.printf("\n>>>> Lifo free");
  lifo_free(&lifo);

  if (DEBUG_BLOB) Serial.printf("\n>>>> Bitmap print");
  bitmap_print(bitmap_ptr);

  if (DEBUG_BLOB) Serial.printf("\n>>>> Bitmap clear");
  bitmap_clear(bitmap_ptr);

  if (DEBUG_BLOB) Serial.printf("\n>>>> Bitmap print");
  bitmap_print(bitmap_ptr);

  if (merge) {
    for (;;) {
      boolean merge_occured = false;

      list_t out_temp;
      list_init(&out_temp, sizeof(blob_t));

      while (list_size(output_ptr)) {

        blob_t blob;

        list_pop_front(output_ptr, &blob, tmpNode_ptr);
        if (DEBUG_BLOB) Serial.printf("Remove blob from the list: %d", list_size(output_ptr));

        for (size_t k = 0, l = list_size(output_ptr); k < l; k++) {

          blob_t tmp_blob;

          list_pop_front(output_ptr, &tmp_blob, tmpNode_ptr);

          rectangle_t temp;

          temp.x = IM_MAX(IM_MIN(tmp_blob.rect.x - margin, INT16_MAX), INT16_MIN); // INT16_MAX - INT16_MIN !?
          temp.y = IM_MAX(IM_MIN(tmp_blob.rect.y - margin, INT16_MAX), INT16_MIN); // ...
          temp.w = IM_MAX(IM_MIN(tmp_blob.rect.w + (margin * 2), INT16_MAX), 0);
          temp.h = IM_MAX(IM_MIN(tmp_blob.rect.h + (margin * 2), INT16_MAX), 0);

          if (rectangle_overlap(&(blob.rect), &temp)) {
            rectangle_united(&(tmp_blob.rect), &(blob.rect));
            blob.centroid.x = ((blob.centroid.x * blob.pixels) + (tmp_blob.centroid.x * tmp_blob.pixels)) / (blob.pixels + tmp_blob.pixels);
            blob.centroid.y = ((blob.centroid.y * blob.pixels) + (tmp_blob.centroid.y * tmp_blob.pixels)) / (blob.pixels + tmp_blob.pixels);
            blob.pixels += tmp_blob.pixels; // won't overflow
            blob.code |= tmp_blob.code;
            blob.count = IM_MAX(IM_MIN(blob.count + tmp_blob.count, UINT16_MAX), 0); // UINT16_MAX !?
            merge_occured = true;
          } else {
            list_push_back(output_ptr, &tmp_blob, tmpNode_ptr);
          }
        }
        list_push_back(&out_temp, &blob, tmpNode_ptr);
      }
      list_copy(output_ptr, &out_temp);

      if (!merge_occured) {
        break;
      }
    }
  }
}
