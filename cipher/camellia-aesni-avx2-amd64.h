/* camellia-aesni-avx2-amd64.h - AES-NI/VAES/GFNI/AVX2 implementation of Camellia
 *
 * Copyright (C) 2013-2015,2020-2023 Jussi Kivilinna <jussi.kivilinna@iki.fi>
 *
 * This file is part of Libgcrypt.
 *
 * Libgcrypt is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * Libgcrypt is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#ifndef GCRY_CAMELLIA_AESNI_AVX2_AMD64_H
#define GCRY_CAMELLIA_AESNI_AVX2_AMD64_H

#include "asm-common-amd64.h"

#define CAMELLIA_TABLE_BYTE_LEN 272

/* struct CAMELLIA_context: */
#define key_table 0
#define key_bitlength CAMELLIA_TABLE_BYTE_LEN

/* register macros */
#define CTX %rdi
#define RIO %r8

/**********************************************************************
  helper macros
 **********************************************************************/

#ifndef CAMELLIA_GFNI_BUILD
#define filter_8bit(x, lo_t, hi_t, mask4bit, tmp0) \
	vpand x, mask4bit, tmp0; \
	vpandn x, mask4bit, x; \
	vpsrld $4, x, x; \
	\
	vpshufb tmp0, lo_t, tmp0; \
	vpshufb x, hi_t, x; \
	vpxor tmp0, x, x;
#endif

#define ymm0_x xmm0
#define ymm1_x xmm1
#define ymm2_x xmm2
#define ymm3_x xmm3
#define ymm4_x xmm4
#define ymm5_x xmm5
#define ymm6_x xmm6
#define ymm7_x xmm7
#define ymm8_x xmm8
#define ymm9_x xmm9
#define ymm10_x xmm10
#define ymm11_x xmm11
#define ymm12_x xmm12
#define ymm13_x xmm13
#define ymm14_x xmm14
#define ymm15_x xmm15

#ifdef CAMELLIA_VAES_BUILD
# define IF_AESNI(...)
# define IF_VAES(...) __VA_ARGS__
#else
# define IF_AESNI(...) __VA_ARGS__
# define IF_VAES(...)
#endif

#ifdef CAMELLIA_GFNI_BUILD
# define IF_GFNI(...) __VA_ARGS__
# define IF_NOT_GFNI(...)
#else
# define IF_GFNI(...)
# define IF_NOT_GFNI(...) __VA_ARGS__
#endif

/**********************************************************************
  GFNI helper macros and constants
 **********************************************************************/

#ifdef CAMELLIA_GFNI_BUILD

#define BV8(a0,a1,a2,a3,a4,a5,a6,a7) \
	( (((a0) & 1) << 0) | \
	  (((a1) & 1) << 1) | \
	  (((a2) & 1) << 2) | \
	  (((a3) & 1) << 3) | \
	  (((a4) & 1) << 4) | \
	  (((a5) & 1) << 5) | \
	  (((a6) & 1) << 6) | \
	  (((a7) & 1) << 7) )

#define BM8X8(l0,l1,l2,l3,l4,l5,l6,l7) \
	( ((l7) << (0 * 8)) | \
	  ((l6) << (1 * 8)) | \
	  ((l5) << (2 * 8)) | \
	  ((l4) << (3 * 8)) | \
	  ((l3) << (4 * 8)) | \
	  ((l2) << (5 * 8)) | \
	  ((l1) << (6 * 8)) | \
	  ((l0) << (7 * 8)) )

/* Pre-filters and post-filters constants for Camellia sboxes s1, s2, s3 and s4.
 *   See http://urn.fi/URN:NBN:fi:oulu-201305311409, pages 43-48.
 *
 * Pre-filters are directly from above source, "θ₁"/"θ₄". Post-filters are
 * combination of function "A" (AES SubBytes affine transformation) and
 * "ψ₁"/"ψ₂"/"ψ₃".
 */

/* Constant from "θ₁(x)" and "θ₄(x)" functions. */
#define pre_filter_constant_s1234 BV8(1, 0, 1, 0, 0, 0, 1, 0)

/* Constant from "ψ₁(A(x))" function: */
#define post_filter_constant_s14  BV8(0, 1, 1, 1, 0, 1, 1, 0)

/* Constant from "ψ₂(A(x))" function: */
#define post_filter_constant_s2   BV8(0, 0, 1, 1, 1, 0, 1, 1)

/* Constant from "ψ₃(A(x))" function: */
#define post_filter_constant_s3   BV8(1, 1, 1, 0, 1, 1, 0, 0)

#endif /* CAMELLIA_GFNI_BUILD */

/**********************************************************************
  32-way camellia
 **********************************************************************/

#ifdef CAMELLIA_GFNI_BUILD

/* roundsm32 (GFNI version)
 * IN:
 *   x0..x7: byte-sliced AB state
 *   mem_cd: register pointer storing CD state
 *   key: index for key material
 * OUT:
 *   x0..x7: new byte-sliced CD state
 */
#define roundsm32(x0, x1, x2, x3, x4, x5, x6, x7, t0, t1, t2, t3, t4, t5, \
		  t6, t7, mem_cd, key) \
	/* \
	 * S-function with AES subbytes \
	 */ \
	vpbroadcastq .Lpre_filter_bitmatrix_s123 rRIP, t5; \
	vpbroadcastq .Lpre_filter_bitmatrix_s4 rRIP, t2; \
	vpbroadcastq .Lpost_filter_bitmatrix_s14 rRIP, t4; \
	vpbroadcastq .Lpost_filter_bitmatrix_s2 rRIP, t3; \
	vpbroadcastq .Lpost_filter_bitmatrix_s3 rRIP, t6; \
	\
	/* prefilter sboxes */ \
	vgf2p8affineqb $(pre_filter_constant_s1234), t5, x0, x0; \
	vgf2p8affineqb $(pre_filter_constant_s1234), t5, x7, x7; \
	vgf2p8affineqb $(pre_filter_constant_s1234), t2, x3, x3; \
	vgf2p8affineqb $(pre_filter_constant_s1234), t2, x6, x6; \
	vgf2p8affineqb $(pre_filter_constant_s1234), t5, x2, x2; \
	vgf2p8affineqb $(pre_filter_constant_s1234), t5, x5, x5; \
	vgf2p8affineqb $(pre_filter_constant_s1234), t5, x1, x1; \
	vgf2p8affineqb $(pre_filter_constant_s1234), t5, x4, x4; \
	\
	/* sbox GF8 inverse + postfilter sboxes 1 and 4 */ \
	vgf2p8affineinvqb $(post_filter_constant_s14), t4, x0, x0; \
	vgf2p8affineinvqb $(post_filter_constant_s14), t4, x7, x7; \
	vgf2p8affineinvqb $(post_filter_constant_s14), t4, x3, x3; \
	vgf2p8affineinvqb $(post_filter_constant_s14), t4, x6, x6; \
	\
	/* sbox GF8 inverse + postfilter sbox 3 */ \
	vgf2p8affineinvqb $(post_filter_constant_s3), t6, x2, x2; \
	vgf2p8affineinvqb $(post_filter_constant_s3), t6, x5, x5; \
	\
	/* sbox GF8 inverse + postfilter sbox 2 */ \
	vgf2p8affineinvqb $(post_filter_constant_s2), t3, x1, x1; \
	vgf2p8affineinvqb $(post_filter_constant_s2), t3, x4, x4; \
	\
	vpbroadcastb 7+key, t7; \
	vpbroadcastb 6+key, t6; \
	\
	/* P-function */ \
	vpxor x5, x0, x0; \
	vpxor x6, x1, x1; \
	vpxor x7, x2, x2; \
	vpxor x4, x3, x3; \
	\
	vpbroadcastb 5+key, t5; \
	vpbroadcastb 4+key, t4; \
	\
	vpxor x2, x4, x4; \
	vpxor x3, x5, x5; \
	vpxor x0, x6, x6; \
	vpxor x1, x7, x7; \
	\
	vpbroadcastb 3+key, t3; \
	vpbroadcastb 2+key, t2; \
	\
	vpxor x7, x0, x0; \
	vpxor x4, x1, x1; \
	vpxor x5, x2, x2; \
	vpxor x6, x3, x3; \
	\
	vpbroadcastb 1+key, t1; \
	vpbroadcastb 0+key, t0; \
	\
	vpxor x3, x4, x4; \
	vpxor x0, x5, x5; \
	vpxor x1, x6, x6; \
	vpxor x2, x7, x7; /* note: high and low parts swapped */ \
	\
	/* Add key material and result to CD (x becomes new CD) */ \
	\
	vpxor t7, x0, x0; \
	vpxor 4 * 32(mem_cd), x0, x0; \
	\
	vpxor t6, x1, x1; \
	vpxor 5 * 32(mem_cd), x1, x1; \
	\
	vpxor t5, x2, x2; \
	vpxor 6 * 32(mem_cd), x2, x2; \
	\
	vpxor t4, x3, x3; \
	vpxor 7 * 32(mem_cd), x3, x3; \
	\
	vpxor t3, x4, x4; \
	vpxor 0 * 32(mem_cd), x4, x4; \
	\
	vpxor t2, x5, x5; \
	vpxor 1 * 32(mem_cd), x5, x5; \
	\
	vpxor t1, x6, x6; \
	vpxor 2 * 32(mem_cd), x6, x6; \
	\
	vpxor t0, x7, x7; \
	vpxor 3 * 32(mem_cd), x7, x7;

#else /* CAMELLIA_GFNI_BUILD */

/* roundsm32 (AES-NI / VAES version)
 * IN:
 *   x0..x7: byte-sliced AB state
 *   mem_cd: register pointer storing CD state
 *   key: index for key material
 * OUT:
 *   x0..x7: new byte-sliced CD state
 */
