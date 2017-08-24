/* This file is part of the OpenMV project.
   Copyright (c) 2013-2017 Ibrahim Abdelkader <iabdalkader@openmv.io> & Kwabena W. Agyeman <kwagyeman@openmv.io>
   This work is licensed under the MIT license, see the file LICENSE for details.
*/

// #ifndef __IMLIB_H__
// #define __IMLIB_H__

#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h> //oK int16_t
#include <string.h>
#include <math.h>
#include <arm_math.h> //ok!
// #include <ff.h>
// #include "fb_alloc.h"
// #include "umm_malloc.h"
// #include "xalloc.h" // !it use mp.h
// #include "array.h"
#include "fmath.h"

#include "collections.h"

#define IM_MAX(a,b) ({ __typeof__ (a) _a = (a); __typeof__ (b) _b = (b); _a > _b ? _a : _b; })
#define IM_MIN(a,b) ({ __typeof__ (a) _a = (a); __typeof__ (b) _b = (b); _a < _b ? _a : _b; })

/////////////////
// Point Stuff //
/////////////////

typedef struct point {
  int16_t x;
  int16_t y;
} point_t;

void point_init(point_t *ptr, int x, int y);
void point_copy(point_t *dst, point_t *src);
bool point_equal_fast(point_t *ptr0, point_t *ptr1);
int point_quadrance(point_t *ptr0, point_t *ptr1);

/////////////////////
// Rectangle Stuff //
/////////////////////

typedef struct rectangle {
  int16_t x;
  int16_t y;
  int16_t w;
  int16_t h;
} rectangle_t;

void rectangle_init(rectangle_t *ptr, int x, int y, int w, int h);
void rectangle_copy(rectangle_t *dst, rectangle_t *src);
bool rectangle_equal_fast(rectangle_t *ptr0, rectangle_t *ptr1);
bool rectangle_overlap(rectangle_t *ptr0, rectangle_t *ptr1);
void rectangle_intersected(rectangle_t *dst, rectangle_t *src);
void rectangle_united(rectangle_t *dst, rectangle_t *src);

/////////////////
// Color Stuff //
/////////////////

typedef struct color_thresholds_list_lnk_data {
  uint8_t LMin, LMax; // or grayscale
  int8_t AMin, AMax;
  int8_t BMin, BMax;
}

color_thresholds_list_lnk_data_t;

#define COLOR_THRESHOLD_GRAYSCALE(pixel, threshold, invert) \
  ({ \
    __typeof__ (pixel) _pixel = (pixel); \
    __typeof__ (threshold) _threshold = (threshold); \
    __typeof__ (invert) _invert = (invert); \
    ((_threshold->LMin <= _pixel) && (_pixel <= _threshold->LMax)) ^ _invert; \
  })

#define COLOR_GRAYSCALE_MIN 0
#define COLOR_GRAYSCALE_MAX 255

/////////////////
// Image Stuff //
/////////////////

typedef struct image {
  int w;
  int h;
  int bpp;
  union {
    uint8_t *pixels;
    uint8_t *data;
  };
} image_t;

void image_init(image_t *ptr, int w, int h, int bpp, void *data);
void image_copy(image_t *dst, image_t *src);
uint32_t image_size(image_t *ptr);

#define IMAGE_GET_BINARY_PIXEL(image, x, y) \
  ({ \
    __typeof__ (image) _image = (image); \
    __typeof__ (x) _x = (x); \
    __typeof__ (y) _y = (y); \
    (((uint32_t *) _image->data)[(((_image->w + UINT32_T_MASK) >> UINT32_T_SHIFT) * _y) + (_x >> UINT32_T_SHIFT)] >> (_x & UINT32_T_MASK)) & 1; \
  })

#define IMAGE_GET_GRAYSCALE_PIXEL(image, x, y) \
  ({ \
    __typeof__ (image) _image = (image); \
    __typeof__ (x) _x = (x); \
    __typeof__ (y) _y = (y); \
    ((uint8_t *) _image->data)[(_image->w * _y) + _x]; \
  })

#define IMAGE_PUT_GRAYSCALE_PIXEL(image, x, y, v) \
  ({ \
    __typeof__ (image) _image = (image); \
    __typeof__ (x) _x = (x); \
    __typeof__ (y) _y = (y); \
    __typeof__ (v) _v = (v); \
    ((uint8_t *) _image->data)[(_image->w * _y) + _x] = _v; \
  })

