/* aeswrap.c -  AESWRAP mode regression tests
 *	Copyright (C) 2009 Free Software Foundation, Inc.
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
 * License along with this program; if not, see <https://www.gnu.org/licenses/>.
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#define PGM "aeswrap"
#include "t-common.h"


static void
check_one (int algo,
           const void *kek, size_t keklen,
           const void *data, size_t datalen,
           const void *expected, size_t expectedlen,
           int inplace)
{
  gcry_error_t err;
  gcry_cipher_hd_t hd;
  unsigned char outbuf[32+8];
  size_t outbuflen;

  err = gcry_cipher_open (&hd, algo, GCRY_CIPHER_MODE_AESWRAP, 0);
  if (err)
    {
      fail ("gcry_cipher_open failed: %s\n", gpg_strerror (err));
      return;
    }

  err = gcry_cipher_setkey (hd, kek, keklen);
  if (err)
    {
      fail ("gcry_cipher_setkey failed: %s\n", gpg_strerror (err));
      return;
    }

  outbuflen = datalen + 8;
  if (outbuflen > sizeof outbuf)
    {
      err = gpg_error (GPG_ERR_INTERNAL);
    }
  else if (inplace)
    {
      memcpy (outbuf, data, datalen);
      err = gcry_cipher_encrypt (hd, outbuf, outbuflen, outbuf, datalen);
    }
  else
    {
      err = gcry_cipher_encrypt (hd, outbuf, outbuflen, data, datalen);
    }

  if (err)
    {
      fail ("gcry_cipher_encrypt failed: %s\n", gpg_strerror (err));
      return;
    }

  if (outbuflen != expectedlen || memcmp (outbuf, expected, expectedlen))
    {
      const unsigned char *s;
      int i;

      fail ("mismatch at encryption!%s\n", inplace ? " (inplace)" : "");
      fprintf (stderr, "computed: ");
      for (i = 0; i < outbuflen; i++)
	fprintf (stderr, "%02x ", outbuf[i]);
      fprintf (stderr, "\nexpected: ");
      for (s = expected, i = 0; i < expectedlen; s++, i++)
        fprintf (stderr, "%02x ", *s);
      putc ('\n', stderr);
    }


  outbuflen = expectedlen - 8;
  if (outbuflen > sizeof outbuf)
    {
      err = gpg_error (GPG_ERR_INTERNAL);
    }
  else if (inplace)
    {
      memcpy (outbuf, expected, expectedlen);
      err = gcry_cipher_decrypt (hd, outbuf, outbuflen, outbuf, expectedlen);
    }
  else
    {
      err = gcry_cipher_decrypt (hd, outbuf, outbuflen, expected, expectedlen);
    }

  if (err)
    {
      fail ("gcry_cipher_decrypt failed: %s\n", gpg_strerror (err));
      return;
    }

  if (outbuflen != datalen || memcmp (outbuf, data, datalen))
    {
      const unsigned char *s;
      int i;

      fail ("mismatch at decryption!%s\n", inplace ? " (inplace)" : "");
      fprintf (stderr, "computed: ");
      for (i = 0; i < outbuflen; i++)
	fprintf (stderr, "%02x ", outbuf[i]);
      fprintf (stderr, "\nexpected: ");
      for (s = data, i = 0; i < datalen; s++, i++)
        fprintf (stderr, "%02x ", *s);
      putc ('\n', stderr);
    }

  /* Now the last step again with a key reset. */
  gcry_cipher_reset (hd);

  outbuflen = expectedlen - 8;
  if (outbuflen > sizeof outbuf)
    {
      err = gpg_error (GPG_ERR_INTERNAL);
    }
  else if (inplace)
    {
      memcpy (outbuf, expected, expectedlen);
      err = gcry_cipher_decrypt (hd, outbuf, outbuflen, outbuf, expectedlen);
    }
  else
    {
      err = gcry_cipher_decrypt (hd, outbuf, outbuflen, expected, expectedlen);
    }

  if (err)
    {
      fail ("gcry_cipher_decrypt(2) failed: %s\n", gpg_strerror (err));
      return;
    }

  if (outbuflen != datalen || memcmp (outbuf, data, datalen))
    fail ("mismatch at decryption(2)!%s\n", inplace ? " (inplace)" : "");

  /* And once more without a key reset. */
  outbuflen = expectedlen - 8;
  if (outbuflen > sizeof outbuf)
    {
      err = gpg_error (GPG_ERR_INTERNAL);
    }
  else if (inplace)
    {
      memcpy (outbuf, expected, expectedlen);
      err = gcry_cipher_decrypt (hd, outbuf, outbuflen, outbuf, expectedlen);
    }
  else
    {
      err = gcry_cipher_decrypt (hd, outbuf, outbuflen, expected, expectedlen);
    }

  if (err)
    {
      fail ("gcry_cipher_decrypt(3) failed: %s\n", gpg_strerror (err));
      return;
    }

  if (outbuflen != datalen || memcmp (outbuf, data, datalen))
    fail ("mismatch at decryption(3)!%s\n", inplace ? " (inplace)" : "");

  gcry_cipher_close (hd);
}