#define roundsm32(x0, x1, x2, x3, x4, x5, x6, x7, t0, t1, t2, t3, t4, t5, \
		  t6, t7, mem_cd, key) \
	/* \
	 * S-function with AES subbytes \
	 */ \
	vbroadcasti128 .Linv_shift_row rRIP, t4; \
	vpbroadcastd .L0f0f0f0f rRIP, t7; \
	vbroadcasti128 .Lpre_tf_lo_s1 rRIP, t5; \
	vbroadcasti128 .Lpre_tf_hi_s1 rRIP, t6; \
	vbroadcasti128 .Lpre_tf_lo_s4 rRIP, t2; \
	vbroadcasti128 .Lpre_tf_hi_s4 rRIP, t3; \
	\
	/* AES inverse shift rows */ \
	vpshufb t4, x0, x0; \
	vpshufb t4, x7, x7; \
	vpshufb t4, x3, x3; \
	vpshufb t4, x6, x6; \
	vpshufb t4, x2, x2; \
	vpshufb t4, x5, x5; \
	vpshufb t4, x1, x1; \
	vpshufb t4, x4, x4; \
	\
	/* prefilter sboxes 1, 2 and 3 */ \
	/* prefilter sbox 4 */ \
	filter_8bit(x0, t5, t6, t7, t4); \
	filter_8bit(x7, t5, t6, t7, t4); \
	IF_AESNI(vextracti128 $1, x0, t0##_x); \
	IF_AESNI(vextracti128 $1, x7, t1##_x); \
	filter_8bit(x3, t2, t3, t7, t4); \
	filter_8bit(x6, t2, t3, t7, t4); \
	IF_AESNI(vextracti128 $1, x3, t3##_x); \
	IF_AESNI(vextracti128 $1, x6, t2##_x); \
	filter_8bit(x2, t5, t6, t7, t4); \
	filter_8bit(x5, t5, t6, t7, t4); \
	filter_8bit(x1, t5, t6, t7, t4); \
	filter_8bit(x4, t5, t6, t7, t4); \
	\
	vpxor t4, t4, t4; \
	\
	/* AES subbytes + AES shift rows */ \
	IF_AESNI(vextracti128 $1, x2, t6##_x; \
		 vextracti128 $1, x5, t5##_x; \
		 vaesenclast t4##_x, x0##_x, x0##_x; \
		 vaesenclast t4##_x, t0##_x, t0##_x; \
		 vaesenclast t4##_x, x7##_x, x7##_x; \
		 vaesenclast t4##_x, t1##_x, t1##_x; \
		 vaesenclast t4##_x, x3##_x, x3##_x; \
		 vaesenclast t4##_x, t3##_x, t3##_x; \
		 vaesenclast t4##_x, x6##_x, x6##_x; \
		 vaesenclast t4##_x, t2##_x, t2##_x; \
		 vinserti128 $1, t0##_x, x0, x0; \
		 vinserti128 $1, t1##_x, x7, x7; \
		 vinserti128 $1, t3##_x, x3, x3; \
		 vinserti128 $1, t2##_x, x6, x6; \
		 vextracti128 $1, x1, t3##_x; \
		 vextracti128 $1, x4, t2##_x); \
	vbroadcasti128 .Lpost_tf_lo_s1 rRIP, t0; \
	vbroadcasti128 .Lpost_tf_hi_s1 rRIP, t1; \
	IF_AESNI(vaesenclast t4##_x, x2##_x, x2##_x; \
		 vaesenclast t4##_x, t6##_x, t6##_x; \
		 vaesenclast t4##_x, x5##_x, x5##_x; \
		 vaesenclast t4##_x, t5##_x, t5##_x; \
		 vaesenclast t4##_x, x1##_x, x1##_x; \
		 vaesenclast t4##_x, t3##_x, t3##_x; \
		 vaesenclast t4##_x, x4##_x, x4##_x; \
		 vaesenclast t4##_x, t2##_x, t2##_x; \
		 vinserti128 $1, t6##_x, x2, x2; \
		 vinserti128 $1, t5##_x, x5, x5; \
		 vinserti128 $1, t3##_x, x1, x1; \
		 vinserti128 $1, t2##_x, x4, x4); \
	IF_VAES(vaesenclast t4, x0, x0; \
		vaesenclast t4, x7, x7; \
		vaesenclast t4, x3, x3; \
		vaesenclast t4, x6, x6; \
		vaesenclast t4, x2, x2; \
		vaesenclast t4, x5, x5; \
		vaesenclast t4, x1, x1; \
		vaesenclast t4, x4, x4); \
	\
	/* postfilter sboxes 1 and 4 */ \
	vbroadcasti128 .Lpost_tf_lo_s3 rRIP, t2; \
	vbroadcasti128 .Lpost_tf_hi_s3 rRIP, t3; \
	filter_8bit(x0, t0, t1, t7, t4); \
	filter_8bit(x7, t0, t1, t7, t4); \
	filter_8bit(x3, t0, t1, t7, t6); \
	filter_8bit(x6, t0, t1, t7, t6); \
	\
	/* postfilter sbox 3 */ \
	vbroadcasti128 .Lpost_tf_lo_s2 rRIP, t4; \
	vbroadcasti128 .Lpost_tf_hi_s2 rRIP, t5; \
	filter_8bit(x2, t2, t3, t7, t6); \
	filter_8bit(x5, t2, t3, t7, t6); \
	\
	/* postfilter sbox 2 */ \
	filter_8bit(x1, t4, t5, t7, t2); \
	filter_8bit(x4, t4, t5, t7, t2); \
	\
	vpbroadcastb 7+key, t7; \
	vpbroadcastb 6+key, t6; \
	\
	/* P-function */ \
	vpxor x5, x0, x0; \
	vpxor x6, x1, x1; \
	vpxor x7, x2, x2; \
	vpxor x4, x3, x3; \
	\
	vpbroadcastb 5+key, t5; \
	vpbroadcastb 4+key, t4; \
	\
	vpxor x2, x4, x4; \
	vpxor x3, x5, x5; \
	vpxor x0, x6, x6; \
	vpxor x1, x7, x7; \
	\
	vpbroadcastb 3+key, t3; \
	vpbroadcastb 2+key, t2; \
	\
	vpxor x7, x0, x0; \
	vpxor x4, x1, x1; \
	vpxor x5, x2, x2; \
	vpxor x6, x3, x3; \
	\
	vpbroadcastb 1+key, t1; \
	vpbroadcastb 0+key, t0; \
	\
	vpxor x3, x4, x4; \
	vpxor x0, x5, x5; \
	vpxor x1, x6, x6; \
	vpxor x2, x7, x7; /* note: high and low parts swapped */ \
	\
	/* Add key material and result to CD (x becomes new CD) */ \
	\
	vpxor t7, x0, x0; \
	vpxor 4 * 32(mem_cd), x0, x0; \
	\
	vpxor t6, x1, x1; \
	vpxor 5 * 32(mem_cd), x1, x1; \
	\
	vpxor t5, x2, x2; \
	vpxor 6 * 32(mem_cd), x2, x2; \
	\
	vpxor t4, x3, x3; \
	vpxor 7 * 32(mem_cd), x3, x3; \
	\
	vpxor t3, x4, x4; \
	vpxor 0 * 32(mem_cd), x4, x4; \
	\
	vpxor t2, x5, x5; \
	vpxor 1 * 32(mem_cd), x5, x5; \
	\
	vpxor t1, x6, x6; \
	vpxor 2 * 32(mem_cd), x6, x6; \
	\
	vpxor t0, x7, x7; \
	vpxor 3 * 32(mem_cd), x7, x7;

#endif /* CAMELLIA_GFNI_BUILD */

/*
 * IN/OUT:
 *  x0..x7: byte-sliced AB state preloaded
 *  mem_ab: byte-sliced AB state in memory
 *  mem_cb: byte-sliced CD state in memory
 */
#define two_roundsm32(x0, x1, x2, x3, x4, x5, x6, x7, y0, y1, y2, y3, y4, y5, \
		      y6, y7, mem_ab, mem_cd, i, dir, store_ab) \
	roundsm32(x0, x1, x2, x3, x4, x5, x6, x7, y0, y1, y2, y3, y4, y5, \
		  y6, y7, mem_cd, (key_table + (i) * 8)(CTX)); \
	\
	vmovdqu x0, 4 * 32(mem_cd); \
	vmovdqu x1, 5 * 32(mem_cd); \
	vmovdqu x2, 6 * 32(mem_cd); \
	vmovdqu x3, 7 * 32(mem_cd); \
	vmovdqu x4, 0 * 32(mem_cd); \
	vmovdqu x5, 1 * 32(mem_cd); \
	vmovdqu x6, 2 * 32(mem_cd); \
	vmovdqu x7, 3 * 32(mem_cd); \
	\
	roundsm32(x4, x5, x6, x7, x0, x1, x2, x3, y0, y1, y2, y3, y4, y5, \
		  y6, y7, mem_ab, (key_table + ((i) + (dir)) * 8)(CTX)); \
	\
	store_ab(x0, x1, x2, x3, x4, x5, x6, x7, mem_ab);

#define dummy_store(x0, x1, x2, x3, x4, x5, x6, x7, mem_ab) /* do nothing */

#define store_ab_state(x0, x1, x2, x3, x4, x5, x6, x7, mem_ab) \
	/* Store new AB state */ \
	vmovdqu x4, 4 * 32(mem_ab); \
	vmovdqu x5, 5 * 32(mem_ab); \
	vmovdqu x6, 6 * 32(mem_ab); \
	vmovdqu x7, 7 * 32(mem_ab); \
	vmovdqu x0, 0 * 32(mem_ab); \
	vmovdqu x1, 1 * 32(mem_ab); \
	vmovdqu x2, 2 * 32(mem_ab); \
	vmovdqu x3, 3 * 32(mem_ab);

#define enc_rounds32(x0, x1, x2, x3, x4, x5, x6, x7, y0, y1, y2, y3, y4, y5, \
		      y6, y7, mem_ab, mem_cd, i) \
	two_roundsm32(x0, x1, x2, x3, x4, x5, x6, x7, y0, y1, y2, y3, y4, y5, \
		      y6, y7, mem_ab, mem_cd, (i) + 2, 1, store_ab_state); \
	two_roundsm32(x0, x1, x2, x3, x4, x5, x6, x7, y0, y1, y2, y3, y4, y5, \
		      y6, y7, mem_ab, mem_cd, (i) + 4, 1, store_ab_state); \
	two_roundsm32(x0, x1, x2, x3, x4, x5, x6, x7, y0, y1, y2, y3, y4, y5, \
		      y6, y7, mem_ab, mem_cd, (i) + 6, 1, dummy_store);

#define dec_rounds32(x0, x1, x2, x3, x4, x5, x6, x7, y0, y1, y2, y3, y4, y5, \
		      y6, y7, mem_ab, mem_cd, i) \
	two_roundsm32(x0, x1, x2, x3, x4, x5, x6, x7, y0, y1, y2, y3, y4, y5, \
		      y6, y7, mem_ab, mem_cd, (i) + 7, -1, store_ab_state); \
	two_roundsm32(x0, x1, x2, x3, x4, x5, x6, x7, y0, y1, y2, y3, y4, y5, \
		      y6, y7, mem_ab, mem_cd, (i) + 5, -1, store_ab_state); \
	two_roundsm32(x0, x1, x2, x3, x4, x5, x6, x7, y0, y1, y2, y3, y4, y5, \
		      y6, y7, mem_ab, mem_cd, (i) + 3, -1, dummy_store);

/*
 * IN:
 *  v0..3: byte-sliced 32-bit integers
 * OUT:
 *  v0..3: (IN <<< 1)
 */
#ifdef CAMELLIA_GFNI_BUILD
#define rol32_1_32(v0, v1, v2, v3, t0, t1, t2, right_shift_by_7) \
	vgf2p8affineqb $0, right_shift_by_7, v0, t0; \
	vpaddb v0, v0, v0; \
	\
	vgf2p8affineqb $0, right_shift_by_7, v1, t1; \
	vpaddb v1, v1, v1; \
	\
	vgf2p8affineqb $0, right_shift_by_7, v2, t2; \
	vpaddb v2, v2, v2; \
	\
	vpor t0, v1, v1; \
	\
	vgf2p8affineqb $0, right_shift_by_7, v3, t0; \
	vpaddb v3, v3, v3; \
	\
	vpor t1, v2, v2; \
	vpor t2, v3, v3; \
	vpor t0, v0, v0;
#else
#define rol32_1_32(v0, v1, v2, v3, t0, t1, t2, zero) \
	vpcmpgtb v0, zero, t0; \
	vpaddb v0, v0, v0; \
	vpabsb t0, t0; \
	\
	vpcmpgtb v1, zero, t1; \
	vpaddb v1, v1, v1; \
	vpabsb t1, t1; \
	\
	vpcmpgtb v2, zero, t2; \
	vpaddb v2, v2, v2; \
	vpabsb t2, t2; \
	\
	vpor t0, v1, v1; \
	\
	vpcmpgtb v3, zero, t0; \
	vpaddb v3, v3, v3; \
	vpabsb t0, t0; \
	\
	vpor t1, v2, v2; \
	vpor t2, v3, v3; \
	vpor t0, v0, v0;
#endif

/*
 * IN:
 *   r: byte-sliced AB state in memory
 *   l: byte-sliced CD state in memory
 * OUT:
 *   x0..x7: new byte-sliced CD state
 */
#define fls32(l, l0, l1, l2, l3, l4, l5, l6, l7, r, t0, t1, t2, t3, tt0, \
	      tt1, tt2, tt3, kll, klr, krl, krr) \
	/* \
	 * t0 = kll; \
	 * t0 &= ll; \
	 * lr ^= rol32(t0, 1); \
	 */ \
	IF_NOT_GFNI(vpxor tt0, tt0, tt0); \
	IF_GFNI(vpbroadcastq .Lright_shift_by_7 rRIP, tt0); \
	vpbroadcastb 0+kll, t3; \
	vpbroadcastb 1+kll, t2; \
	vpbroadcastb 2+kll, t1; \
	vpbroadcastb 3+kll, t0; \
	\
	vpand l0, t0, t0; \
	vpand l1, t1, t1; \
	vpand l2, t2, t2; \
	vpand l3, t3, t3; \
	\
	rol32_1_32(t3, t2, t1, t0, tt1, tt2, tt3, tt0); \
	\
	vpxor l4, t0, l4; \
	vmovdqu l4, 4 * 32(l); \
	vpxor l5, t1, l5; \
	vmovdqu l5, 5 * 32(l); \
	vpxor l6, t2, l6; \
	vmovdqu l6, 6 * 32(l); \
	vpxor l7, t3, l7; \
	vmovdqu l7, 7 * 32(l); \
	\
	/* \
	 * t2 = krr; \
	 * t2 |= rr; \
	 * rl ^= t2; \
	 */ \
	\
	vpbroadcastb 0+krr, t3; \
	vpbroadcastb 1+krr, t2; \
	vpbroadcastb 2+krr, t1; \
	vpbroadcastb 3+krr, t0; \
	\
	vpor 4 * 32(r), t0, t0; \
	vpor 5 * 32(r), t1, t1; \
	vpor 6 * 32(r), t2, t2; \
	vpor 7 * 32(r), t3, t3; \
	\
	vpxor 0 * 32(r), t0, t0; \
	vpxor 1 * 32(r), t1, t1; \
	vpxor 2 * 32(r), t2, t2; \
	vpxor 3 * 32(r), t3, t3; \
	vmovdqu t0, 0 * 32(r); \
	vmovdqu t1, 1 * 32(r); \
	vmovdqu t2, 2 * 32(r); \
	vmovdqu t3, 3 * 32(r); \
	\
	/* \
	 * t2 = krl; \
	 * t2 &= rl; \
	 * rr ^= rol32(t2, 1); \
	 */ \
	vpbroadcastb 0+krl, t3; \
	vpbroadcastb 1+krl, t2; \
	vpbroadcastb 2+krl, t1; \
	vpbroadcastb 3+krl, t0; \
	\
	vpand 0 * 32(r), t0, t0; \
	vpand 1 * 32(r), t1, t1; \
	vpand 2 * 32(r), t2, t2; \
	vpand 3 * 32(r), t3, t3; \
	\
	rol32_1_32(t3, t2, t1, t0, tt1, tt2, tt3, tt0); \
	\
	vpxor 4 * 32(r), t0, t0; \
	vpxor 5 * 32(r), t1, t1; \
	vpxor 6 * 32(r), t2, t2; \
	vpxor 7 * 32(r), t3, t3; \
	vmovdqu t0, 4 * 32(r); \
	vmovdqu t1, 5 * 32(r); \
	vmovdqu t2, 6 * 32(r); \
	vmovdqu t3, 7 * 32(r); \
	\
	/* \
	 * t0 = klr; \
	 * t0 |= lr; \
	 * ll ^= t0; \
	 */ \
	\
	vpbroadcastb 0+klr, t3; \
	vpbroadcastb 1+klr, t2; \
	vpbroadcastb 2+klr, t1; \
	vpbroadcastb 3+klr, t0; \
	\
	vpor l4, t0, t0; \
	vpor l5, t1, t1; \
	vpor l6, t2, t2; \
	vpor l7, t3, t3; \
	\
	vpxor l0, t0, l0; \
	vmovdqu l0, 0 * 32(l); \
	vpxor l1, t1, l1; \
	vmovdqu l1, 1 * 32(l); \
	vpxor l2, t2, l2; \
	vmovdqu l2, 2 * 32(l); \
	vpxor l3, t3, l3; \
	vmovdqu l3, 3 * 32(l);

#define transpose_4x4(x0, x1, x2, x3, t1, t2) \
	vpunpckhdq x1, x0, t2; \
	vpunpckldq x1, x0, x0; \
	\
	vpunpckldq x3, x2, t1; \
	vpunpckhdq x3, x2, x2; \
	\
	vpunpckhqdq t1, x0, x1; \
	vpunpcklqdq t1, x0, x0; \
	\
	vpunpckhqdq x2, t2, x3; \
	vpunpcklqdq x2, t2, x2;

#define byteslice_16x16b_fast(a0, b0, c0, d0, a1, b1, c1, d1, a2, b2, c2, d2, \
			      a3, b3, c3, d3, st0, st1) \
	vmovdqu d2, st0; \
	vmovdqu d3, st1; \
	transpose_4x4(a0, a1, a2, a3, d2, d3); \
	transpose_4x4(b0, b1, b2, b3, d2, d3); \
	vmovdqu st0, d2; \
	vmovdqu st1, d3; \
	\
	vmovdqu a0, st0; \
	vmovdqu a1, st1; \
	transpose_4x4(c0, c1, c2, c3, a0, a1); \
	transpose_4x4(d0, d1, d2, d3, a0, a1); \
	\
	vbroadcasti128 .Lshufb_16x16b rRIP, a0; \
	vmovdqu st1, a1; \
	vpshufb a0, a2, a2; \
	vpshufb a0, a3, a3; \
	vpshufb a0, b0, b0; \
	vpshufb a0, b1, b1; \
	vpshufb a0, b2, b2; \
	vpshufb a0, b3, b3; \
	vpshufb a0, a1, a1; \
	vpshufb a0, c0, c0; \
	vpshufb a0, c1, c1; \
	vpshufb a0, c2, c2; \
	vpshufb a0, c3, c3; \
	vpshufb a0, d0, d0; \
	vpshufb a0, d1, d1; \
	vpshufb a0, d2, d2; \
	vpshufb a0, d3, d3; \
	vmovdqu d3, st1; \
	vmovdqu st0, d3; \
	vpshufb a0, d3, a0; \
	vmovdqu d2, st0; \
	\
	transpose_4x4(a0, b0, c0, d0, d2, d3); \
	transpose_4x4(a1, b1, c1, d1, d2, d3); \
	vmovdqu st0, d2; \
	vmovdqu st1, d3; \
	\
	vmovdqu b0, st0; \
	vmovdqu b1, st1; \
	transpose_4x4(a2, b2, c2, d2, b0, b1); \
	transpose_4x4(a3, b3, c3, d3, b0, b1); \
	vmovdqu st0, b0; \
	vmovdqu st1, b1; \
	/* does not adjust output bytes inside vectors */

/* load blocks to registers and apply pre-whitening */
#define inpack32_pre(x0, x1, x2, x3, x4, x5, x6, x7, y0, y1, y2, y3, y4, y5, \
		     y6, y7, rio, key) \
	vpbroadcastq key, x0; \
	vpshufb .Lpack_bswap rRIP, x0, x0; \
	\
	vpxor 0 * 32(rio), x0, y7; \
	vpxor 1 * 32(rio), x0, y6; \
	vpxor 2 * 32(rio), x0, y5; \
	vpxor 3 * 32(rio), x0, y4; \
	vpxor 4 * 32(rio), x0, y3; \
	vpxor 5 * 32(rio), x0, y2; \
	vpxor 6 * 32(rio), x0, y1; \
	vpxor 7 * 32(rio), x0, y0; \
	vpxor 8 * 32(rio), x0, x7; \
	vpxor 9 * 32(rio), x0, x6; \
	vpxor 10 * 32(rio), x0, x5; \
	vpxor 11 * 32(rio), x0, x4; \
	vpxor 12 * 32(rio), x0, x3; \
	vpxor 13 * 32(rio), x0, x2; \
	vpxor 14 * 32(rio), x0, x1; \
	vpxor 15 * 32(rio), x0, x0;