#define IMAGE_GET_RGB565_PIXEL(image, x, y) \
  ({ \
    __typeof__ (image) _image = (image); \
    __typeof__ (x) _x = (x); \
    __typeof__ (y) _y = (y); \
    ((uint16_t *) _image->data)[(_image->w * _y) + _x]; \
  })

#define IMAGE_PUT_RGB565_PIXEL(image, x, y, v) \
  ({ \
    __typeof__ (image) _image = (image); \
    __typeof__ (x) _x = (x); \
    __typeof__ (y) _y = (y); \
    __typeof__ (v) _v = (v); \
    ((uint16_t *) _image->data)[(_image->w * _y) + _x] = _v; \
  })

#ifdef __arm__

#define IMAGE_REV_RGB565_PIXEL(pixel) \
  ({ \
    __typeof__ (pixel) _pixel = (pixel); \
    __REV16(_pixel); \
  })
#else

#define IMAGE_REV_RGB565_PIXEL(pixel) \
  ({ \
    __typeof__ (pixel) _pixel = (pixel); \
    ((_pixel >> 8) | (_pixel << 8)) & 0xFFFF; \
  })
#endif

#define IMAGE_COMPUTE_TARGET_SIZE_SCALE_FACTOR(target_size, source_rect) \
  __typeof__ (target_size) _target_size = (target_size); \
  __typeof__ (source_rect) _source_rect = (source_rect); \
  int IMAGE_X_SOURCE_OFFSET = _source_rect->p.x; \
  int IMAGE_Y_SOURCE_OFFSET = _source_rect->p.y; \
  int IMAGE_X_TARGET_OFFSET = 0; \
  int IMAGE_Y_TARGET_OFFSET = 0; \
  float IMAGE_X_RATIO = ((float) _source_rect->s.w) / ((float) _target_size->w); \
  float IMAGE_Y_RATIO = ((float) _source_rect->s.h) / ((float) _target_size->h); \
  ({ 0; })

#define IMAGE_COMPUTE_TARGET_RECT_SCALE_FACTOR(target_rect, source_rect) \
  __typeof__ (target_rect) _target_rect = (target_rect); \
  __typeof__ (source_rect) _source_rect = (source_rect); \
  int IMAGE_X_SOURCE_OFFSET = _source_rect->p.x; \
  int IMAGE_Y_SOURCE_OFFSET = _source_rect->p.y; \
  int IMAGE_X_TARGET_OFFSET = _target_rect->p.x; \
  int IMAGE_Y_TARGET_OFFSET = _target_rect->p.y; \
  float IMAGE_X_RATIO = ((float) _source_rect->s.w) / ((float) _target_rect->s.w); \
  float IMAGE_Y_RATIO = ((float) _source_rect->s.h) / ((float) _target_rect->s.h); \
  ({ 0; })

#define IMAGE_GET_SCALED_BINARY_PIXEL(image, x, y) \
  ({ \
    __typeof__ (image) _image = (image); \
    __typeof__ (x) _x = (x); \
    __typeof__ (y) _y = (y); \
    IMAGE_GET_BINARY_PIXEL(_image, ((size_t) ((IMAGE_X_RATIO * (_x - IMAGE_X_TARGET_OFFSET)) + 0.5)) + IMAGE_X_SOURCE_OFFSET, ((size_t) ((IMAGE_Y_RATIO * (_y - IMAGE_Y_TARGET_OFFSET)) + 0.5)) + IMAGE_Y_SOURCE_OFFSET); \
  })

#define IMAGE_GET_SCALED_GRAYSCALE_PIXEL(image, x, y) \
  ({ \
    __typeof__ (image) _image = (image); \
    __typeof__ (x) _x = (x); \
    __typeof__ (y) _y = (y); \
    IMAGE_GET_GRAYSCALE_PIXEL(_image, ((size_t) ((IMAGE_X_RATIO * (_x - IMAGE_X_TARGET_OFFSET)) + 0.5)) + IMAGE_X_SOURCE_OFFSET, ((size_t) ((IMAGE_Y_RATIO * (_y - IMAGE_Y_TARGET_OFFSET)) + 0.5)) + IMAGE_Y_SOURCE_OFFSET); \
  })