static void
check (int algo,
       const void *kek, size_t keklen,
       const void *data, size_t datalen,
       const void *expected, size_t expectedlen)
{
  check_one (algo, kek, keklen, data, datalen, expected, expectedlen, 0);
  check_one (algo, kek, keklen, data, datalen, expected, expectedlen, 1);
}


static void
check_one_with_padding (int algo,
                        const void *kek, size_t keklen,
                        const void *data, size_t datalen,
                        const void *expected, size_t expectedlen)
{
  gcry_error_t err;
  gcry_cipher_hd_t hd;
  unsigned char outbuf[4*16];
  size_t outbuflen;

  err = gcry_cipher_open (&hd, algo, GCRY_CIPHER_MODE_AESWRAP,
                          GCRY_CIPHER_EXTENDED);
  if (err)
    {
      fail ("gcry_cipher_open failed: %s\n", gpg_strerror (err));
      return;
    }

  err = gcry_cipher_setkey (hd, kek, keklen);
  if (err)
    {
      fail ("gcry_cipher_setkey failed: %s\n", gpg_strerror (err));
      gcry_cipher_close (hd);
      return;
    }

  outbuflen = ((datalen+7)/8) * 8 + 8;
  if (outbuflen > sizeof outbuf)
    {
      err = gpg_error (GPG_ERR_INTERNAL);
    }
  else
    {
      err = gcry_cipher_encrypt (hd, outbuf, outbuflen, data, datalen);
    }

  if (err)
    {
      fail ("gcry_cipher_encrypt failed: %s\n", gpg_strerror (err));
      gcry_cipher_close (hd);
      return;
    }

  if (outbuflen != expectedlen || memcmp (outbuf, expected, expectedlen))
    {
      const unsigned char *s;
      int i;

      fail ("mismatch at encryption!(padding)\n");
      fprintf (stderr, "computed: ");
      for (i = 0; i < outbuflen; i++)
	fprintf (stderr, "%02x ", outbuf[i]);
      fprintf (stderr, "\nexpected: ");
      for (s = expected, i = 0; i < expectedlen; s++, i++)
        fprintf (stderr, "%02x ", *s);
      putc ('\n', stderr);
    }

  outbuflen = ((datalen+7)/8) * 8 + 8;
  if (outbuflen > sizeof outbuf)
    {
      err = gpg_error (GPG_ERR_INTERNAL);
    }
  else
    {
      err = gcry_cipher_decrypt (hd, outbuf, outbuflen, expected, expectedlen);
      if (!err)
        {
          unsigned char plen[4];
          size_t nbytes;
          err = gcry_cipher_info (hd, GCRYCTL_GET_KEYLEN, plen, &nbytes);
          if (!err)
            outbuflen = (plen[0] << 24) | (plen[1] << 16)
              | (plen[2] << 8) | plen[3];
        }
    }

  if (err)
    {
      fail ("gcry_cipher_decrypt failed: %s\n", gpg_strerror (err));
      gcry_cipher_close (hd);
      return;
    }

  if (outbuflen != datalen || memcmp (outbuf, data, datalen))
    {
      const unsigned char *s;
      int i;

      fail ("mismatch at decryption!(padding)\n");
      fprintf (stderr, "computed: ");
      for (i = 0; i < outbuflen; i++)
	fprintf (stderr, "%02x ", outbuf[i]);
      fprintf (stderr, "\nexpected: ");
      for (s = data, i = 0; i < datalen; s++, i++)
        fprintf (stderr, "%02x ", *s);
      putc ('\n', stderr);
    }

  /* Now the last step again with a key reset. */
  gcry_cipher_reset (hd);

  outbuflen = ((datalen+7)/8) * 8 + 8;
  if (outbuflen > sizeof outbuf)
    {
      err = gpg_error (GPG_ERR_INTERNAL);
    }
  else
    {
      err = gcry_cipher_decrypt (hd, outbuf, outbuflen, expected, expectedlen);
      if (!err)
        {
          unsigned char plen[4];
          size_t nbytes;
          err = gcry_cipher_info (hd, GCRYCTL_GET_KEYLEN, plen, &nbytes);
          if (!err)
            outbuflen = (plen[0] << 24) | (plen[1] << 16)
              | (plen[2] << 8) | plen[3];
        }
    }

  if (err)
    {
      fail ("gcry_cipher_decrypt(2) failed: %s\n", gpg_strerror (err));
      gcry_cipher_close (hd);
      return;
    }

  if (outbuflen != datalen || memcmp (outbuf, data, datalen))
    fail ("mismatch at decryption(2)(padding)!\n");

  /* And once more without a key reset. */
  outbuflen = ((datalen+7)/8) * 8 + 8;
  if (outbuflen > sizeof outbuf)
    {
      err = gpg_error (GPG_ERR_INTERNAL);
    }
  else
    {
      err = gcry_cipher_decrypt (hd, outbuf, outbuflen, expected, expectedlen);
      if (!err)
        {
          unsigned char plen[4];
          size_t nbytes;
          err = gcry_cipher_info (hd, GCRYCTL_GET_KEYLEN, plen, &nbytes);
          if (!err)
            outbuflen = (plen[0] << 24) | (plen[1] << 16)
              | (plen[2] << 8) | plen[3];
        }
    }

  if (err)
    {
      fail ("gcry_cipher_decrypt(3) failed: %s\n", gpg_strerror (err));
      gcry_cipher_close (hd);
      return;
    }

  if (outbuflen != datalen || memcmp (outbuf, data, datalen))
    fail ("mismatch at decryption(3)(padding)!\n");

  gcry_cipher_close (hd);
}