/* byteslice pre-whitened blocks and store to temporary memory */
#define inpack32_post(x0, x1, x2, x3, x4, x5, x6, x7, y0, y1, y2, y3, y4, y5, \
		      y6, y7, mem_ab, mem_cd) \
	byteslice_16x16b_fast(x0, x1, x2, x3, x4, x5, x6, x7, y0, y1, y2, y3, \
			      y4, y5, y6, y7, (mem_ab), (mem_cd)); \
	\
	vmovdqu x0, 0 * 32(mem_ab); \
	vmovdqu x1, 1 * 32(mem_ab); \
	vmovdqu x2, 2 * 32(mem_ab); \
	vmovdqu x3, 3 * 32(mem_ab); \
	vmovdqu x4, 4 * 32(mem_ab); \
	vmovdqu x5, 5 * 32(mem_ab); \
	vmovdqu x6, 6 * 32(mem_ab); \
	vmovdqu x7, 7 * 32(mem_ab); \
	vmovdqu y0, 0 * 32(mem_cd); \
	vmovdqu y1, 1 * 32(mem_cd); \
	vmovdqu y2, 2 * 32(mem_cd); \
	vmovdqu y3, 3 * 32(mem_cd); \
	vmovdqu y4, 4 * 32(mem_cd); \
	vmovdqu y5, 5 * 32(mem_cd); \
	vmovdqu y6, 6 * 32(mem_cd); \
	vmovdqu y7, 7 * 32(mem_cd);

/* de-byteslice, apply post-whitening and store blocks */
#define outunpack32(x0, x1, x2, x3, x4, x5, x6, x7, y0, y1, y2, y3, y4, \
		    y5, y6, y7, key, stack_tmp0, stack_tmp1) \
	byteslice_16x16b_fast(y0, y4, x0, x4, y1, y5, x1, x5, y2, y6, x2, x6, \
			      y3, y7, x3, x7, stack_tmp0, stack_tmp1); \
	\
	vmovdqu x0, stack_tmp0; \
	\
	vpbroadcastq key, x0; \
	vpshufb .Lpack_bswap rRIP, x0, x0; \
	\
	vpxor x0, y7, y7; \
	vpxor x0, y6, y6; \
	vpxor x0, y5, y5; \
	vpxor x0, y4, y4; \
	vpxor x0, y3, y3; \
	vpxor x0, y2, y2; \
	vpxor x0, y1, y1; \
	vpxor x0, y0, y0; \
	vpxor x0, x7, x7; \
	vpxor x0, x6, x6; \
	vpxor x0, x5, x5; \
	vpxor x0, x4, x4; \
	vpxor x0, x3, x3; \
	vpxor x0, x2, x2; \
	vpxor x0, x1, x1; \
	vpxor stack_tmp0, x0, x0;

#define write_output(x0, x1, x2, x3, x4, x5, x6, x7, y0, y1, y2, y3, y4, y5, \
		     y6, y7, rio) \
	vmovdqu x0, 0 * 32(rio); \
	vmovdqu x1, 1 * 32(rio); \
	vmovdqu x2, 2 * 32(rio); \
	vmovdqu x3, 3 * 32(rio); \
	vmovdqu x4, 4 * 32(rio); \
	vmovdqu x5, 5 * 32(rio); \
	vmovdqu x6, 6 * 32(rio); \
	vmovdqu x7, 7 * 32(rio); \
	vmovdqu y0, 8 * 32(rio); \
	vmovdqu y1, 9 * 32(rio); \
	vmovdqu y2, 10 * 32(rio); \
	vmovdqu y3, 11 * 32(rio); \
	vmovdqu y4, 12 * 32(rio); \
	vmovdqu y5, 13 * 32(rio); \
	vmovdqu y6, 14 * 32(rio); \
	vmovdqu y7, 15 * 32(rio);

SECTION_RODATA

.align 32

#define SHUFB_BYTES(idx) \
	0 + (idx), 4 + (idx), 8 + (idx), 12 + (idx)

FUNC_NAME(_constants):
ELF(.type   FUNC_NAME(_constants),@object;)

.Lpack_bswap:
	.long 0x00010203, 0x04050607, 0x80808080, 0x80808080
	.long 0x00010203, 0x04050607, 0x80808080, 0x80808080

.Lshufb_16x16b:
	.byte SHUFB_BYTES(0), SHUFB_BYTES(1), SHUFB_BYTES(2), SHUFB_BYTES(3)

/* For CTR-mode IV byteswap */
.Lbswap128_mask:
	.byte 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0

/* CTR byte addition constants */
.align 32
.Lbige_addb_0_1:
	.byte 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	.byte 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1
.Lbige_addb_2_3:
	.byte 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2
	.byte 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3
.Lbige_addb_4_5:
	.byte 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4
	.byte 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5
.Lbige_addb_6_7:
	.byte 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6
	.byte 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7
.Lbige_addb_8_9:
	.byte 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8
	.byte 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 9
.Lbige_addb_10_11:
	.byte 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 10
	.byte 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 11
.Lbige_addb_12_13:
	.byte 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 12
	.byte 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 13
.Lbige_addb_14_15:
	.byte 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 14
	.byte 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 15
.Lbige_addb_16_16:
	.byte 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 16
	.byte 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 16

#ifdef CAMELLIA_GFNI_BUILD

.align 64
/* Pre-filters and post-filters bit-matrixes for Camellia sboxes s1, s2, s3
 * and s4.
 *   See http://urn.fi/URN:NBN:fi:oulu-201305311409, pages 43-48.
 *
 * Pre-filters are directly from above source, "θ₁"/"θ₄". Post-filters are
 * combination of function "A" (AES SubBytes affine transformation) and
 * "ψ₁"/"ψ₂"/"ψ₃".
 */

/* Bit-matrix from "θ₁(x)" function: */
.Lpre_filter_bitmatrix_s123:
	.quad BM8X8(BV8(1, 1, 1, 0, 1, 1, 0, 1),
		    BV8(0, 0, 1, 1, 0, 0, 1, 0),
		    BV8(1, 1, 0, 1, 0, 0, 0, 0),
		    BV8(1, 0, 1, 1, 0, 0, 1, 1),
		    BV8(0, 0, 0, 0, 1, 1, 0, 0),
		    BV8(1, 0, 1, 0, 0, 1, 0, 0),
		    BV8(0, 0, 1, 0, 1, 1, 0, 0),
		    BV8(1, 0, 0, 0, 0, 1, 1, 0))

/* Bit-matrix from "θ₄(x)" function: */
.Lpre_filter_bitmatrix_s4:
	.quad BM8X8(BV8(1, 1, 0, 1, 1, 0, 1, 1),
		    BV8(0, 1, 1, 0, 0, 1, 0, 0),
		    BV8(1, 0, 1, 0, 0, 0, 0, 1),
		    BV8(0, 1, 1, 0, 0, 1, 1, 1),
		    BV8(0, 0, 0, 1, 1, 0, 0, 0),
		    BV8(0, 1, 0, 0, 1, 0, 0, 1),
		    BV8(0, 1, 0, 1, 1, 0, 0, 0),
		    BV8(0, 0, 0, 0, 1, 1, 0, 1))

/* Bit-matrix from "ψ₁(A(x))" function: */
.Lpost_filter_bitmatrix_s14:
	.quad BM8X8(BV8(0, 0, 0, 0, 0, 0, 0, 1),
		    BV8(0, 1, 1, 0, 0, 1, 1, 0),
		    BV8(1, 0, 1, 1, 1, 1, 1, 0),
		    BV8(0, 0, 0, 1, 1, 0, 1, 1),
		    BV8(1, 0, 0, 0, 1, 1, 1, 0),
		    BV8(0, 1, 0, 1, 1, 1, 1, 0),
		    BV8(0, 1, 1, 1, 1, 1, 1, 1),
		    BV8(0, 0, 0, 1, 1, 1, 0, 0))

/* Bit-matrix from "ψ₂(A(x))" function: */
.Lpost_filter_bitmatrix_s2:
	.quad BM8X8(BV8(0, 0, 0, 1, 1, 1, 0, 0),
		    BV8(0, 0, 0, 0, 0, 0, 0, 1),
		    BV8(0, 1, 1, 0, 0, 1, 1, 0),
		    BV8(1, 0, 1, 1, 1, 1, 1, 0),
		    BV8(0, 0, 0, 1, 1, 0, 1, 1),
		    BV8(1, 0, 0, 0, 1, 1, 1, 0),
		    BV8(0, 1, 0, 1, 1, 1, 1, 0),
		    BV8(0, 1, 1, 1, 1, 1, 1, 1))

/* Bit-matrix from "ψ₃(A(x))" function: */
.Lpost_filter_bitmatrix_s3:
	.quad BM8X8(BV8(0, 1, 1, 0, 0, 1, 1, 0),
		    BV8(1, 0, 1, 1, 1, 1, 1, 0),
		    BV8(0, 0, 0, 1, 1, 0, 1, 1),
		    BV8(1, 0, 0, 0, 1, 1, 1, 0),
		    BV8(0, 1, 0, 1, 1, 1, 1, 0),
		    BV8(0, 1, 1, 1, 1, 1, 1, 1),
		    BV8(0, 0, 0, 1, 1, 1, 0, 0),
		    BV8(0, 0, 0, 0, 0, 0, 0, 1))

/* Bit-matrix for right shifting uint8_t values in vector by 7. */
.Lright_shift_by_7:
	.quad BM8X8(BV8(0, 0, 0, 0, 0, 0, 0, 1),
		    BV8(0, 0, 0, 0, 0, 0, 0, 0),
		    BV8(0, 0, 0, 0, 0, 0, 0, 0),
		    BV8(0, 0, 0, 0, 0, 0, 0, 0),
		    BV8(0, 0, 0, 0, 0, 0, 0, 0),
		    BV8(0, 0, 0, 0, 0, 0, 0, 0),
		    BV8(0, 0, 0, 0, 0, 0, 0, 0),
		    BV8(0, 0, 0, 0, 0, 0, 0, 0))