#define IMAGE_GET_SCALED_RGB565_PIXEL(image, x, y) \
  ({ \
    __typeof__ (image) _image = (image); \
    __typeof__ (x) _x = (x); \
    __typeof__ (y) _y = (y); \
    IMAGE_GET_RGB565_PIXEL(_image, ((size_t) ((IMAGE_X_RATIO * (_x - IMAGE_X_TARGET_OFFSET)) + 0.5)) + IMAGE_X_SOURCE_OFFSET, ((size_t) ((IMAGE_Y_RATIO * (_y - IMAGE_Y_TARGET_OFFSET)) + 0.5)) + IMAGE_Y_SOURCE_OFFSET); \
  })

// Fast Stuff //

#define IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(image, y) \
  ({ \
    __typeof__ (image) _image = (image); \
    __typeof__ (y) _y = (y); \
    ((uint8_t *) _image->data) + (_image->w * _y); \
  })

#define IMAGE_INC_GRAYSCALE_PIXEL_ROW_PTR(row_ptr, image) \
  ({ \
    __typeof__ (row_ptr) _row_ptr = (row_ptr); \
    __typeof__ (image) _image = (image); \
    row_ptr + _image->w; \
  })

#define IMAGE_GET_GRAYS

#define IMAGE_GET_GRAYSCALE_PIXEL_FAST(row_ptr, x) \
  ({ \
    __typeof__ (row_ptr) _row_ptr = (row_ptr); \
    __typeof__ (x) _x = (x); \
    _row_ptr[_x]; \
  })

#define IMAGE_COMPUTE_SCALED_GRAYSCALE_PIXEL_ROW_PTR(image, y) \
  ({ \
    __typeof__ (image) _image = (image); \
    __typeof__ (y) _y = (y); \
    ((uint8_t *) _image->data) + (_image->w * (((size_t) ((IMAGE_Y_RATIO * (_y - IMAGE_Y_TARGET_OFFSET)) + 0.5)) + IMAGE_Y_SOURCE_OFFSET)); \
  })

#define IMAGE_GET_SCALED_GRAYSCALE_PIXEL_FAST(row_ptr, x) IMAGE_GET_GRAYSCALE_PIXEL_FAST((row_ptr), ((size_t) ((IMAGE_X_RATIO * ((x) - IMAGE_X_TARGET_OFFSET)) + 0.5)) + IMAGE_X_SOURCE_OFFSET) \

#define IMAGE_COMPUTE_SCALED_RGB565_PIXEL_ROW_PTR(image, y) \
  ({ \
    __typeof__ (image) _image = (image); \
    __typeof__ (y) _y = (y); \
    ((uint16_t *) _image->data) + (_image->w * (((size_t) ((IMAGE_Y_RATIO * (_y - IMAGE_Y_TARGET_OFFSET)) + 0.5)) + IMAGE_Y_SOURCE_OFFSET)); \
  }) \

  // Grayscale maxes
#define IM_MAX_GS (255)

  // Grayscale histogram
#define IM_G_HIST_SIZE (256)
#define IM_G_HIST_OFFSET (0)

  // LAB histogram
#define IM_L_HIST_SIZE (256)
#define IM_L_HIST_OFFSET (0)
#define IM_A_HIST_SIZE (256)
#define IM_A_HIST_OFFSET (256)
#define IM_B_HIST_SIZE (256)
#define IM_B_HIST_OFFSET (512)

#define IM_X_INSIDE(img, x) \
  ({ __typeof__ (img) _img = (img); \
    __typeof__ (x) _x = (x); \
    (0 <= _x)&&(_x < _img->w); \
  })

#define IM_Y_INSIDE(img, y) \
  ({ __typeof__ (img) _img = (img); \
    __typeof__ (y) _y = (y); \
    (0 <= _y)&&(_y < _img->h); \
  })

#define IM_GET_GS_PIXEL(img, x, y) \
  ({ __typeof__ (img) _img = (img); \
    __typeof__ (x) _x = (x); \
    __typeof__ (y) _y = (y); \
    ((uint8_t*)_img->pixels)[(_y * _img->w) + _x]; \
  })

#define IM_GET_RAW_PIXEL(img, x, y) \
  ({ __typeof__ (img) _img = (img); \
    __typeof__ (x) _x = (x); \
    __typeof__ (y) _y = (y); \
    ((uint8_t*)_img->pixels)[(_y * _img->w) + _x]; \
  })

