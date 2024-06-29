/* secmem.h -  internal definitions for secmem
 *	Copyright (C) 2000, 2001, 2002, 2003 Free Software Foundation, Inc.
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

#ifndef G10_SECMEM_H
#define G10_SECMEM_H 1

void _gcry_secmem_init (size_t npool);
void _gcry_secmem_term (void);
void *_gcry_secmem_malloc (size_t size, int xhint) _GCRY_GCC_ATTR_MALLOC;
void *_gcry_secmem_realloc (void *a, size_t newsize, int xhint);
int  _gcry_secmem_free (void *a);
void _gcry_secmem_dump_stats (int extended);
void _gcry_secmem_set_auto_expand (unsigned int chunksize);
void _gcry_secmem_set_flags (unsigned flags);
unsigned _gcry_secmem_get_flags(void);
int _gcry_private_is_secure (const void *p);

/* Flags for _gcry_secmem_{set,get}_flags.  */
#define GCRY_SECMEM_FLAG_NO_WARNING      (1 << 0)
#define GCRY_SECMEM_FLAG_SUSPEND_WARNING (1 << 1)
#define GCRY_SECMEM_FLAG_NOT_LOCKED      (1 << 2)
#define GCRY_SECMEM_FLAG_NO_MLOCK        (1 << 3)
#define GCRY_SECMEM_FLAG_NO_PRIV_DROP    (1 << 4)

#endif /* G10_SECMEM_H */