#else /* CAMELLIA_GFNI_BUILD */

/*
 * pre-SubByte transform
 *
 * pre-lookup for sbox1, sbox2, sbox3:
 *   swap_bitendianness(
 *       isom_map_camellia_to_aes(
 *           camellia_f(
 *               swap_bitendianess(in)
 *           )
 *       )
 *   )
 *
 * (note: '⊕ 0xc5' inside camellia_f())
 */
.Lpre_tf_lo_s1:
	.byte 0x45, 0xe8, 0x40, 0xed, 0x2e, 0x83, 0x2b, 0x86
	.byte 0x4b, 0xe6, 0x4e, 0xe3, 0x20, 0x8d, 0x25, 0x88
.Lpre_tf_hi_s1:
	.byte 0x00, 0x51, 0xf1, 0xa0, 0x8a, 0xdb, 0x7b, 0x2a
	.byte 0x09, 0x58, 0xf8, 0xa9, 0x83, 0xd2, 0x72, 0x23

/*
 * pre-SubByte transform
 *
 * pre-lookup for sbox4:
 *   swap_bitendianness(
 *       isom_map_camellia_to_aes(
 *           camellia_f(
 *               swap_bitendianess(in <<< 1)
 *           )
 *       )
 *   )
 *
 * (note: '⊕ 0xc5' inside camellia_f())
 */
.Lpre_tf_lo_s4:
	.byte 0x45, 0x40, 0x2e, 0x2b, 0x4b, 0x4e, 0x20, 0x25
	.byte 0x14, 0x11, 0x7f, 0x7a, 0x1a, 0x1f, 0x71, 0x74
.Lpre_tf_hi_s4:
	.byte 0x00, 0xf1, 0x8a, 0x7b, 0x09, 0xf8, 0x83, 0x72
	.byte 0xad, 0x5c, 0x27, 0xd6, 0xa4, 0x55, 0x2e, 0xdf

/*
 * post-SubByte transform
 *
 * post-lookup for sbox1, sbox4:
 *  swap_bitendianness(
 *      camellia_h(
 *          isom_map_aes_to_camellia(
 *              swap_bitendianness(
 *                  aes_inverse_affine_transform(in)
 *              )
 *          )
 *      )
 *  )
 *
 * (note: '⊕ 0x6e' inside camellia_h())
 */
.Lpost_tf_lo_s1:
	.byte 0x3c, 0xcc, 0xcf, 0x3f, 0x32, 0xc2, 0xc1, 0x31
	.byte 0xdc, 0x2c, 0x2f, 0xdf, 0xd2, 0x22, 0x21, 0xd1
.Lpost_tf_hi_s1:
	.byte 0x00, 0xf9, 0x86, 0x7f, 0xd7, 0x2e, 0x51, 0xa8
	.byte 0xa4, 0x5d, 0x22, 0xdb, 0x73, 0x8a, 0xf5, 0x0c

/*
 * post-SubByte transform
 *
 * post-lookup for sbox2:
 *  swap_bitendianness(
 *      camellia_h(
 *          isom_map_aes_to_camellia(
 *              swap_bitendianness(
 *                  aes_inverse_affine_transform(in)
 *              )
 *          )
 *      )
 *  ) <<< 1
 *
 * (note: '⊕ 0x6e' inside camellia_h())
 */
.Lpost_tf_lo_s2:
	.byte 0x78, 0x99, 0x9f, 0x7e, 0x64, 0x85, 0x83, 0x62
	.byte 0xb9, 0x58, 0x5e, 0xbf, 0xa5, 0x44, 0x42, 0xa3
.Lpost_tf_hi_s2:
	.byte 0x00, 0xf3, 0x0d, 0xfe, 0xaf, 0x5c, 0xa2, 0x51
	.byte 0x49, 0xba, 0x44, 0xb7, 0xe6, 0x15, 0xeb, 0x18

/*
 * post-SubByte transform
 *
 * post-lookup for sbox3:
 *  swap_bitendianness(
 *      camellia_h(
 *          isom_map_aes_to_camellia(
 *              swap_bitendianness(
 *                  aes_inverse_affine_transform(in)
 *              )
 *          )
 *      )
 *  ) >>> 1
 *
 * (note: '⊕ 0x6e' inside camellia_h())
 */
.Lpost_tf_lo_s3:
	.byte 0x1e, 0x66, 0xe7, 0x9f, 0x19, 0x61, 0xe0, 0x98
	.byte 0x6e, 0x16, 0x97, 0xef, 0x69, 0x11, 0x90, 0xe8
.Lpost_tf_hi_s3:
	.byte 0x00, 0xfc, 0x43, 0xbf, 0xeb, 0x17, 0xa8, 0x54
	.byte 0x52, 0xae, 0x11, 0xed, 0xb9, 0x45, 0xfa, 0x06

/* For isolating SubBytes from AESENCLAST, inverse shift row */
.Linv_shift_row:
	.byte 0x00, 0x0d, 0x0a, 0x07, 0x04, 0x01, 0x0e, 0x0b
	.byte 0x08, 0x05, 0x02, 0x0f, 0x0c, 0x09, 0x06, 0x03

.align 4
/* 4-bit mask */
.L0f0f0f0f:
	.long 0x0f0f0f0f

#endif /* CAMELLIA_GFNI_BUILD */

ELF(.size FUNC_NAME(_constants),.-FUNC_NAME(_constants);)

.text

.align 16
ELF(.type   FUNC_NAME(enc_blk32),@function;)

FUNC_NAME(enc_blk32):
	/* input:
	 *	%rdi: ctx, CTX
	 *	%rax: temporary storage, 512 bytes
	 *	%r8d: 24 for 16 byte key, 32 for larger
	 *	%ymm0..%ymm15: 32 plaintext blocks
	 * output:
	 *	%ymm0..%ymm15: 32 encrypted blocks, order swapped:
	 *       7, 8, 6, 5, 4, 3, 2, 1, 0, 15, 14, 13, 12, 11, 10, 9, 8
	 */
	CFI_STARTPROC();

	leaq 8 * 32(%rax), %rcx;

	leaq (-8 * 8)(CTX, %r8, 8), %r8;

	inpack32_post(%ymm0, %ymm1, %ymm2, %ymm3, %ymm4, %ymm5, %ymm6, %ymm7,
		      %ymm8, %ymm9, %ymm10, %ymm11, %ymm12, %ymm13, %ymm14,
		      %ymm15, %rax, %rcx);

.align 8
.Lenc_loop:
	enc_rounds32(%ymm0, %ymm1, %ymm2, %ymm3, %ymm4, %ymm5, %ymm6, %ymm7,
		     %ymm8, %ymm9, %ymm10, %ymm11, %ymm12, %ymm13, %ymm14,
		     %ymm15, %rax, %rcx, 0);

	cmpq %r8, CTX;
	je .Lenc_done;
	leaq (8 * 8)(CTX), CTX;

	fls32(%rax, %ymm0, %ymm1, %ymm2, %ymm3, %ymm4, %ymm5, %ymm6, %ymm7,
	      %rcx, %ymm8, %ymm9, %ymm10, %ymm11, %ymm12, %ymm13, %ymm14,
	      %ymm15,
	      ((key_table) + 0)(CTX),
	      ((key_table) + 4)(CTX),
	      ((key_table) + 8)(CTX),
	      ((key_table) + 12)(CTX));
	jmp .Lenc_loop;

.align 8
.Lenc_done:
	/* load CD for output */
	vmovdqu 0 * 32(%rcx), %ymm8;
	vmovdqu 1 * 32(%rcx), %ymm9;
	vmovdqu 2 * 32(%rcx), %ymm10;
	vmovdqu 3 * 32(%rcx), %ymm11;
	vmovdqu 4 * 32(%rcx), %ymm12;
	vmovdqu 5 * 32(%rcx), %ymm13;
	vmovdqu 6 * 32(%rcx), %ymm14;
	vmovdqu 7 * 32(%rcx), %ymm15;

	outunpack32(%ymm0, %ymm1, %ymm2, %ymm3, %ymm4, %ymm5, %ymm6, %ymm7,
		    %ymm8, %ymm9, %ymm10, %ymm11, %ymm12, %ymm13, %ymm14,
		    %ymm15, ((key_table) + 8 * 8)(%r8), (%rax), 1 * 32(%rax));

	ret_spec_stop;
	CFI_ENDPROC();
ELF(.size FUNC_NAME(enc_blk32),.-FUNC_NAME(enc_blk32);)

.align 16
ELF(.type   FUNC_NAME(dec_blk32),@function;)

FUNC_NAME(dec_blk32):
	/* input:
	 *	%rdi: ctx, CTX
	 *	%rax: temporary storage, 512 bytes
	 *	%r8d: 24 for 16 byte key, 32 for larger
	 *	%ymm0..%ymm15: 32 encrypted blocks
	 * output:
	 *	%ymm0..%ymm15: 32 plaintext blocks, order swapped:
	 *       7, 8, 6, 5, 4, 3, 2, 1, 0, 15, 14, 13, 12, 11, 10, 9, 8
	 */
	CFI_STARTPROC();

	movq %r8, %rcx;
	movq CTX, %r8
	leaq (-8 * 8)(CTX, %rcx, 8), CTX;

	leaq 8 * 32(%rax), %rcx;

	inpack32_post(%ymm0, %ymm1, %ymm2, %ymm3, %ymm4, %ymm5, %ymm6, %ymm7,
		      %ymm8, %ymm9, %ymm10, %ymm11, %ymm12, %ymm13, %ymm14,
		      %ymm15, %rax, %rcx);

.align 8
.Ldec_loop:
	dec_rounds32(%ymm0, %ymm1, %ymm2, %ymm3, %ymm4, %ymm5, %ymm6, %ymm7,
		     %ymm8, %ymm9, %ymm10, %ymm11, %ymm12, %ymm13, %ymm14,
		     %ymm15, %rax, %rcx, 0);

	cmpq %r8, CTX;
	je .Ldec_done;

	fls32(%rax, %ymm0, %ymm1, %ymm2, %ymm3, %ymm4, %ymm5, %ymm6, %ymm7,
	      %rcx, %ymm8, %ymm9, %ymm10, %ymm11, %ymm12, %ymm13, %ymm14,
	      %ymm15,
	      ((key_table) + 8)(CTX),
	      ((key_table) + 12)(CTX),
	      ((key_table) + 0)(CTX),
	      ((key_table) + 4)(CTX));

	leaq (-8 * 8)(CTX), CTX;
	jmp .Ldec_loop;

.align 8
.Ldec_done:
	/* load CD for output */
	vmovdqu 0 * 32(%rcx), %ymm8;
	vmovdqu 1 * 32(%rcx), %ymm9;
	vmovdqu 2 * 32(%rcx), %ymm10;
	vmovdqu 3 * 32(%rcx), %ymm11;
	vmovdqu 4 * 32(%rcx), %ymm12;
	vmovdqu 5 * 32(%rcx), %ymm13;
	vmovdqu 6 * 32(%rcx), %ymm14;
	vmovdqu 7 * 32(%rcx), %ymm15;

	outunpack32(%ymm0, %ymm1, %ymm2, %ymm3, %ymm4, %ymm5, %ymm6, %ymm7,
		    %ymm8, %ymm9, %ymm10, %ymm11, %ymm12, %ymm13, %ymm14,
		    %ymm15, (key_table)(CTX), (%rax), 1 * 32(%rax));

	ret_spec_stop;
	CFI_ENDPROC();
ELF(.size FUNC_NAME(dec_blk32),.-FUNC_NAME(dec_blk32);)

#define inc_le128(x, minus_one, tmp) \
	vpcmpeqq minus_one, x, tmp; \
	vpsubq minus_one, x, x; \
	vpslldq $8, tmp, tmp; \
	vpsubq tmp, x, x;

.align 16
.globl FUNC_NAME(ctr_enc)
ELF(.type   FUNC_NAME(ctr_enc),@function;)

