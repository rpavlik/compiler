/*

  Copyright (C) 2000, 2001 Silicon Graphics, Inc.  All Rights Reserved.

   Path64 is free software; you can redistribute it and/or modify it
   under the terms of the GNU Lesser General Public License as published by
   the Free Software Foundation version 2.1

   Path64 is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
   License for more details.

   You should have received a copy of the GNU General Public License
   along with Path64; see the file COPYING.  If not, write to the Free
   Software Foundation, 51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301, USA.

   Special thanks goes to SGI for their continued support to open source

*/


/* ====================================================================
 * ====================================================================
 *
 * Module: vexpftab.c
 * $Revision$
 * $Date$
 * $Author$
 * $Source$
 *
 * Revision history:
 *  09-Jun-93 - Original Version
 *
 * Description:	tables used by vexpf function
 *
 * ====================================================================
 * ====================================================================
 */

static char *rcs_id = "$Source$ $Revision$";

#include "libm.h"

/* 2**k/32, in double precision */

const du	__expftab[] =
{
{D(0x3ff00000, 0x00000000)},
{D(0x3ff059b0, 0xd3158574)},
{D(0x3ff0b558, 0x6cf9890f)},
{D(0x3ff11301, 0xd0125b51)},
{D(0x3ff172b8, 0x3c7d517b)},
{D(0x3ff1d487, 0x3168b9aa)},
{D(0x3ff2387a, 0x6e756238)},
{D(0x3ff29e9d, 0xf51fdee1)},
{D(0x3ff306fe, 0x0a31b715)},
{D(0x3ff371a7, 0x373aa9cb)},
{D(0x3ff3dea6, 0x4c123422)},
{D(0x3ff44e08, 0x6061892d)},
{D(0x3ff4bfda, 0xd5362a27)},
{D(0x3ff5342b, 0x569d4f82)},
{D(0x3ff5ab07, 0xdd485429)},
{D(0x3ff6247e, 0xb03a5585)},
{D(0x3ff6a09e, 0x667f3bcd)},
{D(0x3ff71f75, 0xe8ec5f74)},
{D(0x3ff7a114, 0x73eb0187)},
{D(0x3ff82589, 0x994cce13)},
{D(0x3ff8ace5, 0x422aa0db)},
{D(0x3ff93737, 0xb0cdc5e5)},
{D(0x3ff9c491, 0x82a3f090)},
{D(0x3ffa5503, 0xb23e255d)},
{D(0x3ffae89f, 0x995ad3ad)},
{D(0x3ffb7f76, 0xf2fb5e47)},
{D(0x3ffc199b, 0xdd85529c)},
{D(0x3ffcb720, 0xdcef9069)},
{D(0x3ffd5818, 0xdcfba487)},
{D(0x3ffdfc97, 0x337b9b5f)},
{D(0x3ffea4af, 0xa2a490da)},
{D(0x3fff5076, 0x5b6e4540)},
};