#define IM_SET_GS_PIXEL(img, x, y, p) \
  ({ __typeof__ (img) _img = (img); \
    __typeof__ (x) _x = (x); \
    __typeof__ (y) _y = (y); \
    __typeof__ (p) _p = (p); \
    ((uint8_t*)_img->pixels)[(_y * _img->w) + _x] = _p; \
  })


#define IM_EQUAL(img0, img1) \
  ({ __typeof__ (img0) _img0 = (img0); \
    __typeof__ (img1) _img1 = (img1); \
    (_img0->w == _img1->w)&&(_img0->h == _img1->h)&&(_img0->bpp == _img1->bpp); \
  })

  typedef struct integral_image {
    int w;
    int h;
    uint32_t *data;
  } i_image_t;

  typedef struct {
    int w;
    int h;
    int y_offs;
    int x_ratio;
    int y_ratio;
    uint32_t **data;
    uint32_t **swap;
  } mw_image_t;

  typedef struct _vector {
    float x;
    float y;
    float m;
    uint16_t cx, cy;
  } vec_t;


  // Return the distance between a cluster centroid and some object.
  typedef float (*cluster_dist_t)(int cx, int cy, void *obj);

  typedef struct size {
    int w;
    int h;
  } wsize_t;

  typedef struct bmp_read_settings {
    int32_t bmp_w;
    int32_t bmp_h;
    uint16_t bmp_bpp;
    uint32_t bmp_fmt;
    uint32_t bmp_row_bytes;
  } bmp_read_settings_t;

  typedef struct find_blobs_list_lnk_data {
    rectangle_t rect;
    uint32_t pixels;
    point_t centroid;
    float rotation;
    uint16_t code, count;
  } find_blobs_list_lnk_data_t;

  /* Basic image functions */
  int imlib_get_pixel(image_t *img, int x, int y);
  void imlib_set_pixel(image_t *img, int x, int y, int p);

  /* Point functions */
  point_t *point_alloc(int16_t x, int16_t y);
  bool point_equal(point_t *p1, point_t *p2);
  float point_distance(point_t *p1, point_t *p2);

  /* Rectangle functions */
  rectangle_t *rectangle_alloc(int16_t x, int16_t y, int16_t w, int16_t h);
  bool rectangle_equal(rectangle_t *r1, rectangle_t *r2);
  bool rectangle_intersects(rectangle_t *r1, rectangle_t *r2);
  bool rectangle_subimg(image_t *img, rectangle_t *r, rectangle_t *r_out);
  // array_t *rectangle_merge(array_t *rectangles);
  void rectangle_expand(rectangle_t *r, int x, int y);

  /* Background Subtraction (Frame Differencing) functions */
  void imlib_negate(image_t *img);
  void imlib_difference(image_t *img, const char *path, image_t *other);
  void imlib_replace(image_t *img, const char *path, image_t *other);
  void imlib_blend(image_t *img, const char *path, image_t *other, int alpha);

  /* Image Filtering */
  void imlib_midpoint_filter(image_t *img, const int ksize, const int bias);
  void imlib_mean_filter(image_t *img, const int ksize);
  void imlib_mode_filter(image_t *img, const int ksize);
  void imlib_median_filter(image_t *img, const int ksize, const int percentile);
  void imlib_histeq(image_t *img);
  void imlib_mask_ellipse(image_t *img);

  // Image filter functions
  void im_filter_bw(uint8_t *src, uint8_t *dst, int size, int bpp, void *args);
  void im_filter_skin(uint8_t *src, uint8_t *dst, int size, int bpp, void *args);

  // Edge detection
  void imlib_edge_simple(image_t *src, rectangle_t *roi, int low_thresh, int high_thresh);
  void imlib_edge_canny(image_t *src, rectangle_t *roi, int low_thresh, int high_thresh);

  // Color Tracking
  void find_blobs(
    list_t *out, image_t *ptr, rectangle_t *roi, unsigned int x_stride, unsigned int y_stride,
    list_t *thresholds, bool invert, unsigned int area_threshold, unsigned int pixels_threshold,
    bool merge, int margin,
    bool (*threshold_cb)(void*, find_blobs_list_lnk_data_t*), void *threshold_cb_arg,
    bool (*merge_cb)(void*, find_blobs_list_lnk_data_t*, find_blobs_list_lnk_data_t*), void *merge_cb_arg
  );

