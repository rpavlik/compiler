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


//-*-c++-*-
/* =======================================================================
 * =======================================================================
 *
 *  Module: tn_map.h
 *  $Revision$
 *  $Date$
 *  $Author$
 *  $Source$
 *
 *  Description:
 *  ============
 *
 *  General TN_MAP facility for dynamically attaching information to
 *  TNs. There are two kinds of TN_MAPs provided:
 *
 *  1. TN_MAP: A vector indexed by TN_number. There is a generation count 
 *     field to avoid reinitialization overhead.
 *
 *  2. hTN_MAP: A hash table of size TN_MAP_HASH (=256). This may be useful 
 *     for TN_MAPs that are very sparsely populated. 
 *
 *  There are separate interfaces for the two kinds of TN_MAPs. The first
 *  kind (TN_MAP) are allocated from the PU_pool. The interface routines 
 *  are:
 *
 *  void TN_MAP_Init (void)
 *	Initialize the TN_MAP package for a PU.
 *
 *  TN_MAP TN_MAP_Create (void)
 *	Create and return a new TN_MAP.
 *
 *  void TN_MAP_Set (TN_MAP map, TN *tn, void *value)
 *	Set the <map> value for <tn> to <value>.
 *
 *  void *TN_MAP_Get ( TN_MAP map, const TN *tn)
 *	Lookup the <map> value for <tn>. If no value has been previously
 *      set, this procedure returns NULL.
 *
 *  void TN_MAP_Delete (TN_MAP map)
 *	Deallocate the <map> and release the associated memory.
 *
 *
 *
 *  The second kind of TN_MAPs (hTN_MAP) is allocated in the memory pool
 *  passed in as a parameter during hTN_MAP_Create. The freeing up of 
 *  the space has to be explicitly done by the user. The memory pool
 *  is assumed to be of non-zeroing type.
 *  The implementation of hTN_MAP uses a hash table of size TN_MAP_HASH. 
 *  This allows for space savings if only a small fraction of TNs are 
 *  assigned values. This is typically true for TN_MAPs associated with the 
 *  TNs in a single basic block.
 *  Three variants of hTN_MAPs are provided: hTN_MAP, hTN_MAP32, and
 *  hTN_MAP64, with values of type void*, INT32, and INT64 respectively.
 *  The interface routines are:
 *
 *  hTN_MAP hTN_MAP_Create(MEM_POOL *pool)
 *	Create and return a new hTN_MAP allocated to <pool>.
 *
 *  hTN_MAP32 hTN_MAP32_Create(MEM_POOL *pool)
 *	Create and return a new hTN_MAP32 allocated to <pool>.
 *
 *  hTN_MAP64 hTN_MAP64_Create(MEM_POOL *pool)
 *	Create and return a new hTN_MAP64 allocated to <pool>.
 *
 *  void hTN_MAP_Set(hTN_MAP map, TN *tn, void *value)
 *      Set the <map> value for <tn> to <value>.
 *
 *  void hTN_MAP32_Set(hTN_MAP32 map, TN *tn, INT32 value)
 *      Set the <map> value for <tn> to <value>.
 *
 *  void hTN_MAP64_Set(hTN_MAP64 map, TN *tn, INT64 value)
 *      Set the <map> value for <tn> to <value>.
 *
 *  void *hTN_MAP_Get(hTN_MAP map, TN *tn)
 *	Lookup the <map> value for <tn>.  If no value has been previously
 *	set, the procedure returns NULL.
 *
 *  INT32 hTN_MAP32_Get(hTN_MAP32 map, TN *tn)
 *	Lookup the <map> value for <tn>.  If no value has been previously
 *	set, the procedure returns 0.
 *
 *  INT64 hTN_MAP64_Get(hTN_MAP64 map, TN *tn)
 *	Lookup the <map> value for <tn>.  If no value has been previously
 *	set, the procedure returns 0.
 *
 *  void *hTN_MAP_Get_And_Set(hTN_MAP map, TN *tn, void *value)
 *	Lookup the <map> value for <tn>, and then set the <map> value for
 *      <tn> to <value>.  If no value has been previously set, the
 *      procedure returns NULL.
 *
 *  INT32 hTN_MAP32_Get_And_Set(hTN_MAP32 map, TN *tn, INT32 value)
 *	Lookup the <map> value for <tn>, and then set the <map> value for
 *      <tn> to <value>.  If no value has been previously set, the
 *      procedure returns 0.
 *
 *  INT64 hTN_MAP64_Get_And_Set(hTN_MAP64 map, TN *tn, INT64 value)
 *	Lookup the <map> value for <tn>, and then set the <map> value for
 *      <tn> to <value>.  If no value has been previously set, the
 *      procedure returns 0.
 *
 *
 * =======================================================================
 * ======================================================================= */

#ifndef TN_MAP_INCLUDED
#define TN_MAP_INCLUDED

#include "defs.h"
#include "mempool.h"
#include "tn.h"

typedef struct tn_map *TN_MAP;
extern void TN_MAP_Init (void);
extern TN_MAP TN_MAP_Create (void);
extern void TN_MAP_Set (TN_MAP map, TN *tn, void *value);
extern void *TN_MAP_Get ( TN_MAP map, const TN *tn);
extern void TN_MAP_Delete (TN_MAP map);


#define TN_MAP_HASH 256
typedef struct htn_map   *hTN_MAP;
typedef struct htn_map32 *hTN_MAP32;
typedef struct htn_map64 *hTN_MAP64;
extern hTN_MAP   hTN_MAP_Create   (MEM_POOL *pool);
extern hTN_MAP32 hTN_MAP32_Create (MEM_POOL *pool);
extern hTN_MAP64 hTN_MAP64_Create (MEM_POOL *pool);
extern void hTN_MAP_Set   (hTN_MAP   map, TN *tn, void *value);
extern void hTN_MAP32_Set (hTN_MAP32 map, TN *tn, INT32 value);
extern void hTN_MAP64_Set (hTN_MAP64 map, TN *tn, INT64 value);
extern void *hTN_MAP_Get   (hTN_MAP   map, TN *tn);
extern INT32 hTN_MAP32_Get (hTN_MAP32 map, TN *tn);
extern INT64 hTN_MAP64_Get (hTN_MAP64 map, TN *tn);
extern void *hTN_MAP_Get_And_Set   (hTN_MAP   map, TN *tn, void *value);
extern INT32 hTN_MAP32_Get_And_Set (hTN_MAP32 map, TN *tn, INT32 value);
extern INT64 hTN_MAP64_Get_And_Set (hTN_MAP64 map, TN *tn, INT64 value);

#endif /* TN_MAP_INCLUDED */
