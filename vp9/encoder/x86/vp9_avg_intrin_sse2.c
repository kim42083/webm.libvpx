/*
 *  Copyright (c) 2014 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include <emmintrin.h>
#include "vpx_ports/mem.h"


unsigned int vp9_avg_8x8_sse2(const uint8_t *s, int p) {
  __m128i s0, s1, u0;
  unsigned int avg = 0;
  u0  = _mm_setzero_si128();
  s0 = _mm_unpacklo_epi8(_mm_loadl_epi64((const __m128i *)(s)), u0);
  s1 = _mm_unpacklo_epi8(_mm_loadl_epi64((const __m128i *)(s + p)), u0);
  s0 = _mm_adds_epu16(s0, s1);
  s1 = _mm_unpacklo_epi8(_mm_loadl_epi64((const __m128i *)(s + 2 * p)), u0);
  s0 = _mm_adds_epu16(s0, s1);
  s1 = _mm_unpacklo_epi8(_mm_loadl_epi64((const __m128i *)(s + 3 * p)), u0);
  s0 = _mm_adds_epu16(s0, s1);
  s1 = _mm_unpacklo_epi8(_mm_loadl_epi64((const __m128i *)(s + 4 * p)), u0);
  s0 = _mm_adds_epu16(s0, s1);
  s1 = _mm_unpacklo_epi8(_mm_loadl_epi64((const __m128i *)(s + 5 * p)), u0);
  s0 = _mm_adds_epu16(s0, s1);
  s1 = _mm_unpacklo_epi8(_mm_loadl_epi64((const __m128i *)(s + 6 * p)), u0);
  s0 = _mm_adds_epu16(s0, s1);
  s1 = _mm_unpacklo_epi8(_mm_loadl_epi64((const __m128i *)(s + 7 * p)), u0);
  s0 = _mm_adds_epu16(s0, s1);

  s0 = _mm_adds_epu16(s0, _mm_srli_si128(s0, 8));
  s0 = _mm_adds_epu16(s0, _mm_srli_epi64(s0, 32));
  s0 = _mm_adds_epu16(s0, _mm_srli_epi64(s0, 16));
  avg = _mm_extract_epi16(s0, 0);
  return (avg + 32) >> 6;
}

unsigned int vp9_avg_4x4_sse2(const uint8_t *s, int p) {
  __m128i s0, s1, u0;
  unsigned int avg = 0;
  u0  = _mm_setzero_si128();
  s0 = _mm_unpacklo_epi8(_mm_loadl_epi64((const __m128i *)(s)), u0);
  s1 = _mm_unpacklo_epi8(_mm_loadl_epi64((const __m128i *)(s + p)), u0);
  s0 = _mm_adds_epu16(s0, s1);
  s1 = _mm_unpacklo_epi8(_mm_loadl_epi64((const __m128i *)(s + 2 * p)), u0);
  s0 = _mm_adds_epu16(s0, s1);
  s1 = _mm_unpacklo_epi8(_mm_loadl_epi64((const __m128i *)(s + 3 * p)), u0);
  s0 = _mm_adds_epu16(s0, s1);

  s0 = _mm_adds_epu16(s0, _mm_srli_si128(s0, 4));
  s0 = _mm_adds_epu16(s0, _mm_srli_epi64(s0, 16));
  avg = _mm_extract_epi16(s0, 0);
  return (avg + 8) >> 4;
}

static void hadamard_col8_sse2(__m128i *in, int iter) {
  __m128i a0 = in[0];
  __m128i a1 = in[1];
  __m128i a2 = in[2];
  __m128i a3 = in[3];
  __m128i a4 = in[4];
  __m128i a5 = in[5];
  __m128i a6 = in[6];
  __m128i a7 = in[7];

  __m128i b0 = _mm_add_epi16(a0, a1);
  __m128i b1 = _mm_sub_epi16(a0, a1);
  __m128i b2 = _mm_add_epi16(a2, a3);
  __m128i b3 = _mm_sub_epi16(a2, a3);
  __m128i b4 = _mm_add_epi16(a4, a5);
  __m128i b5 = _mm_sub_epi16(a4, a5);
  __m128i b6 = _mm_add_epi16(a6, a7);
  __m128i b7 = _mm_sub_epi16(a6, a7);

  a0 = _mm_add_epi16(b0, b2);
  a1 = _mm_add_epi16(b1, b3);
  a2 = _mm_sub_epi16(b0, b2);
  a3 = _mm_sub_epi16(b1, b3);
  a4 = _mm_add_epi16(b4, b6);
  a5 = _mm_add_epi16(b5, b7);
  a6 = _mm_sub_epi16(b4, b6);
  a7 = _mm_sub_epi16(b5, b7);

  if (iter == 0) {
    b0 = _mm_add_epi16(a0, a4);
    b1 = _mm_add_epi16(a1, a5);
    b2 = _mm_add_epi16(a2, a6);
    b3 = _mm_add_epi16(a3, a7);
    b4 = _mm_sub_epi16(a0, a4);
    b5 = _mm_sub_epi16(a1, a5);
    b6 = _mm_sub_epi16(a2, a6);
    b7 = _mm_sub_epi16(a3, a7);

    a0 = _mm_unpacklo_epi16(b0, b1);
    a1 = _mm_unpacklo_epi16(b2, b3);
    a2 = _mm_unpackhi_epi16(b0, b1);
    a3 = _mm_unpackhi_epi16(b2, b3);
    a4 = _mm_unpacklo_epi16(b4, b5);
    a5 = _mm_unpacklo_epi16(b6, b7);
    a6 = _mm_unpackhi_epi16(b4, b5);
    a7 = _mm_unpackhi_epi16(b6, b7);

    b0 = _mm_unpacklo_epi32(a0, a1);
    b1 = _mm_unpacklo_epi32(a4, a5);
    b2 = _mm_unpackhi_epi32(a0, a1);
    b3 = _mm_unpackhi_epi32(a4, a5);
    b4 = _mm_unpacklo_epi32(a2, a3);
    b5 = _mm_unpacklo_epi32(a6, a7);
    b6 = _mm_unpackhi_epi32(a2, a3);
    b7 = _mm_unpackhi_epi32(a6, a7);

    in[0] = _mm_unpacklo_epi64(b0, b1);
    in[7] = _mm_unpackhi_epi64(b0, b1);
    in[3] = _mm_unpacklo_epi64(b2, b3);
    in[4] = _mm_unpackhi_epi64(b2, b3);
    in[2] = _mm_unpacklo_epi64(b4, b5);
    in[6] = _mm_unpackhi_epi64(b4, b5);
    in[1] = _mm_unpacklo_epi64(b6, b7);
    in[5] = _mm_unpackhi_epi64(b6, b7);
  } else {
    in[0] = _mm_add_epi16(a0, a4);
    in[7] = _mm_add_epi16(a1, a5);
    in[3] = _mm_add_epi16(a2, a6);
    in[4] = _mm_add_epi16(a3, a7);
    in[2] = _mm_sub_epi16(a0, a4);
    in[6] = _mm_sub_epi16(a1, a5);
    in[1] = _mm_sub_epi16(a2, a6);
    in[5] = _mm_sub_epi16(a3, a7);
  }
}

void vp9_hadamard_8x8_sse2(int16_t const *src_diff, int src_stride,
                           int16_t *coeff) {
  __m128i src[8];
  src[0] = _mm_load_si128((const __m128i *)src_diff);
  src[1] = _mm_load_si128((const __m128i *)(src_diff += src_stride));
  src[2] = _mm_load_si128((const __m128i *)(src_diff += src_stride));
  src[3] = _mm_load_si128((const __m128i *)(src_diff += src_stride));
  src[4] = _mm_load_si128((const __m128i *)(src_diff += src_stride));
  src[5] = _mm_load_si128((const __m128i *)(src_diff += src_stride));
  src[6] = _mm_load_si128((const __m128i *)(src_diff += src_stride));
  src[7] = _mm_load_si128((const __m128i *)(src_diff += src_stride));

  hadamard_col8_sse2(src, 0);
  hadamard_col8_sse2(src, 1);

  _mm_storeu_si128((__m128i *)coeff, src[0]);
  coeff += 8;
  _mm_storeu_si128((__m128i *)coeff, src[1]);
  coeff += 8;
  _mm_storeu_si128((__m128i *)coeff, src[2]);
  coeff += 8;
  _mm_storeu_si128((__m128i *)coeff, src[3]);
  coeff += 8;
  _mm_storeu_si128((__m128i *)coeff, src[4]);
  coeff += 8;
  _mm_storeu_si128((__m128i *)coeff, src[5]);
  coeff += 8;
  _mm_storeu_si128((__m128i *)coeff, src[6]);
  coeff += 8;
  _mm_storeu_si128((__m128i *)coeff, src[7]);
}

int16_t vp9_satd_sse2(const int16_t *coeff, int length) {
  int i;
  __m128i sum = _mm_load_si128((const __m128i *)coeff);
  __m128i sign = _mm_srai_epi16(sum, 15);
  __m128i val = _mm_xor_si128(sum, sign);
  sum = _mm_sub_epi16(val, sign);
  coeff += 8;

  for (i = 8; i < length; i += 8) {
    __m128i src_line = _mm_load_si128((const __m128i *)coeff);
    sign = _mm_srai_epi16(src_line, 15);
    val = _mm_xor_si128(src_line, sign);
    val = _mm_sub_epi16(val, sign);
    sum = _mm_add_epi16(sum, val);
    coeff += 8;
  }

  val = _mm_srli_si128(sum, 8);
  sum = _mm_add_epi16(sum, val);
  val = _mm_srli_epi64(sum, 32);
  sum = _mm_add_epi16(sum, val);
  val = _mm_srli_epi32(sum, 16);
  sum = _mm_add_epi16(sum, val);

  return _mm_extract_epi16(sum, 0);
}

void vp9_int_pro_row_sse2(int16_t *hbuf, uint8_t const*ref,
                          const int ref_stride, const int height) {
  int idx;
  __m128i zero = _mm_setzero_si128();
  __m128i src_line = _mm_loadu_si128((const __m128i *)ref);
  __m128i s0 = _mm_unpacklo_epi8(src_line, zero);
  __m128i s1 = _mm_unpackhi_epi8(src_line, zero);
  __m128i t0, t1;
  int height_1 = height - 1;
  ref += ref_stride;

  for (idx = 1; idx < height_1; idx += 2) {
    src_line = _mm_loadu_si128((const __m128i *)ref);
    t0 = _mm_unpacklo_epi8(src_line, zero);
    t1 = _mm_unpackhi_epi8(src_line, zero);
    s0 = _mm_adds_epu16(s0, t0);
    s1 = _mm_adds_epu16(s1, t1);
    ref += ref_stride;

    src_line = _mm_loadu_si128((const __m128i *)ref);
    t0 = _mm_unpacklo_epi8(src_line, zero);
    t1 = _mm_unpackhi_epi8(src_line, zero);
    s0 = _mm_adds_epu16(s0, t0);
    s1 = _mm_adds_epu16(s1, t1);
    ref += ref_stride;
  }

  src_line = _mm_loadu_si128((const __m128i *)ref);
  t0 = _mm_unpacklo_epi8(src_line, zero);
  t1 = _mm_unpackhi_epi8(src_line, zero);
  s0 = _mm_adds_epu16(s0, t0);
  s1 = _mm_adds_epu16(s1, t1);

  if (height == 64) {
    s0 = _mm_srai_epi16(s0, 5);
    s1 = _mm_srai_epi16(s1, 5);
  } else if (height == 32) {
    s0 = _mm_srai_epi16(s0, 4);
    s1 = _mm_srai_epi16(s1, 4);
  } else {
    s0 = _mm_srai_epi16(s0, 3);
    s1 = _mm_srai_epi16(s1, 3);
  }

  _mm_storeu_si128((__m128i *)hbuf, s0);
  hbuf += 8;
  _mm_storeu_si128((__m128i *)hbuf, s1);
}

int16_t vp9_int_pro_col_sse2(uint8_t const *ref, const int width) {
  __m128i zero = _mm_setzero_si128();
  __m128i src_line = _mm_load_si128((const __m128i *)ref);
  __m128i s0 = _mm_sad_epu8(src_line, zero);
  __m128i s1;
  int i;

  for (i = 16; i < width; i += 16) {
    ref += 16;
    src_line = _mm_load_si128((const __m128i *)ref);
    s1 = _mm_sad_epu8(src_line, zero);
    s0 = _mm_adds_epu16(s0, s1);
  }

  s1 = _mm_srli_si128(s0, 8);
  s0 = _mm_adds_epu16(s0, s1);

  return _mm_extract_epi16(s0, 0);
}

int vp9_vector_var_sse2(int16_t const *ref, int16_t const *src,
                        const int bwl) {
  int idx;
  int width = 4 << bwl;
  int16_t mean;
  __m128i v0 = _mm_loadu_si128((const __m128i *)ref);
  __m128i v1 = _mm_load_si128((const __m128i *)src);
  __m128i diff = _mm_subs_epi16(v0, v1);
  __m128i sum = diff;
  __m128i sse = _mm_madd_epi16(diff, diff);

  ref += 8;
  src += 8;

  for (idx = 8; idx < width; idx += 8) {
    v0 = _mm_loadu_si128((const __m128i *)ref);
    v1 = _mm_load_si128((const __m128i *)src);
    diff = _mm_subs_epi16(v0, v1);

    sum = _mm_add_epi16(sum, diff);
    v0  = _mm_madd_epi16(diff, diff);
    sse = _mm_add_epi32(sse, v0);

    ref += 8;
    src += 8;
  }

  v0  = _mm_srli_si128(sum, 8);
  sum = _mm_add_epi16(sum, v0);
  v0  = _mm_srli_epi64(sum, 32);
  sum = _mm_add_epi16(sum, v0);
  v0  = _mm_srli_epi32(sum, 16);
  sum = _mm_add_epi16(sum, v0);

  v1  = _mm_srli_si128(sse, 8);
  sse = _mm_add_epi32(sse, v1);
  v1  = _mm_srli_epi64(sse, 32);
  sse = _mm_add_epi32(sse, v1);

  mean = _mm_extract_epi16(sum, 0);

  return _mm_cvtsi128_si32(sse) - ((mean * mean) >> (bwl + 2));
}