FUNC_NAME(ctr_enc):
	/* input:
	 *	%rdi: ctx, CTX
	 *	%rsi: dst (32 blocks)
	 *	%rdx: src (32 blocks)
	 *	%rcx: iv (big endian, 128bit)
	 */
	CFI_STARTPROC();

	pushq %rbp;
	CFI_PUSH(%rbp);
	movq %rsp, %rbp;
	CFI_DEF_CFA_REGISTER(%rbp);

	cmpl $128, key_bitlength(CTX);
	movl $32, %r8d;
	movl $24, %eax;
	cmovel %eax, %r8d; /* max */

	subq $(16 * 32), %rsp;
	andq $~63, %rsp;
	movq %rsp, %rax;

	cmpb $(0x100 - 32), 15(%rcx);
	jbe .Lctr_byteadd;

	movq 8(%rcx), %r11;
	bswapq %r11;

	vpcmpeqd %ymm15, %ymm15, %ymm15;
	vpsrldq $8, %ymm15, %ymm15; /* ab: -1:0 ; cd: -1:0 */

	/* load IV and byteswap */
	vmovdqu (%rcx), %xmm0;
	vpshufb .Lbswap128_mask rRIP, %xmm0, %xmm0;
	vmovdqa %xmm0, %xmm1;
	inc_le128(%xmm0, %xmm15, %xmm14);
	vbroadcasti128 .Lbswap128_mask rRIP, %ymm14;
	vinserti128 $1, %xmm0, %ymm1, %ymm0;
	vpshufb %ymm14, %ymm0, %ymm13;
	vmovdqu %ymm13, 15 * 32(%rax);

	/* check need for handling 64-bit overflow and carry */
	cmpq $(0xffffffffffffffff - 32), %r11;
	ja .Lload_ctr_carry;

	/* construct IVs */
	vpaddq %ymm15, %ymm15, %ymm15; /* ab: -2:0 ; cd: -2:0 */
	vpsubq %ymm15, %ymm0, %ymm0;
	vpshufb %ymm14, %ymm0, %ymm13;
	vmovdqu %ymm13, 14 * 32(%rax);
	vpsubq %ymm15, %ymm0, %ymm0;
	vpshufb %ymm14, %ymm0, %ymm13;
	vmovdqu %ymm13, 13 * 32(%rax);
	vpsubq %ymm15, %ymm0, %ymm0;
	vpshufb %ymm14, %ymm0, %ymm12;
	vpsubq %ymm15, %ymm0, %ymm0;
	vpshufb %ymm14, %ymm0, %ymm11;
	vpsubq %ymm15, %ymm0, %ymm0;
	vpshufb %ymm14, %ymm0, %ymm10;
	vpsubq %ymm15, %ymm0, %ymm0;
	vpshufb %ymm14, %ymm0, %ymm9;
	vpsubq %ymm15, %ymm0, %ymm0;
	vpshufb %ymm14, %ymm0, %ymm8;
	vpsubq %ymm15, %ymm0, %ymm0;
	vpshufb %ymm14, %ymm0, %ymm7;
	vpsubq %ymm15, %ymm0, %ymm0;
	vpshufb %ymm14, %ymm0, %ymm6;
	vpsubq %ymm15, %ymm0, %ymm0;
	vpshufb %ymm14, %ymm0, %ymm5;
	vpsubq %ymm15, %ymm0, %ymm0;
	vpshufb %ymm14, %ymm0, %ymm4;
	vpsubq %ymm15, %ymm0, %ymm0;
	vpshufb %ymm14, %ymm0, %ymm3;
	vpsubq %ymm15, %ymm0, %ymm0;
	vpshufb %ymm14, %ymm0, %ymm2;
	vpsubq %ymm15, %ymm0, %ymm0;
	vpshufb %ymm14, %ymm0, %ymm1;
	vpsubq %ymm15, %ymm0, %ymm0;  /* +30 ; +31 */
	vpsubq %xmm15, %xmm0, %xmm13; /* +32 */
	vpshufb %ymm14, %ymm0, %ymm0;
	vpshufb %xmm14, %xmm13, %xmm13;
	vmovdqu %xmm13, (%rcx);

	jmp .Lload_ctr_done;

.align 4
.Lload_ctr_carry:
	/* construct IVs */
	inc_le128(%ymm0, %ymm15, %ymm13); /* ab: le1 ; cd: le2 */
	inc_le128(%ymm0, %ymm15, %ymm13); /* ab: le2 ; cd: le3 */
	vpshufb %ymm14, %ymm0, %ymm13;
	vmovdqu %ymm13, 14 * 32(%rax);
	inc_le128(%ymm0, %ymm15, %ymm13);
	inc_le128(%ymm0, %ymm15, %ymm13);
	vpshufb %ymm14, %ymm0, %ymm13;
	vmovdqu %ymm13, 13 * 32(%rax);
	inc_le128(%ymm0, %ymm15, %ymm13);
	inc_le128(%ymm0, %ymm15, %ymm13);
	vpshufb %ymm14, %ymm0, %ymm12;
	inc_le128(%ymm0, %ymm15, %ymm13);
	inc_le128(%ymm0, %ymm15, %ymm13);
	vpshufb %ymm14, %ymm0, %ymm11;
	inc_le128(%ymm0, %ymm15, %ymm13);
	inc_le128(%ymm0, %ymm15, %ymm13);
	vpshufb %ymm14, %ymm0, %ymm10;
	inc_le128(%ymm0, %ymm15, %ymm13);
	inc_le128(%ymm0, %ymm15, %ymm13);
	vpshufb %ymm14, %ymm0, %ymm9;
	inc_le128(%ymm0, %ymm15, %ymm13);
	inc_le128(%ymm0, %ymm15, %ymm13);
	vpshufb %ymm14, %ymm0, %ymm8;
	inc_le128(%ymm0, %ymm15, %ymm13);
	inc_le128(%ymm0, %ymm15, %ymm13);
	vpshufb %ymm14, %ymm0, %ymm7;
	inc_le128(%ymm0, %ymm15, %ymm13);
	inc_le128(%ymm0, %ymm15, %ymm13);
	vpshufb %ymm14, %ymm0, %ymm6;
	inc_le128(%ymm0, %ymm15, %ymm13);
	inc_le128(%ymm0, %ymm15, %ymm13);
	vpshufb %ymm14, %ymm0, %ymm5;
	inc_le128(%ymm0, %ymm15, %ymm13);
	inc_le128(%ymm0, %ymm15, %ymm13);
	vpshufb %ymm14, %ymm0, %ymm4;
	inc_le128(%ymm0, %ymm15, %ymm13);
	inc_le128(%ymm0, %ymm15, %ymm13);
	vpshufb %ymm14, %ymm0, %ymm3;
	inc_le128(%ymm0, %ymm15, %ymm13);
	inc_le128(%ymm0, %ymm15, %ymm13);
	vpshufb %ymm14, %ymm0, %ymm2;
	inc_le128(%ymm0, %ymm15, %ymm13);
	inc_le128(%ymm0, %ymm15, %ymm13);
	vpshufb %ymm14, %ymm0, %ymm1;
	inc_le128(%ymm0, %ymm15, %ymm13);
	inc_le128(%ymm0, %ymm15, %ymm13);
	vextracti128 $1, %ymm0, %xmm13;
	vpshufb %ymm14, %ymm0, %ymm0;
	inc_le128(%xmm13, %xmm15, %xmm14);
	vpshufb .Lbswap128_mask rRIP, %xmm13, %xmm13;
	vmovdqu %xmm13, (%rcx);

.align 8
.Lload_ctr_done:
	/* inpack32_pre: */
	vpbroadcastq (key_table)(CTX), %ymm15;
	vpshufb .Lpack_bswap rRIP, %ymm15, %ymm15;
	vpxor %ymm0, %ymm15, %ymm0;
	vpxor %ymm1, %ymm15, %ymm1;
	vpxor %ymm2, %ymm15, %ymm2;
	vpxor %ymm3, %ymm15, %ymm3;
	vpxor %ymm4, %ymm15, %ymm4;
	vpxor %ymm5, %ymm15, %ymm5;
	vpxor %ymm6, %ymm15, %ymm6;
	vpxor %ymm7, %ymm15, %ymm7;
	vpxor %ymm8, %ymm15, %ymm8;
	vpxor %ymm9, %ymm15, %ymm9;
	vpxor %ymm10, %ymm15, %ymm10;
	vpxor %ymm11, %ymm15, %ymm11;
	vpxor %ymm12, %ymm15, %ymm12;
	vpxor 13 * 32(%rax), %ymm15, %ymm13;
	vpxor 14 * 32(%rax), %ymm15, %ymm14;
	vpxor 15 * 32(%rax), %ymm15, %ymm15;

	call FUNC_NAME(enc_blk32);

	vpxor 0 * 32(%rdx), %ymm7, %ymm7;
	vpxor 1 * 32(%rdx), %ymm6, %ymm6;
	vpxor 2 * 32(%rdx), %ymm5, %ymm5;
	vpxor 3 * 32(%rdx), %ymm4, %ymm4;
	vpxor 4 * 32(%rdx), %ymm3, %ymm3;
	vpxor 5 * 32(%rdx), %ymm2, %ymm2;
	vpxor 6 * 32(%rdx), %ymm1, %ymm1;
	vpxor 7 * 32(%rdx), %ymm0, %ymm0;
	vpxor 8 * 32(%rdx), %ymm15, %ymm15;
	vpxor 9 * 32(%rdx), %ymm14, %ymm14;
	vpxor 10 * 32(%rdx), %ymm13, %ymm13;
	vpxor 11 * 32(%rdx), %ymm12, %ymm12;
	vpxor 12 * 32(%rdx), %ymm11, %ymm11;
	vpxor 13 * 32(%rdx), %ymm10, %ymm10;
	vpxor 14 * 32(%rdx), %ymm9, %ymm9;
	vpxor 15 * 32(%rdx), %ymm8, %ymm8;

	write_output(%ymm7, %ymm6, %ymm5, %ymm4, %ymm3, %ymm2, %ymm1, %ymm0,
		     %ymm15, %ymm14, %ymm13, %ymm12, %ymm11, %ymm10, %ymm9,
		     %ymm8, %rsi);

	vzeroall;

	leave;
	CFI_LEAVE();
	ret_spec_stop;

.align 8
.Lctr_byteadd_full_ctr_carry:
	movq 8(%rcx), %r11;
	movq (%rcx), %r10;
	bswapq %r11;
	bswapq %r10;
	addq $32, %r11;
	adcq $0, %r10;
	bswapq %r11;
	bswapq %r10;
	movq %r11, 8(%rcx);
	movq %r10, (%rcx);
	jmp .Lctr_byteadd_ymm;
.align 8
.Lctr_byteadd:
	vbroadcasti128 (%rcx), %ymm8;
	je .Lctr_byteadd_full_ctr_carry;
	addb $32, 15(%rcx);
.Lctr_byteadd_ymm:
	vpaddb .Lbige_addb_16_16 rRIP, %ymm8, %ymm0;
	vpaddb .Lbige_addb_0_1 rRIP, %ymm8, %ymm15;
	vpaddb .Lbige_addb_2_3 rRIP, %ymm8, %ymm14;
	vmovdqu %ymm15, 15 * 32(%rax);
	vpaddb .Lbige_addb_4_5 rRIP, %ymm8, %ymm13;
	vmovdqu %ymm14, 14 * 32(%rax);
	vpaddb .Lbige_addb_6_7 rRIP, %ymm8, %ymm12;
	vmovdqu %ymm13, 13 * 32(%rax);
	vpaddb .Lbige_addb_8_9 rRIP, %ymm8, %ymm11;
	vpaddb .Lbige_addb_10_11 rRIP, %ymm8, %ymm10;
	vpaddb .Lbige_addb_12_13 rRIP, %ymm8, %ymm9;
	vpaddb .Lbige_addb_14_15 rRIP, %ymm8, %ymm8;
	vpaddb .Lbige_addb_0_1 rRIP, %ymm0, %ymm7;
	vpaddb .Lbige_addb_2_3 rRIP, %ymm0, %ymm6;
	vpaddb .Lbige_addb_4_5 rRIP, %ymm0, %ymm5;
	vpaddb .Lbige_addb_6_7 rRIP, %ymm0, %ymm4;
	vpaddb .Lbige_addb_8_9 rRIP, %ymm0, %ymm3;
	vpaddb .Lbige_addb_10_11 rRIP, %ymm0, %ymm2;
	vpaddb .Lbige_addb_12_13 rRIP, %ymm0, %ymm1;
	vpaddb .Lbige_addb_14_15 rRIP, %ymm0, %ymm0;

	jmp .Lload_ctr_done;
	CFI_ENDPROC();
ELF(.size FUNC_NAME(ctr_enc),.-FUNC_NAME(ctr_enc);)

.align 16
.globl FUNC_NAME(cbc_dec)
ELF(.type   FUNC_NAME(cbc_dec),@function;)

FUNC_NAME(cbc_dec):
	/* input:
	 *	%rdi: ctx, CTX
	 *	%rsi: dst (32 blocks)
	 *	%rdx: src (32 blocks)
	 *	%rcx: iv
	 */
	CFI_STARTPROC();

	pushq %rbp;
	CFI_PUSH(%rbp);
	movq %rsp, %rbp;
	CFI_DEF_CFA_REGISTER(%rbp);

	movq %rcx, %r9;

	cmpl $128, key_bitlength(CTX);
	movl $32, %r8d;
	movl $24, %eax;
	cmovel %eax, %r8d; /* max */

	subq $(16 * 32), %rsp;
	andq $~63, %rsp;
	movq %rsp, %rax;

	inpack32_pre(%ymm0, %ymm1, %ymm2, %ymm3, %ymm4, %ymm5, %ymm6, %ymm7,
		     %ymm8, %ymm9, %ymm10, %ymm11, %ymm12, %ymm13, %ymm14,
		     %ymm15, %rdx, (key_table)(CTX, %r8, 8));

	call FUNC_NAME(dec_blk32);

	/* XOR output with IV */
	vmovdqu %ymm8, (%rax);
	vmovdqu (%r9), %xmm8;
	vinserti128 $1, (%rdx), %ymm8, %ymm8;
	vpxor %ymm8, %ymm7, %ymm7;
	vmovdqu (%rax), %ymm8;
	vpxor (0 * 32 + 16)(%rdx), %ymm6, %ymm6;
	vpxor (1 * 32 + 16)(%rdx), %ymm5, %ymm5;
	vpxor (2 * 32 + 16)(%rdx), %ymm4, %ymm4;
	vpxor (3 * 32 + 16)(%rdx), %ymm3, %ymm3;
	vpxor (4 * 32 + 16)(%rdx), %ymm2, %ymm2;
	vpxor (5 * 32 + 16)(%rdx), %ymm1, %ymm1;
	vpxor (6 * 32 + 16)(%rdx), %ymm0, %ymm0;
	vpxor (7 * 32 + 16)(%rdx), %ymm15, %ymm15;
	vpxor (8 * 32 + 16)(%rdx), %ymm14, %ymm14;
	vpxor (9 * 32 + 16)(%rdx), %ymm13, %ymm13;
	vpxor (10 * 32 + 16)(%rdx), %ymm12, %ymm12;
	vpxor (11 * 32 + 16)(%rdx), %ymm11, %ymm11;
	vpxor (12 * 32 + 16)(%rdx), %ymm10, %ymm10;
	vpxor (13 * 32 + 16)(%rdx), %ymm9, %ymm9;
	vpxor (14 * 32 + 16)(%rdx), %ymm8, %ymm8;
	movq (15 * 32 + 16 + 0)(%rdx), %rax;
	movq (15 * 32 + 16 + 8)(%rdx), %rcx;

	write_output(%ymm7, %ymm6, %ymm5, %ymm4, %ymm3, %ymm2, %ymm1, %ymm0,
		     %ymm15, %ymm14, %ymm13, %ymm12, %ymm11, %ymm10, %ymm9,
		     %ymm8, %rsi);

	/* store new IV */
	movq %rax, (0)(%r9);
	movq %rcx, (8)(%r9);

	vzeroall;

	leave;
	CFI_LEAVE();
	ret_spec_stop;
	CFI_ENDPROC();
