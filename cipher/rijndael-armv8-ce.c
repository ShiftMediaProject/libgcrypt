/* ARMv8 Crypto Extension AES for Libgcrypt
 * Copyright (C) 2016, 2022 Jussi Kivilinna <jussi.kivilinna@iki.fi>
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
 *
 */

#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h> /* for memcmp() */

#include "types.h"  /* for byte and u32 typedefs */
#include "g10lib.h"
#include "cipher.h"
#include "bufhelp.h"
#include "rijndael-internal.h"
#include "./cipher-internal.h"


#ifdef USE_ARM_CE


typedef struct u128_s { u32 a, b, c, d; } u128_t;

extern u32 _gcry_aes_sbox4_armv8_ce(u32 in4b);
extern void _gcry_aes_invmixcol_armv8_ce(u128_t *dst, const u128_t *src);

extern unsigned int _gcry_aes_enc_armv8_ce(const void *keysched, byte *dst,
                                           const byte *src,
                                           unsigned int nrounds);
extern unsigned int _gcry_aes_dec_armv8_ce(const void *keysched, byte *dst,
                                           const byte *src,
                                           unsigned int nrounds);

extern void _gcry_aes_cbc_enc_armv8_ce (const void *keysched,
                                        unsigned char *outbuf,
                                        const unsigned char *inbuf,
                                        unsigned char *iv, size_t nblocks,
                                        int cbc_mac, unsigned int nrounds);
extern void _gcry_aes_cbc_dec_armv8_ce (const void *keysched,
                                        unsigned char *outbuf,
                                        const unsigned char *inbuf,
                                        unsigned char *iv, size_t nblocks,
                                        unsigned int nrounds);

extern void _gcry_aes_cfb_enc_armv8_ce (const void *keysched,
                                        unsigned char *outbuf,
                                        const unsigned char *inbuf,
                                        unsigned char *iv, size_t nblocks,
                                        unsigned int nrounds);
extern void _gcry_aes_cfb_dec_armv8_ce (const void *keysched,
                                        unsigned char *outbuf,
                                        const unsigned char *inbuf,
                                        unsigned char *iv, size_t nblocks,
                                        unsigned int nrounds);

extern void _gcry_aes_ctr_enc_armv8_ce (const void *keysched,
                                        unsigned char *outbuf,
                                        const unsigned char *inbuf,
                                        unsigned char *iv, size_t nblocks,
                                        unsigned int nrounds);

extern void _gcry_aes_ctr32le_enc_armv8_ce (const void *keysched,
                                            unsigned char *outbuf,
                                            const unsigned char *inbuf,
                                            unsigned char *iv, size_t nblocks,
                                            unsigned int nrounds);

extern size_t _gcry_aes_ocb_enc_armv8_ce (const void *keysched,
                                          unsigned char *outbuf,
                                          const unsigned char *inbuf,
                                          unsigned char *offset,
                                          unsigned char *checksum,
                                          unsigned char *L_table,
                                          size_t nblocks,
                                          unsigned int nrounds,
                                          unsigned int blkn);
extern size_t _gcry_aes_ocb_dec_armv8_ce (const void *keysched,
                                          unsigned char *outbuf,
                                          const unsigned char *inbuf,
                                          unsigned char *offset,
                                          unsigned char *checksum,
                                          unsigned char *L_table,
                                          size_t nblocks,
                                          unsigned int nrounds,
                                          unsigned int blkn);
extern size_t _gcry_aes_ocb_auth_armv8_ce (const void *keysched,
                                           const unsigned char *abuf,
                                           unsigned char *offset,
                                           unsigned char *checksum,
                                           unsigned char *L_table,
                                           size_t nblocks,
                                           unsigned int nrounds,
                                           unsigned int blkn);
extern void _gcry_aes_xts_enc_armv8_ce (const void *keysched,
                                        unsigned char *outbuf,
                                        const unsigned char *inbuf,
                                        unsigned char *tweak,
                                        size_t nblocks, unsigned int nrounds);
extern void _gcry_aes_xts_dec_armv8_ce (const void *keysched,
                                        unsigned char *outbuf,
                                        const unsigned char *inbuf,
                                        unsigned char *tweak,
                                        size_t nblocks, unsigned int nrounds);
extern void _gcry_aes_ecb_enc_armv8_ce (const void *keysched,
                                        unsigned char *outbuf,
                                        const unsigned char *inbuf,
                                        size_t nblocks, unsigned int nrounds);
extern void _gcry_aes_ecb_dec_armv8_ce (const void *keysched,
                                        unsigned char *outbuf,
                                        const unsigned char *inbuf,
                                        size_t nblocks, unsigned int nrounds);


