/* mpi-cmp.c  -  MPI functions
 * Copyright (C) 1998, 1999, 2001, 2002, 2005 Free Software Foundation, Inc.
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

#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include "mpi-internal.h"

int
_gcry_mpi_cmp_ui (gcry_mpi_t u, unsigned long v)
{
  mpi_limb_t limb = v;

  _gcry_mpi_normalize (u);

  /* Handle the case that U contains no limb.  */
  if (u->nlimbs == 0)
    return -(limb != 0);

  /* Handle the case that U is negative.  */
  if (u->sign)
    return -1;

  if (u->nlimbs == 1)
    {
      /* Handle the case that U contains exactly one limb.  */

      if (u->d[0] > limb)
	return 1;
      if (u->d[0] < limb)
	return -1;
      return 0;
    }
  else
    /* Handle the case that U contains more than one limb.  */
    return 1;
}


/* Helper for _gcry_mpi_cmp and _gcry_mpi_cmpabs.  */
static int
do_mpi_cmp (gcry_mpi_t u, gcry_mpi_t v, int absmode)
{
  mpi_size_t usize;
  mpi_size_t vsize;
  int usign;
  int vsign;
  int cmp;

  if (mpi_is_opaque (u) || mpi_is_opaque (v))
    {
      /* We have no signan and thus ABSMODE has no efeect here.  */
      if (mpi_is_opaque (u) && !mpi_is_opaque (v))
        return -1;
      if (!mpi_is_opaque (u) && mpi_is_opaque (v))
        return 1;
      if (!u->sign && !v->sign)
        return 0; /* Empty buffers are identical.  */
      if (u->sign < v->sign)
        return -1;
      if (u->sign > v->sign)
        return 1;
      return memcmp (u->d, v->d, (u->sign+7)/8);
    }
  else
    {
      _gcry_mpi_normalize (u);
      _gcry_mpi_normalize (v);

      usize = u->nlimbs;
      vsize = v->nlimbs;
      usign = absmode? 0 : u->sign;
      vsign = absmode? 0 : v->sign;

      /* Special treatment for +0 == -0 */
      if (!usize && !vsize)
        return 0;

      /* Compare sign bits.  */
      if (!usign && vsign)
        return 1;
      if (usign && !vsign)
        return -1;

      /* U and V are either both positive or both negative.  */

      if (usize != vsize && !usign && !vsign)
        return usize - vsize;
      if (usize != vsize && usign && vsign)
        return vsize + usize;
      if (!usize )
        return 0;
      if (!(cmp = _gcry_mpih_cmp (u->d, v->d, usize)))
        return 0;
      if ((cmp < 0?1:0) == (usign?1:0))
        return 1;
    }
  return -1;
}


int
_gcry_mpi_cmp (gcry_mpi_t u, gcry_mpi_t v)
{
  return do_mpi_cmp (u, v, 0);
}

/* Compare only the absolute values.  */
int
_gcry_mpi_cmpabs (gcry_mpi_t u, gcry_mpi_t v)
{
  return do_mpi_cmp (u, v, 1);
}