ELF(.size FUNC_NAME(cbc_dec),.-FUNC_NAME(cbc_dec);)

.align 16
.globl FUNC_NAME(cfb_dec)
ELF(.type   FUNC_NAME(cfb_dec),@function;)

FUNC_NAME(cfb_dec):
	/* input:
	 *	%rdi: ctx, CTX
	 *	%rsi: dst (32 blocks)
	 *	%rdx: src (32 blocks)
	 *	%rcx: iv
	 */
	CFI_STARTPROC();

	pushq %rbp;
	CFI_PUSH(%rbp);
	movq %rsp, %rbp;
	CFI_DEF_CFA_REGISTER(%rbp);

	cmpl $128, key_bitlength(CTX);
	movl $32, %r8d;
	movl $24, %eax;
	cmovel %eax, %r8d; /* max */

	subq $(16 * 32), %rsp;
	andq $~63, %rsp;
	movq %rsp, %rax;

	/* inpack32_pre: */
	vpbroadcastq (key_table)(CTX), %ymm0;
	vpshufb .Lpack_bswap rRIP, %ymm0, %ymm0;
	vmovdqu (%rcx), %xmm15;
	vinserti128 $1, (%rdx), %ymm15, %ymm15;
	vpxor %ymm15, %ymm0, %ymm15;
	vmovdqu (15 * 32 + 16)(%rdx), %xmm1;
	vmovdqu %xmm1, (%rcx); /* store new IV */
	vpxor (0 * 32 + 16)(%rdx), %ymm0, %ymm14;
	vpxor (1 * 32 + 16)(%rdx), %ymm0, %ymm13;
	vpxor (2 * 32 + 16)(%rdx), %ymm0, %ymm12;
	vpxor (3 * 32 + 16)(%rdx), %ymm0, %ymm11;
	vpxor (4 * 32 + 16)(%rdx), %ymm0, %ymm10;
	vpxor (5 * 32 + 16)(%rdx), %ymm0, %ymm9;
	vpxor (6 * 32 + 16)(%rdx), %ymm0, %ymm8;
	vpxor (7 * 32 + 16)(%rdx), %ymm0, %ymm7;
	vpxor (8 * 32 + 16)(%rdx), %ymm0, %ymm6;
	vpxor (9 * 32 + 16)(%rdx), %ymm0, %ymm5;
	vpxor (10 * 32 + 16)(%rdx), %ymm0, %ymm4;
	vpxor (11 * 32 + 16)(%rdx), %ymm0, %ymm3;
	vpxor (12 * 32 + 16)(%rdx), %ymm0, %ymm2;
	vpxor (13 * 32 + 16)(%rdx), %ymm0, %ymm1;
	vpxor (14 * 32 + 16)(%rdx), %ymm0, %ymm0;

	call FUNC_NAME(enc_blk32);

	vpxor 0 * 32(%rdx), %ymm7, %ymm7;
	vpxor 1 * 32(%rdx), %ymm6, %ymm6;
	vpxor 2 * 32(%rdx), %ymm5, %ymm5;
	vpxor 3 * 32(%rdx), %ymm4, %ymm4;
	vpxor 4 * 32(%rdx), %ymm3, %ymm3;
	vpxor 5 * 32(%rdx), %ymm2, %ymm2;
	vpxor 6 * 32(%rdx), %ymm1, %ymm1;
	vpxor 7 * 32(%rdx), %ymm0, %ymm0;
	vpxor 8 * 32(%rdx), %ymm15, %ymm15;
	vpxor 9 * 32(%rdx), %ymm14, %ymm14;
	vpxor 10 * 32(%rdx), %ymm13, %ymm13;
	vpxor 11 * 32(%rdx), %ymm12, %ymm12;
	vpxor 12 * 32(%rdx), %ymm11, %ymm11;
	vpxor 13 * 32(%rdx), %ymm10, %ymm10;
	vpxor 14 * 32(%rdx), %ymm9, %ymm9;
	vpxor 15 * 32(%rdx), %ymm8, %ymm8;

	write_output(%ymm7, %ymm6, %ymm5, %ymm4, %ymm3, %ymm2, %ymm1, %ymm0,
		     %ymm15, %ymm14, %ymm13, %ymm12, %ymm11, %ymm10, %ymm9,
		     %ymm8, %rsi);

	vzeroall;

	leave;
	CFI_LEAVE();
	ret_spec_stop;
	CFI_ENDPROC();
ELF(.size FUNC_NAME(cfb_dec),.-FUNC_NAME(cfb_dec);)

.align 16
.globl FUNC_NAME(ocb_enc)
ELF(.type   FUNC_NAME(ocb_enc),@function;)

FUNC_NAME(ocb_enc):
	/* input:
	 *	%rdi: ctx, CTX
	 *	%rsi: dst (32 blocks)
	 *	%rdx: src (32 blocks)
	 *	%rcx: offset
	 *	%r8 : checksum
	 *	%r9 : L pointers (void *L[32])
	 */
	CFI_STARTPROC();

	pushq %rbp;
	CFI_PUSH(%rbp);
	movq %rsp, %rbp;
	CFI_DEF_CFA_REGISTER(%rbp);

	subq $(16 * 32 + 4 * 8), %rsp;
	andq $~63, %rsp;
	movq %rsp, %rax;

	movq %r10, (16 * 32 + 0 * 8)(%rsp);
	movq %r11, (16 * 32 + 1 * 8)(%rsp);
	movq %r12, (16 * 32 + 2 * 8)(%rsp);
	movq %r13, (16 * 32 + 3 * 8)(%rsp);
	CFI_REG_ON_STACK(r10, 16 * 32 + 0 * 8);
	CFI_REG_ON_STACK(r11, 16 * 32 + 1 * 8);
	CFI_REG_ON_STACK(r12, 16 * 32 + 2 * 8);
	CFI_REG_ON_STACK(r13, 16 * 32 + 3 * 8);

	vmovdqu (%rcx), %xmm14;
	vmovdqu (%r8), %xmm13;

	/* Offset_i = Offset_{i-1} xor L_{ntz(i)} */
	/* Checksum_i = Checksum_{i-1} xor P_i  */
	/* C_i = Offset_i xor ENCIPHER(K, P_i xor Offset_i)  */

#define OCB_INPUT(n, l0reg, l1reg, yreg) \
	  vmovdqu (n * 32)(%rdx), yreg; \
	  vpxor (l0reg), %xmm14, %xmm15; \
	  vpxor (l1reg), %xmm15, %xmm14; \
	  vinserti128 $1, %xmm14, %ymm15, %ymm15; \
	  vpxor yreg, %ymm13, %ymm13; \
	  vpxor yreg, %ymm15, yreg; \
	  vmovdqu %ymm15, (n * 32)(%rsi);

	movq (0 * 8)(%r9), %r10;
	movq (1 * 8)(%r9), %r11;
	movq (2 * 8)(%r9), %r12;
	movq (3 * 8)(%r9), %r13;
	OCB_INPUT(0, %r10, %r11, %ymm0);
	vmovdqu %ymm0, (15 * 32)(%rax);
	OCB_INPUT(1, %r12, %r13, %ymm0);
	vmovdqu %ymm0, (14 * 32)(%rax);
	movq (4 * 8)(%r9), %r10;
	movq (5 * 8)(%r9), %r11;
	movq (6 * 8)(%r9), %r12;
	movq (7 * 8)(%r9), %r13;
	OCB_INPUT(2, %r10, %r11, %ymm0);
	vmovdqu %ymm0, (13 * 32)(%rax);
	OCB_INPUT(3, %r12, %r13, %ymm12);
	movq (8 * 8)(%r9), %r10;
	movq (9 * 8)(%r9), %r11;
	movq (10 * 8)(%r9), %r12;
	movq (11 * 8)(%r9), %r13;
	OCB_INPUT(4, %r10, %r11, %ymm11);
	OCB_INPUT(5, %r12, %r13, %ymm10);
	movq (12 * 8)(%r9), %r10;
	movq (13 * 8)(%r9), %r11;
	movq (14 * 8)(%r9), %r12;
	movq (15 * 8)(%r9), %r13;
	OCB_INPUT(6, %r10, %r11, %ymm9);
	OCB_INPUT(7, %r12, %r13, %ymm8);
	movq (16 * 8)(%r9), %r10;
	movq (17 * 8)(%r9), %r11;
	movq (18 * 8)(%r9), %r12;
	movq (19 * 8)(%r9), %r13;
	OCB_INPUT(8, %r10, %r11, %ymm7);
	OCB_INPUT(9, %r12, %r13, %ymm6);
	movq (20 * 8)(%r9), %r10;
	movq (21 * 8)(%r9), %r11;
	movq (22 * 8)(%r9), %r12;
	movq (23 * 8)(%r9), %r13;
	OCB_INPUT(10, %r10, %r11, %ymm5);
	OCB_INPUT(11, %r12, %r13, %ymm4);
	movq (24 * 8)(%r9), %r10;
	movq (25 * 8)(%r9), %r11;
	movq (26 * 8)(%r9), %r12;
	movq (27 * 8)(%r9), %r13;
	OCB_INPUT(12, %r10, %r11, %ymm3);
	OCB_INPUT(13, %r12, %r13, %ymm2);
	movq (28 * 8)(%r9), %r10;
	movq (29 * 8)(%r9), %r11;
	movq (30 * 8)(%r9), %r12;
	movq (31 * 8)(%r9), %r13;
	OCB_INPUT(14, %r10, %r11, %ymm1);
	OCB_INPUT(15, %r12, %r13, %ymm0);
#undef OCB_INPUT

	vextracti128 $1, %ymm13, %xmm15;
	vmovdqu %xmm14, (%rcx);
	vpxor %xmm13, %xmm15, %xmm15;
	vmovdqu %xmm15, (%r8);

	cmpl $128, key_bitlength(CTX);
	movl $32, %r8d;
	movl $24, %r10d;
	cmovel %r10d, %r8d; /* max */

	/* inpack32_pre: */
	vpbroadcastq (key_table)(CTX), %ymm15;
	vpshufb .Lpack_bswap rRIP, %ymm15, %ymm15;
	vpxor %ymm0, %ymm15, %ymm0;
	vpxor %ymm1, %ymm15, %ymm1;
	vpxor %ymm2, %ymm15, %ymm2;
	vpxor %ymm3, %ymm15, %ymm3;
	vpxor %ymm4, %ymm15, %ymm4;
	vpxor %ymm5, %ymm15, %ymm5;
	vpxor %ymm6, %ymm15, %ymm6;
	vpxor %ymm7, %ymm15, %ymm7;
	vpxor %ymm8, %ymm15, %ymm8;
	vpxor %ymm9, %ymm15, %ymm9;
	vpxor %ymm10, %ymm15, %ymm10;
	vpxor %ymm11, %ymm15, %ymm11;
	vpxor %ymm12, %ymm15, %ymm12;
	vpxor 13 * 32(%rax), %ymm15, %ymm13;
	vpxor 14 * 32(%rax), %ymm15, %ymm14;
	vpxor 15 * 32(%rax), %ymm15, %ymm15;

	call FUNC_NAME(enc_blk32);

	vpxor 0 * 32(%rsi), %ymm7, %ymm7;
	vpxor 1 * 32(%rsi), %ymm6, %ymm6;
	vpxor 2 * 32(%rsi), %ymm5, %ymm5;
	vpxor 3 * 32(%rsi), %ymm4, %ymm4;
	vpxor 4 * 32(%rsi), %ymm3, %ymm3;
	vpxor 5 * 32(%rsi), %ymm2, %ymm2;
	vpxor 6 * 32(%rsi), %ymm1, %ymm1;
	vpxor 7 * 32(%rsi), %ymm0, %ymm0;
	vpxor 8 * 32(%rsi), %ymm15, %ymm15;
	vpxor 9 * 32(%rsi), %ymm14, %ymm14;
	vpxor 10 * 32(%rsi), %ymm13, %ymm13;
	vpxor 11 * 32(%rsi), %ymm12, %ymm12;
	vpxor 12 * 32(%rsi), %ymm11, %ymm11;
	vpxor 13 * 32(%rsi), %ymm10, %ymm10;
	vpxor 14 * 32(%rsi), %ymm9, %ymm9;
	vpxor 15 * 32(%rsi), %ymm8, %ymm8;

	write_output(%ymm7, %ymm6, %ymm5, %ymm4, %ymm3, %ymm2, %ymm1, %ymm0,
		     %ymm15, %ymm14, %ymm13, %ymm12, %ymm11, %ymm10, %ymm9,
		     %ymm8, %rsi);

	vzeroall;

	movq (16 * 32 + 0 * 8)(%rsp), %r10;
	movq (16 * 32 + 1 * 8)(%rsp), %r11;
	movq (16 * 32 + 2 * 8)(%rsp), %r12;
	movq (16 * 32 + 3 * 8)(%rsp), %r13;
	CFI_RESTORE(%r10);
	CFI_RESTORE(%r11);
	CFI_RESTORE(%r12);
	CFI_RESTORE(%r13);

	leave;
	CFI_LEAVE();
	ret_spec_stop;
	CFI_ENDPROC();
