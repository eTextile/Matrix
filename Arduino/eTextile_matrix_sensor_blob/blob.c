/* This file is part of the OpenMV project.
   Copyright (c) 2013-2017 Ibrahim Abdelkader <iabdalkader@openmv.io> & Kwabena W. Agyeman <kwagyeman@openmv.io>
   This work is licensed under the MIT license, see the file LICENSE for details.
*/

#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h> /* Used for memset() */
#include <math.h>

#include "blob.h"
#include "collections.h"

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
  list_t *out,
  image_t *ptr,
  rectangle_t *roi,
  const unsigned int pixelThreshold,
  const unsigned int minBlobSize,
  const unsigned int minBlobPix,
  bool merge,
  int margin
) {


  heap_t *heap = (heap_t*)malloc(sizeof(heap_t));
  memset(heap, 0, sizeof(heap_t));

  void *region = malloc(HEAP_INIT_SIZE);
  memset(region, 0, HEAP_INIT_SIZE);

  for (int i = 0; i < BIN_COUNT; i++) {
    heap->bins[i] = (bin_t*)malloc(sizeof(bin_t));
    memset(heap->bins[i], 0, sizeof(bin_t));
  }

  init_heap(heap, (uint)region);

  bitmap_t bitmap; // Create the bitmap_t instance
  bitmap_alloc(&bitmap, ptr->w * ptr->h);   // Allocate memody for the bitmap_t instance with fb_alloc0()

  lifo_t lifo;
  size_t lifo_len;
  lifo_alloc_all(&lifo, &lifo_len, sizeof(xylf_t));   // Allocate memody for the lifo buffer
  list_init(out, sizeof(find_blobs_list_lnk_data_t));

  size_t code = 0;

  for (int y = roi->y, yy = roi->y + roi->h; y < yy; y++) {

    uint8_t *row_ptr = IMAGE_COMPUTE_ROW_PTR(ptr, y);         // Return pointer to image curent row
    size_t row_index = BITMAP_COMPUTE_ROW_INDEX(ptr, y);      // Return bitmap curent row

    for (int x = roi->x, xx = roi->x + roi->w; x < xx; x++) {

      if ((!bitmap_bit_get(&bitmap, BITMAP_COMPUTE_INDEX(row_index, x)))
          && GRAYSCALE_THRESHOLD(IMAGE_GET_PIXEL_FAST(row_ptr, x), pixelThreshold)) {

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

          uint8_t *row = IMAGE_COMPUTE_ROW_PTR(ptr, y);       // Return pointer to image curent row
          size_t index = BITMAP_COMPUTE_ROW_INDEX(ptr, y);     // Return bitmap curent row

          while ((left > roi->x)
                 && (!bitmap_bit_get(&bitmap, BITMAP_COMPUTE_INDEX(index, left - 1))) // BITMAP_COMPUTE_INDEX return : index + x
                 && GRAYSCALE_THRESHOLD(IMAGE_GET_PIXEL_FAST(row, left - 1), pixelThreshold)) {
            left--;
          }
          while ((right < (roi->x + roi->w - 1))
                 && (!bitmap_bit_get(&bitmap, BITMAP_COMPUTE_INDEX(index, right + 1)))
                 && GRAYSCALE_THRESHOLD(IMAGE_GET_PIXEL_FAST(row, right + 1), pixelThreshold)) {
            right++;
          }

          blob_x1 = IM_MIN(blob_x1, left);
          blob_y1 = IM_MIN(blob_y1, y);
          blob_x2 = IM_MAX(blob_x2, right);
          blob_y2 = IM_MAX(blob_y2, y);

          for (int i = left; i <= right; i++) {
            bitmap_bit_set(&bitmap, BITMAP_COMPUTE_INDEX(index, i));
            blob_pixels += 1;
            blob_cx += i;
            blob_cy += y;
          }

          bool break_out = false;

          for (;;) {
            if (lifo_size(&lifo) < lifo_len) {

              if (y > roi->y) {
                row = IMAGE_COMPUTE_ROW_PTR(ptr, y + 1);      // return : (uint8_t *) ptr->data + ptr->w * y
                index = BITMAP_COMPUTE_ROW_INDEX(ptr, y + 1); // return : ptr->w * y

                bool recurse = false;
                for (int i = left; i <= right; i++) {
                  if ((!bitmap_bit_get(&bitmap, BITMAP_COMPUTE_INDEX(index, i)))
                      && GRAYSCALE_THRESHOLD(IMAGE_GET_PIXEL_FAST(row, i), pixelThreshold)) {
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
                  break;
                }
              }

              if (y < (roi->y + roi->h - 1)) {
                row = IMAGE_COMPUTE_ROW_PTR(ptr, y + 1);      // return : (uint8_t *) ptr->data + ptr->w * y
                index = BITMAP_COMPUTE_ROW_INDEX(ptr, y + 1); // return : ptr->w * y

                bool recurse = false;

                for (int i = left; i <= right; i++) {
                  if ((!bitmap_bit_get(&bitmap, BITMAP_COMPUTE_INDEX(index, i)))
                      && GRAYSCALE_THRESHOLD(IMAGE_GET_PIXEL_FAST(row, i), pixelThreshold)) {
                    xylf_t context;
                    context.x = x;
                    context.y = y;
                    context.l = left;
                    context.r = right;
                    lifo_enqueue(&lifo, &context);
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
            break;
          }
        }

        int mx = blob_cx / blob_pixels; // x centroid
        int my = blob_cy / blob_pixels; // y centroid

        find_blobs_list_lnk_data_t lnk_blob;
        lnk_blob.rect.x = blob_x1;
        lnk_blob.rect.y = blob_y1;
        lnk_blob.rect.w = blob_x2 - blob_x1;
        lnk_blob.rect.h = blob_y2 - blob_y1;
        lnk_blob.pixels = blob_pixels;
        lnk_blob.centroid.x = mx;
        lnk_blob.centroid.y = my;
        lnk_blob.code = 1 << code;
        lnk_blob.count = 1;

        if (((lnk_blob.rect.w * lnk_blob.rect.h) >= minBlobSize) && (lnk_blob.pixels >= minBlobPix)) {
          list_push_back(heap, out, &lnk_blob);
        }

        x = old_x;
        y = old_y;
      }
    }
  }
  code++;

  lifo_free(&lifo);
  bitmap_free(&bitmap);

  if (merge) {
    for (;;) {
      bool merge_occured = false;

      list_t out_temp;
      list_init(&out_temp, sizeof(find_blobs_list_lnk_data_t));

      while (list_size(out)) {

        find_blobs_list_lnk_data_t lnk_blob;

        list_pop_front(heap, out, &lnk_blob);

        for (size_t k = 0, l = list_size(out); k < l; k++) {

          find_blobs_list_lnk_data_t tmp_blob;

          list_pop_front(heap, out, &tmp_blob);

          rectangle_t temp;
          temp.x = IM_MAX(IM_MIN(tmp_blob.rect.x - margin, INT16_MAX), INT16_MIN);
          temp.y = IM_MAX(IM_MIN(tmp_blob.rect.y - margin, INT16_MAX), INT16_MIN);
          temp.w = IM_MAX(IM_MIN(tmp_blob.rect.w + (margin * 2), INT16_MAX), 0);
          temp.h = IM_MAX(IM_MIN(tmp_blob.rect.h + (margin * 2), INT16_MAX), 0);

          if (rectangle_overlap(&(lnk_blob.rect), &temp)) {
            rectangle_united(&(tmp_blob.rect), &(lnk_blob.rect));
            lnk_blob.centroid.x = ((lnk_blob.centroid.x * lnk_blob.pixels) + (tmp_blob.centroid.x * tmp_blob.pixels)) / (lnk_blob.pixels + tmp_blob.pixels);
            lnk_blob.centroid.y = ((lnk_blob.centroid.y * lnk_blob.pixels) + (tmp_blob.centroid.y * tmp_blob.pixels)) / (lnk_blob.pixels + tmp_blob.pixels);
            lnk_blob.pixels += tmp_blob.pixels; // won't overflow
            lnk_blob.code |= tmp_blob.code;
            lnk_blob.count = IM_MAX(IM_MIN(lnk_blob.count + tmp_blob.count, UINT16_MAX), 0);
            merge_occured = true;
          } else {
            list_push_back(heap, out, &tmp_blob);
          }
        }
        list_push_back(heap, &out_temp, &lnk_blob);
      }
      list_copy(out, &out_temp);

      if (!merge_occured) {
        break;
      }
    }
  }
}