void
_gcry_aes_armv8_ce_setkey (RIJNDAEL_context *ctx, const byte *key)
{
  unsigned int rounds = ctx->rounds;
  unsigned int KC = rounds - 6;
  u32 *W_u32 = ctx->keyschenc32b;
  unsigned int i, j;
  u32 W_prev;
  byte rcon = 1;

  for (i = 0; i < KC; i += 2)
    {
      W_u32[i + 0] = buf_get_le32(key + i * 4 + 0);
      W_u32[i + 1] = buf_get_le32(key + i * 4 + 4);
    }

  for (i = KC, j = KC, W_prev = W_u32[KC - 1];
       i < 4 * (rounds + 1);
       i += 2, j += 2)
    {
      u32 temp0 = W_prev;
      u32 temp1;

      if (j == KC)
        {
          j = 0;
          temp0 = _gcry_aes_sbox4_armv8_ce(rol(temp0, 24)) ^ rcon;
          rcon = ((rcon << 1) ^ (-(rcon >> 7) & 0x1b)) & 0xff;
        }
      else if (KC == 8 && j == 4)
        {
          temp0 = _gcry_aes_sbox4_armv8_ce(temp0);
        }

      temp1 = W_u32[i - KC + 0];

      W_u32[i + 0] = temp0 ^ temp1;
      W_u32[i + 1] = W_u32[i - KC + 1] ^ temp0 ^ temp1;
      W_prev = W_u32[i + 1];
    }
}

/* Make a decryption key from an encryption key. */
void
_gcry_aes_armv8_ce_prepare_decryption (RIJNDAEL_context *ctx)
{
  u128_t *ekey = (u128_t *)(void *)ctx->keyschenc;
  u128_t *dkey = (u128_t *)(void *)ctx->keyschdec;
  int rounds = ctx->rounds;
  int rr;
  int r;

#define DO_AESIMC() _gcry_aes_invmixcol_armv8_ce(&dkey[r], &ekey[rr])

  dkey[0] = ekey[rounds];
  r = 1;
  rr = rounds-1;
  DO_AESIMC(); r++; rr--; /* round 1 */
  DO_AESIMC(); r++; rr--; /* round 2 */
  DO_AESIMC(); r++; rr--; /* round 3 */
  DO_AESIMC(); r++; rr--; /* round 4 */
  DO_AESIMC(); r++; rr--; /* round 5 */
  DO_AESIMC(); r++; rr--; /* round 6 */
  DO_AESIMC(); r++; rr--; /* round 7 */
  DO_AESIMC(); r++; rr--; /* round 8 */
  DO_AESIMC(); r++; rr--; /* round 9 */
  if (rounds >= 12)
    {
      if (rounds > 12)
        {
          DO_AESIMC(); r++; rr--; /* round 10 */
          DO_AESIMC(); r++; rr--; /* round 11 */
        }

      DO_AESIMC(); r++; rr--; /* round 12 / 10 */
      DO_AESIMC(); r++; rr--; /* round 13 / 11 */
    }

  dkey[r] = ekey[0];

#undef DO_AESIMC
}

unsigned int
_gcry_aes_armv8_ce_encrypt (const RIJNDAEL_context *ctx, unsigned char *dst,
                            const unsigned char *src)
{
  const void *keysched = ctx->keyschenc32;
  unsigned int nrounds = ctx->rounds;

  return _gcry_aes_enc_armv8_ce(keysched, dst, src, nrounds);
}

unsigned int
_gcry_aes_armv8_ce_decrypt (const RIJNDAEL_context *ctx, unsigned char *dst,
                            const unsigned char *src)
{
  const void *keysched = ctx->keyschdec32;
  unsigned int nrounds = ctx->rounds;

  return _gcry_aes_dec_armv8_ce(keysched, dst, src, nrounds);
}

void
_gcry_aes_armv8_ce_cbc_enc (const RIJNDAEL_context *ctx, unsigned char *iv,
                            unsigned char *outbuf, const unsigned char *inbuf,
                            size_t nblocks, int cbc_mac)
{
  const void *keysched = ctx->keyschenc32;
  unsigned int nrounds = ctx->rounds;

  _gcry_aes_cbc_enc_armv8_ce(keysched, outbuf, inbuf, iv, nblocks, cbc_mac,
                             nrounds);
}

void
_gcry_aes_armv8_ce_cbc_dec (RIJNDAEL_context *ctx, unsigned char *iv,
                            unsigned char *outbuf, const unsigned char *inbuf,
                            size_t nblocks)
{
  const void *keysched = ctx->keyschdec32;
  unsigned int nrounds = ctx->rounds;

  if ( !ctx->decryption_prepared )
    {
      _gcry_aes_armv8_ce_prepare_decryption ( ctx );
      ctx->decryption_prepared = 1;
    }

  _gcry_aes_cbc_dec_armv8_ce(keysched, outbuf, inbuf, iv, nblocks, nrounds);
}

void
_gcry_aes_armv8_ce_cfb_enc (RIJNDAEL_context *ctx, unsigned char *iv,
                            unsigned char *outbuf, const unsigned char *inbuf,
                            size_t nblocks)
{
  const void *keysched = ctx->keyschenc32;
  unsigned int nrounds = ctx->rounds;

  _gcry_aes_cfb_enc_armv8_ce(keysched, outbuf, inbuf, iv, nblocks, nrounds);
}

void
_gcry_aes_armv8_ce_cfb_dec (RIJNDAEL_context *ctx, unsigned char *iv,
                            unsigned char *outbuf, const unsigned char *inbuf,
                            size_t nblocks)
{
  const void *keysched = ctx->keyschenc32;
  unsigned int nrounds = ctx->rounds;

  _gcry_aes_cfb_dec_armv8_ce(keysched, outbuf, inbuf, iv, nblocks, nrounds);
}