ELF(.size FUNC_NAME(ocb_enc),.-FUNC_NAME(ocb_enc);)

.align 16
.globl FUNC_NAME(ocb_dec)
ELF(.type   FUNC_NAME(ocb_dec),@function;)

FUNC_NAME(ocb_dec):
	/* input:
	 *	%rdi: ctx, CTX
	 *	%rsi: dst (32 blocks)
	 *	%rdx: src (32 blocks)
	 *	%rcx: offset
	 *	%r8 : checksum
	 *	%r9 : L pointers (void *L[32])
	 */
	CFI_STARTPROC();

	pushq %rbp;
	CFI_PUSH(%rbp);
	movq %rsp, %rbp;
	CFI_DEF_CFA_REGISTER(%rbp);

	subq $(16 * 32 + 4 * 8), %rsp;
	andq $~63, %rsp;
	movq %rsp, %rax;

	movq %r10, (16 * 32 + 0 * 8)(%rsp);
	movq %r11, (16 * 32 + 1 * 8)(%rsp);
	movq %r12, (16 * 32 + 2 * 8)(%rsp);
	movq %r13, (16 * 32 + 3 * 8)(%rsp);
	CFI_REG_ON_STACK(r10, 16 * 32 + 0 * 8);
	CFI_REG_ON_STACK(r11, 16 * 32 + 1 * 8);
	CFI_REG_ON_STACK(r12, 16 * 32 + 2 * 8);
	CFI_REG_ON_STACK(r13, 16 * 32 + 3 * 8);

	vmovdqu (%rcx), %xmm14;

	/* Offset_i = Offset_{i-1} xor L_{ntz(i)} */
	/* P_i = Offset_i xor DECIPHER(K, C_i xor Offset_i)  */

#define OCB_INPUT(n, l0reg, l1reg, yreg) \
	  vmovdqu (n * 32)(%rdx), yreg; \
	  vpxor (l0reg), %xmm14, %xmm15; \
	  vpxor (l1reg), %xmm15, %xmm14; \
	  vinserti128 $1, %xmm14, %ymm15, %ymm15; \
	  vpxor yreg, %ymm15, yreg; \
	  vmovdqu %ymm15, (n * 32)(%rsi);

	movq (0 * 8)(%r9), %r10;
	movq (1 * 8)(%r9), %r11;
	movq (2 * 8)(%r9), %r12;
	movq (3 * 8)(%r9), %r13;
	OCB_INPUT(0, %r10, %r11, %ymm0);
	vmovdqu %ymm0, (15 * 32)(%rax);
	OCB_INPUT(1, %r12, %r13, %ymm0);
	vmovdqu %ymm0, (14 * 32)(%rax);
	movq (4 * 8)(%r9), %r10;
	movq (5 * 8)(%r9), %r11;
	movq (6 * 8)(%r9), %r12;
	movq (7 * 8)(%r9), %r13;
	OCB_INPUT(2, %r10, %r11, %ymm13);
	OCB_INPUT(3, %r12, %r13, %ymm12);
	movq (8 * 8)(%r9), %r10;
	movq (9 * 8)(%r9), %r11;
	movq (10 * 8)(%r9), %r12;
	movq (11 * 8)(%r9), %r13;
	OCB_INPUT(4, %r10, %r11, %ymm11);
	OCB_INPUT(5, %r12, %r13, %ymm10);
	movq (12 * 8)(%r9), %r10;
	movq (13 * 8)(%r9), %r11;
	movq (14 * 8)(%r9), %r12;
	movq (15 * 8)(%r9), %r13;
	OCB_INPUT(6, %r10, %r11, %ymm9);
	OCB_INPUT(7, %r12, %r13, %ymm8);
	movq (16 * 8)(%r9), %r10;
	movq (17 * 8)(%r9), %r11;
	movq (18 * 8)(%r9), %r12;
	movq (19 * 8)(%r9), %r13;
	OCB_INPUT(8, %r10, %r11, %ymm7);
	OCB_INPUT(9, %r12, %r13, %ymm6);
	movq (20 * 8)(%r9), %r10;
	movq (21 * 8)(%r9), %r11;
	movq (22 * 8)(%r9), %r12;
	movq (23 * 8)(%r9), %r13;
	OCB_INPUT(10, %r10, %r11, %ymm5);
	OCB_INPUT(11, %r12, %r13, %ymm4);
	movq (24 * 8)(%r9), %r10;
	movq (25 * 8)(%r9), %r11;
	movq (26 * 8)(%r9), %r12;
	movq (27 * 8)(%r9), %r13;
	OCB_INPUT(12, %r10, %r11, %ymm3);
	OCB_INPUT(13, %r12, %r13, %ymm2);
	movq (28 * 8)(%r9), %r10;
	movq (29 * 8)(%r9), %r11;
	movq (30 * 8)(%r9), %r12;
	movq (31 * 8)(%r9), %r13;
	OCB_INPUT(14, %r10, %r11, %ymm1);
	OCB_INPUT(15, %r12, %r13, %ymm0);
#undef OCB_INPUT

	vmovdqu %xmm14, (%rcx);

	movq %r8, %r10;

	cmpl $128, key_bitlength(CTX);
	movl $32, %r8d;
	movl $24, %r9d;
	cmovel %r9d, %r8d; /* max */

	/* inpack32_pre: */
	vpbroadcastq (key_table)(CTX, %r8, 8), %ymm15;
	vpshufb .Lpack_bswap rRIP, %ymm15, %ymm15;
	vpxor %ymm0, %ymm15, %ymm0;
	vpxor %ymm1, %ymm15, %ymm1;
	vpxor %ymm2, %ymm15, %ymm2;
	vpxor %ymm3, %ymm15, %ymm3;
	vpxor %ymm4, %ymm15, %ymm4;
	vpxor %ymm5, %ymm15, %ymm5;
	vpxor %ymm6, %ymm15, %ymm6;
	vpxor %ymm7, %ymm15, %ymm7;
	vpxor %ymm8, %ymm15, %ymm8;
	vpxor %ymm9, %ymm15, %ymm9;
	vpxor %ymm10, %ymm15, %ymm10;
	vpxor %ymm11, %ymm15, %ymm11;
	vpxor %ymm12, %ymm15, %ymm12;
	vpxor %ymm13, %ymm15, %ymm13;
	vpxor 14 * 32(%rax), %ymm15, %ymm14;
	vpxor 15 * 32(%rax), %ymm15, %ymm15;

	call FUNC_NAME(dec_blk32);

	vpxor 0 * 32(%rsi), %ymm7, %ymm7;
	vpxor 1 * 32(%rsi), %ymm6, %ymm6;
	vpxor 2 * 32(%rsi), %ymm5, %ymm5;
	vpxor 3 * 32(%rsi), %ymm4, %ymm4;
	vpxor 4 * 32(%rsi), %ymm3, %ymm3;
	vpxor 5 * 32(%rsi), %ymm2, %ymm2;
	vpxor 6 * 32(%rsi), %ymm1, %ymm1;
	vpxor 7 * 32(%rsi), %ymm0, %ymm0;
	vmovdqu %ymm7, (7 * 32)(%rax);
	vmovdqu %ymm6, (6 * 32)(%rax);
	vpxor 8 * 32(%rsi), %ymm15, %ymm15;
	vpxor 9 * 32(%rsi), %ymm14, %ymm14;
	vpxor 10 * 32(%rsi), %ymm13, %ymm13;
	vpxor 11 * 32(%rsi), %ymm12, %ymm12;
	vpxor 12 * 32(%rsi), %ymm11, %ymm11;
	vpxor 13 * 32(%rsi), %ymm10, %ymm10;
	vpxor 14 * 32(%rsi), %ymm9, %ymm9;
	vpxor 15 * 32(%rsi), %ymm8, %ymm8;

	/* Checksum_i = Checksum_{i-1} xor P_i  */

	vpxor %ymm5, %ymm7, %ymm7;
	vpxor %ymm4, %ymm6, %ymm6;
	vpxor %ymm3, %ymm7, %ymm7;
	vpxor %ymm2, %ymm6, %ymm6;
	vpxor %ymm1, %ymm7, %ymm7;
	vpxor %ymm0, %ymm6, %ymm6;
	vpxor %ymm15, %ymm7, %ymm7;
	vpxor %ymm14, %ymm6, %ymm6;
	vpxor %ymm13, %ymm7, %ymm7;
	vpxor %ymm12, %ymm6, %ymm6;
	vpxor %ymm11, %ymm7, %ymm7;
	vpxor %ymm10, %ymm6, %ymm6;
	vpxor %ymm9, %ymm7, %ymm7;
	vpxor %ymm8, %ymm6, %ymm6;
	vpxor %ymm7, %ymm6, %ymm7;

	vextracti128 $1, %ymm7, %xmm6;
	vpxor %xmm6, %xmm7, %xmm7;
	vpxor (%r10), %xmm7, %xmm7;
	vmovdqu %xmm7, (%r10);

	vmovdqu 7 * 32(%rax), %ymm7;
	vmovdqu 6 * 32(%rax), %ymm6;

	write_output(%ymm7, %ymm6, %ymm5, %ymm4, %ymm3, %ymm2, %ymm1, %ymm0,
		     %ymm15, %ymm14, %ymm13, %ymm12, %ymm11, %ymm10, %ymm9,
		     %ymm8, %rsi);

	vzeroall;

	movq (16 * 32 + 0 * 8)(%rsp), %r10;
	movq (16 * 32 + 1 * 8)(%rsp), %r11;
	movq (16 * 32 + 2 * 8)(%rsp), %r12;
	movq (16 * 32 + 3 * 8)(%rsp), %r13;
	CFI_RESTORE(%r10);
	CFI_RESTORE(%r11);
	CFI_RESTORE(%r12);
	CFI_RESTORE(%r13);

	leave;
	CFI_LEAVE();
	ret_spec_stop;
	CFI_ENDPROC();
ELF(.size FUNC_NAME(ocb_dec),.-FUNC_NAME(ocb_dec);)

.align 16
.globl FUNC_NAME(ocb_auth)
ELF(.type   FUNC_NAME(ocb_auth),@function;)

FUNC_NAME(ocb_auth):
	/* input:
	 *	%rdi: ctx, CTX
	 *	%rsi: abuf (16 blocks)
	 *	%rdx: offset
	 *	%rcx: checksum
	 *	%r8 : L pointers (void *L[16])
	 */
	CFI_STARTPROC();

	pushq %rbp;
	CFI_PUSH(%rbp);
	movq %rsp, %rbp;
	CFI_DEF_CFA_REGISTER(%rbp);

	subq $(16 * 32 + 4 * 8), %rsp;
	andq $~63, %rsp;
	movq %rsp, %rax;

	movq %r10, (16 * 32 + 0 * 8)(%rsp);
	movq %r11, (16 * 32 + 1 * 8)(%rsp);
	movq %r12, (16 * 32 + 2 * 8)(%rsp);
	movq %r13, (16 * 32 + 3 * 8)(%rsp);
	CFI_REG_ON_STACK(r10, 16 * 32 + 0 * 8);
	CFI_REG_ON_STACK(r11, 16 * 32 + 1 * 8);
	CFI_REG_ON_STACK(r12, 16 * 32 + 2 * 8);
	CFI_REG_ON_STACK(r13, 16 * 32 + 3 * 8);

	vmovdqu (%rdx), %xmm14;

	/* Offset_i = Offset_{i-1} xor L_{ntz(i)} */
	/* Checksum_i = Checksum_{i-1} xor P_i  */
	/* C_i = Offset_i xor ENCIPHER(K, P_i xor Offset_i)  */

