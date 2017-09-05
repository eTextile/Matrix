/* This file is part of the OpenMV project.
   Copyright (c) 2013-2017 Ibrahim Abdelkader <iabdalkader@openmv.io> & Kwabena W. Agyeman <kwagyeman@openmv.io>
   This work is licensed under the MIT license, see the file LICENSE for details.
*/
#include "blob.h"
#include <stdint.h>

typedef struct xylf {
  int16_t x, y, l, r;
} xylf_t;

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

void rectangle_united(rectangle_t *dst, rectangle_t *src) {
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
  thresholds_t *thresholds,
  bool invert,
  unsigned int area_threshold,
  unsigned int pixels_threshold,
  bool merge,
  int margin
  // bool (*threshold_cb)(void*, find_blobs_list_lnk_data_t*), void *threshold_cb_arg,
  // bool (*merge_cb)(void*, find_blobs_list_lnk_data_t*, find_blobs_list_lnk_data_t*), void *merge_cb_arg
) {

  bitmap_t bitmap; // Create the bitmap_t instance
  bitmap_alloc(&bitmap, ptr->w * ptr->h); // Set the size of the bitmap_t instance

  lifo_t lifo;
  size_t lifo_len;
  lifo_alloc_all(&lifo, &lifo_len, sizeof(xylf_t));
  list_init(out, sizeof(find_blobs_list_lnk_data_t));

  size_t code = 0;

  for (int y = roi->y, yy = roi->y + roi->h; y < yy; y++) {

    uint8_t *row_ptr = IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(ptr, y); // image->data + image->w * y

    size_t row_index = BITMAP_COMPUTE_ROW_INDEX(ptr, y); // image->w * y

    for (int x = roi->x, xx = roi->x + roi->w; x < xx; x++) {

      if ((!bitmap_bit_get(&bitmap, BITMAP_COMPUTE_INDEX(row_index, x)))
          && GRAYSCALE_THRESHOLD(IMAGE_GET_GRAYSCALE_PIXEL_FAST(row_ptr, x), &thresholds, invert)) {

        int old_x = x;
        int old_y = y;

        int blob_x1 = x;
        int blob_y1 = y;
        int blob_x2 = x;
        int blob_y2 = y;
        int blob_pixels = 0;
        int blob_cx = 0;
        int blob_cy = 0;
        long long blob_a = 0;
        long long blob_b = 0;
        long long blob_c = 0;

        // Scanline Flood Fill Algorithm //

        for (;;) {
          int left = x, right = x;
          uint8_t *row = IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(ptr, y);  // Déclaration
          size_t index = BITMAP_COMPUTE_ROW_INDEX(ptr, y);

          while ((left > roi->x)
                 && (!bitmap_bit_get(&bitmap, BITMAP_COMPUTE_INDEX(index, left - 1)))
                 && GRAYSCALE_THRESHOLD(IMAGE_GET_GRAYSCALE_PIXEL_FAST(row, left - 1), &thresholds, invert)) {
            left--;
          }

          while ((right < (roi->x + roi->w - 1))
                 && (!bitmap_bit_get(&bitmap, BITMAP_COMPUTE_INDEX(index, right + 1)))
                 && GRAYSCALE_THRESHOLD(IMAGE_GET_GRAYSCALE_PIXEL_FAST(row, right + 1), &thresholds, invert)) {
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
            blob_a += i * i;
            blob_b += i * y;
            blob_c += y * y;
          }

          bool break_out = false;
          for (;;) {
            if (lifo_size(&lifo) < lifo_len) {

              if (y > roi->y) {
                row = IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(ptr, y - 1);
                index = BITMAP_COMPUTE_ROW_INDEX(ptr, y - 1);

                bool recurse = false;
                for (int i = left; i <= right; i++) {
                  if ((!bitmap_bit_get(&bitmap, BITMAP_COMPUTE_INDEX(index, i)))
                      && GRAYSCALE_THRESHOLD(IMAGE_GET_GRAYSCALE_PIXEL_FAST(row, i), &thresholds, invert)) {
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
                row = IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(ptr, y + 1);
                index = BITMAP_COMPUTE_ROW_INDEX(ptr, y + 1);

                bool recurse = false;

                for (int i = left; i <= right; i++) {
                  if ((!bitmap_bit_get(&bitmap, BITMAP_COMPUTE_INDEX(index, i)))
                      && GRAYSCALE_THRESHOLD(IMAGE_GET_GRAYSCALE_PIXEL_FAST(row, i), &thresholds, invert)) {
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

        // http://www.cse.usf.edu/~r1k/MachineVisionBook/MachineVision.files/MachineVision_Chapter2.pdf
        // https://www.strchr.com/standard_deviation_in_one_pass
        //
        // a = sigma(x*x) + (mx*sigma(x)) + (mx*sigma(x)) + (sigma()*mx*mx)
        // b = sigma(x*y) + (mx*sigma(y)) + (my*sigma(x)) + (sigma()*mx*my)
        // c = sigma(y*y) + (my*sigma(y)) + (my*sigma(y)) + (sigma()*my*my)
        //
        // blob_a = sigma(x*x)
        // blob_b = sigma(x*y)
        // blob_c = sigma(y*y)
        // blob_cx = sigma(x)
        // blob_cy = sigma(y)
        // blob_pixels = sigma()

        int mx = blob_cx / blob_pixels; // x centroid
        int my = blob_cy / blob_pixels; // y centroid
        // int small_blob_a = blob_a - ((mx * blob_cx) + (mx * blob_cx)) + (blob_pixels * mx * mx);
        // int small_blob_b = blob_b - ((mx * blob_cy) + (my * blob_cx)) + (blob_pixels * mx * my);
        // int small_blob_c = blob_c - ((my * blob_cy) + (my * blob_cy)) + (blob_pixels * my * my);

        find_blobs_list_lnk_data_t lnk_blob;
        lnk_blob.rect.x = blob_x1;
        lnk_blob.rect.y = blob_y1;
        lnk_blob.rect.w = blob_x2 - blob_x1;
        lnk_blob.rect.h = blob_y2 - blob_y1;
        lnk_blob.pixels = blob_pixels;
        lnk_blob.centroid.x = mx;
        lnk_blob.centroid.y = my;
        // lnk_blob.rotation = (small_blob_a != small_blob_c) ? (fast_atan2f(2 * small_blob_b, small_blob_a - small_blob_c) / 2.0f) : 0.0f;
        lnk_blob.code = 1 << code;
        lnk_blob.count = 1;

        if (((lnk_blob.rect.w * lnk_blob.rect.h) >= area_threshold) && (lnk_blob.pixels >= pixels_threshold)) {
          // && ((threshold_cb_arg == NULL) || threshold_cb(threshold_cb_arg, &lnk_blob))) {
          list_push_back(out, &lnk_blob);
        }

        x = old_x;
        y = old_y;
      }
    }
  }
  code += 1;

  lifo_free(&lifo);
  bitmap_free(&bitmap);

  if (merge) {
    for (;;) {
      bool merge_occured = false;

      list_t out_temp;

      list_init(&out_temp, sizeof(find_blobs_list_lnk_data_t));

      while (list_size(out)) {

        find_blobs_list_lnk_data_t lnk_blob;

        list_pop_front(out, &lnk_blob);

        for (size_t k = 0, l = list_size(out); k < l; k++) {

          find_blobs_list_lnk_data_t tmp_blob;

          list_pop_front(out, &tmp_blob);

          rectangle_t temp;
          temp.x = IM_MAX(IM_MIN(tmp_blob.rect.x - margin, INT16_MAX), INT16_MIN);
          temp.y = IM_MAX(IM_MIN(tmp_blob.rect.y - margin, INT16_MAX), INT16_MIN);
          temp.w = IM_MAX(IM_MIN(tmp_blob.rect.w + (margin * 2), INT16_MAX), 0);
          temp.h = IM_MAX(IM_MIN(tmp_blob.rect.h + (margin * 2), INT16_MAX), 0);

          if (rectangle_overlap(&(lnk_blob.rect), &temp)) {
            // && ((merge_cb_arg == NULL) || merge_cb(merge_cb_arg, &lnk_blob, &tmp_blob))) {
            rectangle_united(&(lnk_blob.rect), &(tmp_blob.rect));
            lnk_blob.centroid.x = ((lnk_blob.centroid.x * lnk_blob.pixels) + (tmp_blob.centroid.x * tmp_blob.pixels)) / (lnk_blob.pixels + tmp_blob.pixels);
            lnk_blob.centroid.y = ((lnk_blob.centroid.y * lnk_blob.pixels) + (tmp_blob.centroid.y * tmp_blob.pixels)) / (lnk_blob.pixels + tmp_blob.pixels);
            // float sin_mean = ((sinf(lnk_blob.rotation) * lnk_blob.pixels) + (sinf(tmp_blob.rotation) * tmp_blob.pixels)) / (lnk_blob.pixels + tmp_blob.pixels);
            // float cos_mean = ((cosf(lnk_blob.rotation) * lnk_blob.pixels) + (cosf(tmp_blob.rotation) * tmp_blob.pixels)) / (lnk_blob.pixels + tmp_blob.pixels);
            // lnk_blob.rotation = fast_atan2f(sin_mean, cos_mean);
            lnk_blob.pixels += tmp_blob.pixels; // won't overflow
            lnk_blob.code |= tmp_blob.code;
            lnk_blob.count = IM_MAX(IM_MIN(lnk_blob.count + tmp_blob.count, UINT16_MAX), 0);
            merge_occured = true;
          } else {
            list_push_back(out, &tmp_blob);
          }
        }
        list_push_back(&out_temp, &lnk_blob);
      }

      list_copy(out, &out_temp);

      if (!merge_occured) {
        break;
      }
    }
  }
}
