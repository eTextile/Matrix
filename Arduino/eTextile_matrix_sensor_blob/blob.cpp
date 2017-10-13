/* This file is part of the OpenMV project.
   Copyright (c) 2013-2017 Ibrahim Abdelkader <iabdalkader@openmv.io> & Kwabena W. Agyeman <kwagyeman@openmv.io>
   This work is licensed under the MIT license, see the file LICENSE for details.
*/

#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h> /* Used for memset() */
#include <math.h>

#include <Arduino.h>

#include "blob.h"
#include "collections.h"

boolean DEBUG_BLOB = false;

////////////// Rectangle Stuff //////////////

bool rectangle_overlap(rectangle_t *ptr0, rectangle_t *ptr1) {
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
  image_t *input,
  list_t *output,
  node_t *tmpNode,
  char *bitmapPtr,
  const int rows,
  const int cols,
  const int pixelThreshold,
  const int minBlobSize,
  const int minBlobPix,
  bool merge,
  int margin
) {

  lifo_t lifo;
  size_t lifo_len;
  lifo_alloc_all(&lifo, &lifo_len, sizeof(xylf_t));
  if (DEBUG_BLOB) Serial.printf("\nlifo_len: %d", lifo_len);

  size_t code = 0;

  for (int y = 0; y < rows; y++) {

    uint8_t *rowPtr = FRAME_ROW_PTR(input, y);         // Return pointer to image curent row
    size_t rowIndex = BITMAP_ROW_INDEX(input, y);      // Return bitmap curent row

    for (int x = 0; x < cols; x++) {
      if (DEBUG_BLOB) Serial.printf(" x:%d y:%d \n", x, y);

      if ((!bitmap_bit_get(bitmapPtr, BITMAP_INDEX(rowIndex, x)))
          && PIXEL_THRESHOLD(GET_FRAME_PIXEL(rowPtr, x), pixelThreshold)) {

        if (DEBUG_BLOB) Serial.println("__INIT_BLOB__");
        int old_x = x;
        int old_y = y;

        int blob_x1 = x;
        int blob_y1 = y;
        int blob_x2 = x;
        int blob_y2 = y;
        int blob_pixels = 0;
        int blob_cx = 0;
        int blob_cy = 0;

        ////////// Scanline flood fill algorithm //////////

        for (;;) {

          int left = x, right = x;

          uint8_t *rowPtr = FRAME_ROW_PTR(input, y);       // Return pointer to image curent row
          size_t rowIndex = BITMAP_ROW_INDEX(input, y);    // Return bitmap curent row

          while ((left > 0)
                 && (!bitmap_bit_get(bitmapPtr, BITMAP_INDEX(rowIndex, left - 1)))
                 && PIXEL_THRESHOLD(GET_FRAME_PIXEL(rowPtr, left - 1), pixelThreshold)) {
            left--;
            if (DEBUG_BLOB) Serial.printf("left_%d ", left);
          }
          while ((right < (cols - 1))
                 && (!bitmap_bit_get(bitmapPtr, BITMAP_INDEX(rowIndex, right + 1)))
                 && PIXEL_THRESHOLD(GET_FRAME_PIXEL(rowPtr, right + 1), pixelThreshold)) {
            right++;
            if (DEBUG_BLOB) Serial.printf("right_%d ", right);
          }

          blob_x1 = IM_MIN(blob_x1, left);
          blob_y1 = IM_MIN(blob_y1, y);
          blob_x2 = IM_MAX(blob_x2, right);
          blob_y2 = IM_MAX(blob_y2, y);

          for (int i = left; i <= right; i++) {
            bitmap_bit_set(bitmapPtr, BITMAP_INDEX(rowIndex, i));
            blob_pixels++;
            blob_cx += i;
            blob_cy += y;
          }
          if (DEBUG_BLOB) Serial.printf(">>>> blob_pixels: %d \n", blob_pixels);
          delay(1000);

          bool break_out = false;

          for (;;) {

            if (lifo_size(&lifo) < lifo_len) {
              if (DEBUG_BLOB) Serial.println("Adding blob to lifo");

              if (y > 0) {
                rowPtr = FRAME_ROW_PTR(input, y + 1);
                rowIndex = BITMAP_ROW_INDEX(input, y + 1);

                bool recurse = false;

                for (int i = left; i <= right; i++) {
                  if ((!bitmap_bit_get(bitmapPtr, BITMAP_INDEX(rowIndex, i)))
                      && PIXEL_THRESHOLD(GET_FRAME_PIXEL(rowPtr, i), pixelThreshold)) {
                    xylf_t context;
                    context.x = x;
                    context.y = y;
                    context.l = left;
                    context.r = right;
                    lifo_enqueue(&lifo, &context);
                    x = i;
                    y = y - 1;
                    recurse = true;
                    break;
                  }
                }
                if (recurse) {
                  if (DEBUG_BLOB) Serial.println(">>>> break_1");
                  break;
                }
              }

              if (y < (rows - 1)) {
                if (DEBUG_BLOB) Serial.println("y < rows - 1");
                rowPtr = FRAME_ROW_PTR(input, y + 1);      // return : (uint8_t *) input->data + input->w * y
                rowIndex = BITMAP_ROW_INDEX(input, y + 1); // return : input->w * y

                bool recurse = false;

                for (int i = left; i <= right; i++) {
                  if ((!bitmap_bit_get(bitmapPtr, BITMAP_INDEX(rowIndex, i)))
                      && PIXEL_THRESHOLD(GET_FRAME_PIXEL(rowPtr, i), pixelThreshold)) {
                        
                    xylf_t context;
                    context.x = x;
                    context.y = y;
                    context.l = left;
                    context.r = right;
                    lifo_enqueue(&lifo, &context); // Add element
                    x = i;
                    y = y + 1;
                    recurse = true;
                    break;
                  }
                }
                if (recurse) {
                  break;
                }
              }
            }

            if (!lifo_size(&lifo)) {
              if (DEBUG_BLOB) Serial.printf("Lifo umpty: %d", lifo_size(&lifo));
              break_out = true;
              break;
            }

            xylf_t context;
            lifo_dequeue(&lifo, &context);
            x = context.x;
            y = context.y;
            left = context.l;
            right = context.r;
          }

          if (break_out) {
            if (DEBUG_BLOB) Serial.println(">>>> break_2");
            break;
          }
        }

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
          list_push_back(output, &blob, tmpNode);
          if (DEBUG_BLOB) Serial.printf("Added blob to the blob list: %d", code);
        }

        x = old_x;
        y = old_y;
      }
    }
  }
  code++;

  lifo_free(&lifo);

  // memset(bitmap, 0, NEW_FRAME * sizeof(char)); // TODO: use NEW_FRAME constante
  memset(bitmapPtr, 0, rows * cols * sizeof(char)); // Set bitmap array datas to 0

  if (merge) {
    for (;;) {
      bool merge_occured = false;

      list_t out_temp;
      list_init(&out_temp, sizeof(blob_t));

      while (list_size(output)) {

        blob_t blob;

        list_pop_front(output, &blob, tmpNode);

        for (size_t k = 0, l = list_size(output); k < l; k++) {

          blob_t tmp_blob;

          list_pop_front(output, &tmp_blob, tmpNode);

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
            list_push_back(output, &tmp_blob, tmpNode);
          }
        }
        list_push_back(&out_temp, &blob, tmpNode);
      }
      list_copy(output, &out_temp);

      if (!merge_occured) {
        break;
      }
    }
  }
}