#define OCB_INPUT(n, l0reg, l1reg, yreg) \
	  vmovdqu (n * 32)(%rsi), yreg; \
	  vpxor (l0reg), %xmm14, %xmm15; \
	  vpxor (l1reg), %xmm15, %xmm14; \
	  vinserti128 $1, %xmm14, %ymm15, %ymm15; \
	  vpxor yreg, %ymm15, yreg;

	movq (0 * 8)(%r8), %r10;
	movq (1 * 8)(%r8), %r11;
	movq (2 * 8)(%r8), %r12;
	movq (3 * 8)(%r8), %r13;
	OCB_INPUT(0, %r10, %r11, %ymm0);
	vmovdqu %ymm0, (15 * 32)(%rax);
	OCB_INPUT(1, %r12, %r13, %ymm0);
	vmovdqu %ymm0, (14 * 32)(%rax);
	movq (4 * 8)(%r8), %r10;
	movq (5 * 8)(%r8), %r11;
	movq (6 * 8)(%r8), %r12;
	movq (7 * 8)(%r8), %r13;
	OCB_INPUT(2, %r10, %r11, %ymm13);
	OCB_INPUT(3, %r12, %r13, %ymm12);
	movq (8 * 8)(%r8), %r10;
	movq (9 * 8)(%r8), %r11;
	movq (10 * 8)(%r8), %r12;
	movq (11 * 8)(%r8), %r13;
	OCB_INPUT(4, %r10, %r11, %ymm11);
	OCB_INPUT(5, %r12, %r13, %ymm10);
	movq (12 * 8)(%r8), %r10;
	movq (13 * 8)(%r8), %r11;
	movq (14 * 8)(%r8), %r12;
	movq (15 * 8)(%r8), %r13;
	OCB_INPUT(6, %r10, %r11, %ymm9);
	OCB_INPUT(7, %r12, %r13, %ymm8);
	movq (16 * 8)(%r8), %r10;
	movq (17 * 8)(%r8), %r11;
	movq (18 * 8)(%r8), %r12;
	movq (19 * 8)(%r8), %r13;
	OCB_INPUT(8, %r10, %r11, %ymm7);
	OCB_INPUT(9, %r12, %r13, %ymm6);
	movq (20 * 8)(%r8), %r10;
	movq (21 * 8)(%r8), %r11;
	movq (22 * 8)(%r8), %r12;
	movq (23 * 8)(%r8), %r13;
	OCB_INPUT(10, %r10, %r11, %ymm5);
	OCB_INPUT(11, %r12, %r13, %ymm4);
	movq (24 * 8)(%r8), %r10;
	movq (25 * 8)(%r8), %r11;
	movq (26 * 8)(%r8), %r12;
	movq (27 * 8)(%r8), %r13;
	OCB_INPUT(12, %r10, %r11, %ymm3);
	OCB_INPUT(13, %r12, %r13, %ymm2);
	movq (28 * 8)(%r8), %r10;
	movq (29 * 8)(%r8), %r11;
	movq (30 * 8)(%r8), %r12;
	movq (31 * 8)(%r8), %r13;
	OCB_INPUT(14, %r10, %r11, %ymm1);
	OCB_INPUT(15, %r12, %r13, %ymm0);
#undef OCB_INPUT

	vmovdqu %xmm14, (%rdx);

	cmpl $128, key_bitlength(CTX);
	movl $32, %r8d;
	movl $24, %r10d;
	cmovel %r10d, %r8d; /* max */

	movq %rcx, %r10;

	/* inpack32_pre: */
	vpbroadcastq (key_table)(CTX), %ymm15;
	vpshufb .Lpack_bswap rRIP, %ymm15, %ymm15;
	vpxor %ymm0, %ymm15, %ymm0;
	vpxor %ymm1, %ymm15, %ymm1;
	vpxor %ymm2, %ymm15, %ymm2;
	vpxor %ymm3, %ymm15, %ymm3;
	vpxor %ymm4, %ymm15, %ymm4;
	vpxor %ymm5, %ymm15, %ymm5;
	vpxor %ymm6, %ymm15, %ymm6;
	vpxor %ymm7, %ymm15, %ymm7;
	vpxor %ymm8, %ymm15, %ymm8;
	vpxor %ymm9, %ymm15, %ymm9;
	vpxor %ymm10, %ymm15, %ymm10;
	vpxor %ymm11, %ymm15, %ymm11;
	vpxor %ymm12, %ymm15, %ymm12;
	vpxor %ymm13, %ymm15, %ymm13;
	vpxor 14 * 32(%rax), %ymm15, %ymm14;
	vpxor 15 * 32(%rax), %ymm15, %ymm15;

	call FUNC_NAME(enc_blk32);

	vpxor %ymm7, %ymm6, %ymm6;
	vpxor %ymm5, %ymm4, %ymm4;
	vpxor %ymm3, %ymm2, %ymm2;
	vpxor %ymm1, %ymm0, %ymm0;
	vpxor %ymm15, %ymm14, %ymm14;
	vpxor %ymm13, %ymm12, %ymm12;
	vpxor %ymm11, %ymm10, %ymm10;
	vpxor %ymm9, %ymm8, %ymm8;

	vpxor %ymm6, %ymm4, %ymm4;
	vpxor %ymm2, %ymm0, %ymm0;
	vpxor %ymm14, %ymm12, %ymm12;
	vpxor %ymm10, %ymm8, %ymm8;

	vpxor %ymm4, %ymm0, %ymm0;
	vpxor %ymm12, %ymm8, %ymm8;

	vpxor %ymm0, %ymm8, %ymm0;

	vextracti128 $1, %ymm0, %xmm1;
	vpxor (%r10), %xmm0, %xmm0;
	vpxor %xmm0, %xmm1, %xmm0;
	vmovdqu %xmm0, (%r10);

	vzeroall;

	movq (16 * 32 + 0 * 8)(%rsp), %r10;
	movq (16 * 32 + 1 * 8)(%rsp), %r11;
	movq (16 * 32 + 2 * 8)(%rsp), %r12;
	movq (16 * 32 + 3 * 8)(%rsp), %r13;
	CFI_RESTORE(%r10);
	CFI_RESTORE(%r11);
	CFI_RESTORE(%r12);
	CFI_RESTORE(%r13);

	leave;
	CFI_LEAVE();
	ret_spec_stop;
	CFI_ENDPROC();
ELF(.size FUNC_NAME(ocb_auth),.-FUNC_NAME(ocb_auth);)

.align 16
.globl FUNC_NAME(enc_blk1_32)
ELF(.type   FUNC_NAME(enc_blk1_32),@function;)

FUNC_NAME(enc_blk1_32):
	/* input:
	 *	%rdi: ctx, CTX
	 *	%rsi: dst (32 blocks)
	 *	%rdx: src (32 blocks)
	 *	%ecx: nblocks (1 to 32)
	 */
	CFI_STARTPROC();

	pushq %rbp;
	CFI_PUSH(%rbp);
	movq %rsp, %rbp;
	CFI_DEF_CFA_REGISTER(%rbp);

	movl %ecx, %r9d;

	cmpl $128, key_bitlength(CTX);
	movl $32, %r8d;
	movl $24, %eax;
	cmovel %eax, %r8d; /* max */

	subq $(16 * 32), %rsp;
	andq $~63, %rsp;
	movq %rsp, %rax;

	cmpl $31, %ecx;
	vpxor %xmm0, %xmm0, %xmm0;
	ja .Lenc_blk32;
	jb 2f;
	  vmovdqu 15 * 32(%rdx), %xmm0;
	2:
	  vmovdqu %ymm0, (%rax);

	vpbroadcastq (key_table)(CTX), %ymm0;
	vpshufb .Lpack_bswap rRIP, %ymm0, %ymm0;

#define LOAD_INPUT(offset, ymm) \
	cmpl $(1 + 2 * (offset)), %ecx; \
	jb 2f; \
	ja 1f; \
	  vmovdqu (offset) * 32(%rdx), %ymm##_x; \
	  vpxor %ymm0, %ymm, %ymm; \
	  jmp 2f; \
	1: \
	  vpxor (offset) * 32(%rdx), %ymm0, %ymm;

	LOAD_INPUT(0, ymm15);
	LOAD_INPUT(1, ymm14);
	LOAD_INPUT(2, ymm13);
	LOAD_INPUT(3, ymm12);
	LOAD_INPUT(4, ymm11);
	LOAD_INPUT(5, ymm10);
	LOAD_INPUT(6, ymm9);
	LOAD_INPUT(7, ymm8);
	LOAD_INPUT(8, ymm7);
	LOAD_INPUT(9, ymm6);
	LOAD_INPUT(10, ymm5);
	LOAD_INPUT(11, ymm4);
	LOAD_INPUT(12, ymm3);
	LOAD_INPUT(13, ymm2);
	LOAD_INPUT(14, ymm1);
	vpxor (%rax), %ymm0, %ymm0;

2:
	call FUNC_NAME(enc_blk32);

#define STORE_OUTPUT(ymm, offset) \
	cmpl $(1 + 2 * (offset)), %r9d; \
	jb 2f; \
	ja 1f; \
	  vmovdqu %ymm##_x, (offset) * 32(%rsi); \
	  jmp 2f; \
	1: \
	  vmovdqu %ymm, (offset) * 32(%rsi);

	STORE_OUTPUT(ymm7, 0);
	STORE_OUTPUT(ymm6, 1);
	STORE_OUTPUT(ymm5, 2);
	STORE_OUTPUT(ymm4, 3);
	STORE_OUTPUT(ymm3, 4);
	STORE_OUTPUT(ymm2, 5);
	STORE_OUTPUT(ymm1, 6);
	STORE_OUTPUT(ymm0, 7);
	STORE_OUTPUT(ymm15, 8);
	STORE_OUTPUT(ymm14, 9);
	STORE_OUTPUT(ymm13, 10);
	STORE_OUTPUT(ymm12, 11);
	STORE_OUTPUT(ymm11, 12);
	STORE_OUTPUT(ymm10, 13);
	STORE_OUTPUT(ymm9, 14);
	STORE_OUTPUT(ymm8, 15);
	jmp .Lenc_blk32_done;

.align 8
.Lenc_blk32:
	inpack32_pre(%ymm0, %ymm1, %ymm2, %ymm3, %ymm4, %ymm5, %ymm6, %ymm7,
		     %ymm8, %ymm9, %ymm10, %ymm11, %ymm12, %ymm13, %ymm14,
		     %ymm15, %rdx, (key_table)(CTX));

	call FUNC_NAME(enc_blk32);

	write_output(%ymm7, %ymm6, %ymm5, %ymm4, %ymm3, %ymm2, %ymm1, %ymm0,
		     %ymm15, %ymm14, %ymm13, %ymm12, %ymm11, %ymm10, %ymm9,
		     %ymm8, %rsi);

.align 8
2:
.Lenc_blk32_done:
	vzeroall;

	leave;
	CFI_LEAVE();
	ret_spec_stop;
	CFI_ENDPROC();
ELF(.size FUNC_NAME(enc_blk1_32),.-FUNC_NAME(enc_blk1_32);)

.align 16
.globl FUNC_NAME(dec_blk1_32)
ELF(.type   FUNC_NAME(dec_blk1_32),@function;)

FUNC_NAME(dec_blk1_32):
	/* input:
	 *	%rdi: ctx, CTX
	 *	%rsi: dst (32 blocks)
	 *	%rdx: src (32 blocks)
	 *	%ecx: nblocks (1 to 32)
	 */
	CFI_STARTPROC();

	pushq %rbp;
	CFI_PUSH(%rbp);
	movq %rsp, %rbp;
	CFI_DEF_CFA_REGISTER(%rbp);

	movl %ecx, %r9d;

	cmpl $128, key_bitlength(CTX);
	movl $32, %r8d;
	movl $24, %eax;
	cmovel %eax, %r8d; /* max */

	subq $(16 * 32), %rsp;
	andq $~63, %rsp;
	movq %rsp, %rax;

	cmpl $31, %ecx;
	vpxor %xmm0, %xmm0, %xmm0;
	ja .Ldec_blk32;
	jb 2f;
	  vmovdqu 15 * 32(%rdx), %xmm0;
	2:
	  vmovdqu %ymm0, (%rax);

	vpbroadcastq (key_table)(CTX, %r8, 8), %ymm0;
	vpshufb .Lpack_bswap rRIP, %ymm0, %ymm0;

	LOAD_INPUT(0, ymm15);
	LOAD_INPUT(1, ymm14);
	LOAD_INPUT(2, ymm13);
	LOAD_INPUT(3, ymm12);
	LOAD_INPUT(4, ymm11);
	LOAD_INPUT(5, ymm10);
	LOAD_INPUT(6, ymm9);
	LOAD_INPUT(7, ymm8);
	LOAD_INPUT(8, ymm7);
	LOAD_INPUT(9, ymm6);
	LOAD_INPUT(10, ymm5);
	LOAD_INPUT(11, ymm4);
	LOAD_INPUT(12, ymm3);
	LOAD_INPUT(13, ymm2);
	LOAD_INPUT(14, ymm1);
	vpxor (%rax), %ymm0, %ymm0;

2:
	call FUNC_NAME(dec_blk32);

	STORE_OUTPUT(ymm7, 0);
	STORE_OUTPUT(ymm6, 1);
	STORE_OUTPUT(ymm5, 2);
	STORE_OUTPUT(ymm4, 3);
	STORE_OUTPUT(ymm3, 4);
	STORE_OUTPUT(ymm2, 5);
	STORE_OUTPUT(ymm1, 6);
	STORE_OUTPUT(ymm0, 7);
	STORE_OUTPUT(ymm15, 8);
	STORE_OUTPUT(ymm14, 9);
	STORE_OUTPUT(ymm13, 10);
	STORE_OUTPUT(ymm12, 11);
	STORE_OUTPUT(ymm11, 12);
	STORE_OUTPUT(ymm10, 13);
	STORE_OUTPUT(ymm9, 14);
	STORE_OUTPUT(ymm8, 15);

.align 8
2:
.Ldec_blk32_done:
	vzeroall;

	leave;
	CFI_LEAVE();
	ret_spec_stop;

.align 8
.Ldec_blk32:
	inpack32_pre(%ymm0, %ymm1, %ymm2, %ymm3, %ymm4, %ymm5, %ymm6, %ymm7,
		     %ymm8, %ymm9, %ymm10, %ymm11, %ymm12, %ymm13, %ymm14,
		     %ymm15, %rdx, (key_table)(CTX, %r8, 8));

	call FUNC_NAME(dec_blk32);

	write_output(%ymm7, %ymm6, %ymm5, %ymm4, %ymm3, %ymm2, %ymm1, %ymm0,
		     %ymm15, %ymm14, %ymm13, %ymm12, %ymm11, %ymm10, %ymm9,
		     %ymm8, %rsi);
	jmp .Ldec_blk32_done;
	CFI_ENDPROC();
ELF(.size FUNC_NAME(dec_blk1_32),.-FUNC_NAME(dec_blk1_32);)

#endif /* GCRY_CAMELLIA_AESNI_AVX2_AMD64_H */