static void
check_all (void)
{
  if (verbose)
    fprintf (stderr, "4.1 Wrap 128 bits of Key Data with a 128-bit KEK\n");
  check
    (GCRY_CIPHER_AES128,
     "\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F", 16,
     "\x00\x11\x22\x33\x44\x55\x66\x77\x88\x99\xAA\xBB\xCC\xDD\xEE\xFF", 16,
     "\x1F\xA6\x8B\x0A\x81\x12\xB4\x47\xAE\xF3\x4B\xD8\xFB\x5A\x7B\x82"
     "\x9D\x3E\x86\x23\x71\xD2\xCF\xE5", 24);

  if (verbose)
    fprintf (stderr, "4.2 Wrap 128 bits of Key Data with a 192-bit KEK\n");
  check
    (GCRY_CIPHER_AES192,
     "\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F"
     "\x10\x11\x12\x13\x14\x15\x16\x17", 24,
     "\x00\x11\x22\x33\x44\x55\x66\x77\x88\x99\xAA\xBB\xCC\xDD\xEE\xFF", 16,
     "\x96\x77\x8B\x25\xAE\x6C\xA4\x35\xF9\x2B\x5B\x97\xC0\x50\xAE\xD2"
     "\x46\x8A\xB8\xA1\x7A\xD8\x4E\x5D", 24);

  if (verbose)
    fprintf (stderr, "4.3 Wrap 128 bits of Key Data with a 256-bit KEK\n");
  check
    (GCRY_CIPHER_AES256,
     "\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F"
     "\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F", 32,
     "\x00\x11\x22\x33\x44\x55\x66\x77\x88\x99\xAA\xBB\xCC\xDD\xEE\xFF", 16,
     "\x64\xE8\xC3\xF9\xCE\x0F\x5B\xA2\x63\xE9\x77\x79\x05\x81\x8A\x2A"
     "\x93\xC8\x19\x1E\x7D\x6E\x8A\xE7", 24);

  if (verbose)
    fprintf (stderr, "4.4 Wrap 192 bits of Key Data with a 192-bit KEK\n");
  check
    (GCRY_CIPHER_AES192,
     "\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F"
     "\x10\x11\x12\x13\x14\x15\x16\x17", 24,
     "\x00\x11\x22\x33\x44\x55\x66\x77\x88\x99\xAA\xBB\xCC\xDD\xEE\xFF"
     "\x00\x01\x02\x03\x04\x05\x06\x07", 24,
     "\x03\x1D\x33\x26\x4E\x15\xD3\x32\x68\xF2\x4E\xC2\x60\x74\x3E\xDC"
     "\xE1\xC6\xC7\xDD\xEE\x72\x5A\x93\x6B\xA8\x14\x91\x5C\x67\x62\xD2", 32);

  if (verbose)
    fprintf (stderr, "4.5 Wrap 192 bits of Key Data with a 256-bit KEK\n");
  check
    (GCRY_CIPHER_AES256,
     "\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F"
     "\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F", 32,
     "\x00\x11\x22\x33\x44\x55\x66\x77\x88\x99\xAA\xBB\xCC\xDD\xEE\xFF"
     "\x00\x01\x02\x03\x04\x05\x06\x07", 24,
     "\xA8\xF9\xBC\x16\x12\xC6\x8B\x3F\xF6\xE6\xF4\xFB\xE3\x0E\x71\xE4"
     "\x76\x9C\x8B\x80\xA3\x2C\xB8\x95\x8C\xD5\xD1\x7D\x6B\x25\x4D\xA1", 32);

  if (verbose)
    fprintf (stderr, "4.6 Wrap 256 bits of Key Data with a 256-bit KEK\n");
  check
    (GCRY_CIPHER_AES,
     "\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F"
     "\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F", 32,
     "\x00\x11\x22\x33\x44\x55\x66\x77\x88\x99\xAA\xBB\xCC\xDD\xEE\xFF"
     "\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F", 32,
     "\x28\xC9\xF4\x04\xC4\xB8\x10\xF4\xCB\xCC\xB3\x5C\xFB\x87\xF8\x26"
     "\x3F\x57\x86\xE2\xD8\x0E\xD3\x26\xCB\xC7\xF0\xE7\x1A\x99\xF4\x3B"
     "\xFB\x98\x8B\x9B\x7A\x02\xDD\x21", 40);

  if (verbose)
    fprintf (stderr, "6 Wrap 160 bits of Key Data with a 192-bit KEK\n");
  check_one_with_padding
    (GCRY_CIPHER_AES192,
     "\x58\x40\xdf\x6e\x29\xb0\x2a\xf1\xab\x49\x3b\x70\x5b\xf1\x6e\xa1"
     "\xae\x83\x38\xf4\xdc\xc1\x76\xa8", 24,
     "\xc3\x7b\x7e\x64\x92\x58\x43\x40\xbe\xd1\x22\x07\x80\x89\x41\x15"
     "\x50\x68\xf7\x38", 20,
     "\x13\x8b\xde\xaa\x9b\x8f\xa7\xfc\x61\xf9\x77\x42\xe7\x22\x48\xee"
     "\x5a\xe6\xae\x53\x60\xd1\xae\x6a\x5f\x54\xf3\x73\xfa\x54\x3b\x6a", 32);

  if (verbose)
    fprintf (stderr, "6 Wrap 56 bits of Key Data with a 192-bit KEK\n");
  check_one_with_padding
    (GCRY_CIPHER_AES192,
     "\x58\x40\xdf\x6e\x29\xb0\x2a\xf1\xab\x49\x3b\x70\x5b\xf1\x6e\xa1"
     "\xae\x83\x38\xf4\xdc\xc1\x76\xa8", 24,
     "\x46\x6f\x72\x50\x61\x73\x69", 7,
     "\xaf\xbe\xb0\xf0\x7d\xfb\xf5\x41\x92\x00\xf2\xcc\xb5\x0b\xb2\x4f", 16);
}

int
main (int argc, char **argv)
{
  if (argc > 1 && !strcmp (argv[1], "--verbose"))
    verbose = 1;
  else if (argc > 1 && !strcmp (argv[1], "--debug"))
    verbose = debug = 1;

  if (!gcry_check_version (GCRYPT_VERSION))
    die ("version mismatch\n");

  xgcry_control ((GCRYCTL_DISABLE_SECMEM, 0));
  xgcry_control ((GCRYCTL_INITIALIZATION_FINISHED, 0));
  if (debug)
    xgcry_control ((GCRYCTL_SET_DEBUG_FLAGS, 1u, 0));
  check_all ();

  return error_count ? 1 : 0;
}
