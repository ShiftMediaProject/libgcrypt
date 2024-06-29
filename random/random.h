/* random.h - random functions
 *	Copyright (C) 1998, 2002, 2006 Free Software Foundation, Inc.
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
#ifndef G10_RANDOM_H
#define G10_RANDOM_H

#include "types.h"
#include "../src/gcrypt-testapi.h"  /* struct gcry_drbg_test_vector */

/*-- random.c --*/
void _gcry_register_random_progress (void (*cb)(void *,const char*,int,int,int),
                                     void *cb_data );

void _gcry_set_preferred_rng_type (int type);
void _gcry_random_initialize (int full);
void _gcry_random_close_fds (void);
int  _gcry_get_rng_type (int ignore_fips_mode);
void _gcry_random_dump_stats(void);
void _gcry_secure_random_alloc(void);
void _gcry_enable_quick_random_gen (void);
int  _gcry_random_is_faked(void);
void _gcry_set_random_seed_file (const char *name);
void _gcry_update_random_seed_file (void);

void _gcry_fast_random_poll( void );

gcry_err_code_t _gcry_random_init_external_test (void **r_context,
                                                 unsigned int flags,
                                                 const void *key,
                                                 size_t keylen,
                                                 const void *seed,
                                                 size_t seedlen,
                                                 const void *dt,
                                                 size_t dtlen);
gcry_err_code_t _gcry_random_run_external_test (void *context,
                                                char *buffer, size_t buflen);
void            _gcry_random_deinit_external_test (void *context);

/*-- random-drbg.c --*/
gpg_err_code_t _gcry_rngdrbg_reinit (const char *flagstr,
                                     gcry_buffer_t *pers, int npers);
gpg_err_code_t _gcry_rngdrbg_cavs_test (struct gcry_drbg_test_vector *t,
                                        unsigned char *buf);
gpg_err_code_t _gcry_rngdrbg_healthcheck_one (struct gcry_drbg_test_vector *t);

/*-- rndegd.c --*/
gpg_error_t _gcry_rndegd_set_socket_name (const char *name);

/*-- rndjent.c --*/
unsigned int _gcry_rndjent_get_version (int *r_active);


#endif /*G10_RANDOM_H*/