void
_gcry_aes_armv8_ce_ctr_enc (RIJNDAEL_context *ctx, unsigned char *iv,
                            unsigned char *outbuf, const unsigned char *inbuf,
                            size_t nblocks)
{
  const void *keysched = ctx->keyschenc32;
  unsigned int nrounds = ctx->rounds;

  _gcry_aes_ctr_enc_armv8_ce(keysched, outbuf, inbuf, iv, nblocks, nrounds);
}

void
_gcry_aes_armv8_ce_ctr32le_enc (RIJNDAEL_context *ctx, unsigned char *iv,
                                unsigned char *outbuf,
                                const unsigned char *inbuf, size_t nblocks)
{
  const void *keysched = ctx->keyschenc32;
  unsigned int nrounds = ctx->rounds;

  _gcry_aes_ctr32le_enc_armv8_ce(keysched, outbuf, inbuf, iv, nblocks, nrounds);
}

size_t
_gcry_aes_armv8_ce_ocb_crypt (gcry_cipher_hd_t c, void *outbuf_arg,
                              const void *inbuf_arg, size_t nblocks,
                              int encrypt)
{
  RIJNDAEL_context *ctx = (void *)&c->context.c;
  const void *keysched = encrypt ? ctx->keyschenc32 : ctx->keyschdec32;
  unsigned char *outbuf = outbuf_arg;
  const unsigned char *inbuf = inbuf_arg;
  unsigned int nrounds = ctx->rounds;
  u64 blkn = c->u_mode.ocb.data_nblocks;

  if ( !encrypt && !ctx->decryption_prepared )
    {
      _gcry_aes_armv8_ce_prepare_decryption ( ctx );
      ctx->decryption_prepared = 1;
    }

  c->u_mode.ocb.data_nblocks = blkn + nblocks;

  if (encrypt)
    return _gcry_aes_ocb_enc_armv8_ce (keysched, outbuf, inbuf,
				       c->u_iv.iv, c->u_ctr.ctr,
				       c->u_mode.ocb.L[0], nblocks, nrounds,
				       (unsigned int)blkn);
  else
    return _gcry_aes_ocb_dec_armv8_ce (keysched, outbuf, inbuf,
				       c->u_iv.iv, c->u_ctr.ctr,
				       c->u_mode.ocb.L[0], nblocks, nrounds,
				       (unsigned int)blkn);
}

size_t
_gcry_aes_armv8_ce_ocb_auth (gcry_cipher_hd_t c, void *abuf_arg,
                             size_t nblocks)
{
  RIJNDAEL_context *ctx = (void *)&c->context.c;
  const void *keysched = ctx->keyschenc32;
  const unsigned char *abuf = abuf_arg;
  unsigned int nrounds = ctx->rounds;
  u64 blkn = c->u_mode.ocb.aad_nblocks;

  c->u_mode.ocb.aad_nblocks = blkn + nblocks;

  return _gcry_aes_ocb_auth_armv8_ce (keysched, abuf, c->u_mode.ocb.aad_offset,
				      c->u_mode.ocb.aad_sum, c->u_mode.ocb.L[0],
				      nblocks, nrounds, (unsigned int)blkn);
}

void
_gcry_aes_armv8_ce_xts_crypt (RIJNDAEL_context *ctx, unsigned char *tweak,
			      unsigned char *outbuf, const unsigned char *inbuf,
			      size_t nblocks, int encrypt)
{
  const void *keysched = encrypt ? ctx->keyschenc32 : ctx->keyschdec32;
  unsigned int nrounds = ctx->rounds;

  if ( !encrypt && !ctx->decryption_prepared )
    {
      _gcry_aes_armv8_ce_prepare_decryption ( ctx );
      ctx->decryption_prepared = 1;
    }

  if (encrypt)
    _gcry_aes_xts_enc_armv8_ce (keysched, outbuf, inbuf, tweak,
				nblocks, nrounds);
  else
    _gcry_aes_xts_dec_armv8_ce (keysched, outbuf, inbuf, tweak,
				nblocks, nrounds);
}

void
_gcry_aes_armv8_ce_ecb_crypt (void *context, void *outbuf,
			      const void *inbuf, size_t nblocks,
			      int encrypt)
{
  RIJNDAEL_context *ctx = context;
  const void *keysched = encrypt ? ctx->keyschenc32 : ctx->keyschdec32;
  unsigned int nrounds = ctx->rounds;

  if ( !encrypt && !ctx->decryption_prepared )
    {
      _gcry_aes_armv8_ce_prepare_decryption ( ctx );
      ctx->decryption_prepared = 1;
    }

  if (encrypt)
    _gcry_aes_ecb_enc_armv8_ce (keysched, outbuf, inbuf, nblocks, nrounds);
  else
    _gcry_aes_ecb_dec_armv8_ce (keysched, outbuf, inbuf, nblocks, nrounds);
}
#endif /* USE_ARM_CE */
