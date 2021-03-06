/*

  Copyright (C) 2000, 2001 Silicon Graphics, Inc.  All Rights Reserved.

   Path64 is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

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



static const char source_file[] = __FILE__;
static const char rcs_id[] = "$Source$ $Revision$";


/* ====================================================================
 *
 *  X_PROP_Create
 *
 *  See interface description
 *
 * ====================================================================
 */

_X_PROP_TYPE_ *
_X_PROP_CREATE_(
  INT32     universe_size,
  MEM_POOL *pool
)
{
  /* We allocate fixed size bit vector, preceeded by its size.
   * Round up size to words and add 1 for the size.
   */
  UINT32 words = (  (universe_size + _X_PROP_TYPE_SIZE_ - 1)
		  >> _X_PROP_TYPE_SIZE_LOG2_) + 1;
  _X_PROP_TYPE_ *prop = TYPE_MEM_POOL_ALLOC_N(_X_PROP_TYPE_, pool, words);
  if ( ! MEM_POOL_Zeroed(pool) ) memset(prop, 0, words * sizeof(_X_PROP_TYPE_));
  prop[0] = universe_size;
  return prop;
}


/* ====================================================================
 *
 *  X_PROP_Set
 *
 *  See interface description
 *
 * ====================================================================
 */

void
_X_PROP_SET_(
  _X_PROP_TYPE_            *prop,
  _X_PROP_LOCAL_BASE_TYPE_  x
)
{
  UINT32 id = _X_id_(x);

  Is_True(id < prop[0], ("property id out of range"));

  (prop+1)[id >> _X_PROP_TYPE_SIZE_LOG2_] |=
	(_X_PROP_TYPE_)1 << (id & (_X_PROP_TYPE_SIZE_-1));
}


/* ====================================================================
 *
 *  X_PROP_Reset
 *
 *  See interface description
 *
 * ====================================================================
 */

void
_X_PROP_RESET_(
  _X_PROP_TYPE_            *prop,
  _X_PROP_LOCAL_BASE_TYPE_  x
)
{
  UINT32 id = _X_id_(x);

  Is_True(id < prop[0], ("property id out of range"));

  (prop+1)[id >> _X_PROP_TYPE_SIZE_LOG2_] &= 
	~((_X_PROP_TYPE_)1 << (id & (_X_PROP_TYPE_SIZE_-1)));
}


/* ====================================================================
 *
 *  X_PROP_Get
 *
 *  See interface description
 *
 * ====================================================================
 */

BOOL
_X_PROP_GET_(
  _X_PROP_TYPE_            *prop,
  _X_PROP_LOCAL_BASE_TYPE_  x
)
{
  UINT32 id = _X_id_(x);

  Is_True(id < prop[0], ("property id out of range"));

  return   ((prop+1)[id >> _X_PROP_TYPE_SIZE_LOG2_] 
	 & ((_X_PROP_TYPE_)1 << (id & (_X_PROP_TYPE_SIZE_-1)))) != 0;
}


/* ====================================================================
 *
 *  X_PROP_Uniond
 *
 *  See interface description
 *
 * ====================================================================
 */

void
_X_PROP_UNIOND_(
  _X_PROP_TYPE_ *prop0,
  _X_PROP_TYPE_ *prop1
)
{
  UINT32 i;
  UINT32 universe_size = prop0[0];
  UINT32 words =   (universe_size + _X_PROP_TYPE_SIZE_-1) 
		>> _X_PROP_TYPE_SIZE_LOG2_;

  Is_True(universe_size == prop1[0], ("vectors are different length"));

  for ( i = 1; i <= words; ++i ) prop0[i] |= prop1[i];
}


/* ====================================================================
 *
 *  X_PROP_Intersection_Is_NonEmpty
 *
 *  See interface description
 *
 * ====================================================================
 */

BOOL
_X_PROP_INTERSECTION_IS_NONEMPTY_(
  _X_PROP_TYPE_ *prop0,
  _X_PROP_TYPE_ *prop1
)
{
  UINT32 i;
  UINT32 universe_size = prop0[0];
  UINT32 words =   (universe_size + _X_PROP_TYPE_SIZE_-1) 
		>> _X_PROP_TYPE_SIZE_LOG2_;

  Is_True(universe_size == prop1[0], ("vectors are different length"));

  for ( i = 1; i <= words; ++i ) {
    if ( (prop0[i] & prop1[i]) != 0 ) return TRUE;
  }
  return FALSE;
}
