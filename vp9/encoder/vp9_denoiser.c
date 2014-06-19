/*
 *  Copyright (c) 2012 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include <stdio.h>
#include <stdint.h>
#include "vp9/encoder/vp9_denoiser.h"
#include "vpx_scale/yv12config.h"

static const int widths[]  = {4, 4, 8, 8,  8, 16, 16, 16, 32, 32, 32, 64, 64};
static const int heights[] = {4, 8, 4, 8, 16,  8, 16, 32, 16, 32, 64, 32, 64};

int vp9_denoiser_filter() {
  return 0;
}

int update_running_avg(uint8_t *mc_avg, int mc_avg_stride, uint8_t *avg,
                       int avg_stride, uint8_t *sig, int sig_stride,
                       BLOCK_SIZE bs) {
  //                           Indices: 0, 1, 2, 3, 4, 5 ,6, 7,
  static const uint8_t adjustments[] = {0, 0, 0, 0, 3, 3, 3, 3,
  //                                    8, 9,10,11,12,13,14,15,16
                                        4, 4, 4, 4, 4, 4, 4, 4, 6};
  int r, c;
  int diff;
  int adjustment;
  int total_adj = 0;

  for (r = 0; r < heights[bs]; ++r) {
    for (c = 0; c < widths[bs]; ++c) {
      diff = mc_avg[c] - sig[c];
      adjustment = adjustments[MIN(abs(diff), 16)];

      if (diff > 0) {
        avg[c] = MIN(UINT8_MAX, sig[c] + adjustment);
        total_adj += adjustment;
      } else {
        avg[c] = MAX(0, sig[c] - adjustment);
        total_adj -= adjustment;
      }
    }
    sig += sig_stride;
    avg += avg_stride;
    mc_avg += mc_avg_stride;
  }
  return total_adj;
}

uint8_t *block_start(uint8_t *framebuf, int stride, int mi_row, int mi_col) {
  return framebuf + (stride * mi_row) + mi_col;
}

void copy_block(uint8_t *dest, int dest_stride,
                uint8_t *src, int src_stride, BLOCK_SIZE bs) {
  int r, c;
  for (r = 0; r < heights[bs]; ++r) {
    for (c = 0; c < widths[bs]; ++c) {
      dest[c] = src[c];
    }
    dest += dest_stride;
    src += src_stride;
  }
  return;
}

void vp9_denoiser_denoise(VP9_DENOISER *denoiser,
                          MACROBLOCK *mb, MODE_INFO **grid,
                          int mi_row, int mi_col, BLOCK_SIZE bs) {
  int decision = COPY_BLOCK;

  YV12_BUFFER_CONFIG avg = denoiser->running_avg_y[INTRA_FRAME];
  struct buf_2d src = mb->plane[0].src;

  update_running_avg(denoiser->mc_running_avg_y.y_buffer,
                     denoiser->mc_running_avg_y.y_stride,
                     denoiser->running_avg_y[INTRA_FRAME].y_buffer,
                     denoiser->running_avg_y[INTRA_FRAME].y_stride,
                     mb->plane[0].src.buf, mb->plane[0].src.stride, bs);

  if (decision == FILTER_BLOCK) {
  }
  if (decision == COPY_BLOCK) {
    copy_block(block_start(avg.y_buffer, avg.y_stride, mi_row, mi_col),
               avg.y_stride,
               block_start(src.buf, src.stride, mi_row, mi_col),
               src.stride,
               bs);
  }
  return;
}

void copy_frame(YV12_BUFFER_CONFIG dest, YV12_BUFFER_CONFIG src) {
  memcpy(dest.buffer_alloc, src.buffer_alloc, src.buffer_alloc_sz);
}

void vp9_denoiser_update_frame_info(VP9_DENOISER *denoiser,
                                    YV12_BUFFER_CONFIG src,
                                    FRAME_TYPE frame_type,
                                    int refresh_alt_ref_frame,
                                    int refresh_golden_frame,
                                    int refresh_last_frame) {
  int i;
  if (frame_type == KEY_FRAME) {
    copy_frame(denoiser->running_avg_y[LAST_FRAME], src);
    for (i = 2; i < MAX_REF_FRAMES - 1; i++) {
      copy_frame(denoiser->running_avg_y[i],
                 denoiser->running_avg_y[LAST_FRAME]);
    }
  } else { /* For non key frames */
    if (refresh_alt_ref_frame) {
      copy_frame(denoiser->running_avg_y[ALTREF_FRAME],
                 denoiser->running_avg_y[INTRA_FRAME]);
    }
    if (refresh_golden_frame) {
      copy_frame(denoiser->running_avg_y[GOLDEN_FRAME],
                 denoiser->running_avg_y[INTRA_FRAME]);
    }
    if (refresh_last_frame) {
      copy_frame(denoiser->running_avg_y[LAST_FRAME],
                 denoiser->running_avg_y[INTRA_FRAME]);
    }
  }

  return;
}

void vp9_denoiser_update_frame_stats() {
  return;
}

int vp9_denoiser_alloc(VP9_DENOISER *denoiser, int width, int height,
                       int ssx, int ssy, int border) {
  int i, fail;
  assert(denoiser);

  for (i = 0; i < MAX_REF_FRAMES; ++i) {
    fail = vp9_alloc_frame_buffer(&denoiser->running_avg_y[i], width, height,
                                ssx, ssy, border);
    if (fail) {
      vp9_denoiser_free(denoiser);
      return 1;
    }
  }

  fail = vp9_alloc_frame_buffer(&denoiser->mc_running_avg_y, width, height,
                              ssx, ssy, border);
  if (fail) {
    vp9_denoiser_free(denoiser);
    return 1;
  }

  return 0;
}

void vp9_denoiser_free(VP9_DENOISER *denoiser) {
  int i;
  for (i = 0; i < MAX_REF_FRAMES; ++i) {
    if (&denoiser->running_avg_y[i] != NULL) {
      vp9_free_frame_buffer(&denoiser->running_avg_y[i]);
    }
  }
  if (&denoiser->mc_running_avg_y != NULL) {
    vp9_free_frame_buffer(&denoiser->mc_running_avg_y);
  }
  return;
}
