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
#ifndef ff_pragmas_INCLUDED
#define ff_pragmas_INCLUDED

/**
*** Module: ff_pragmas.h
*** $Revision$
*** $Date$
*** $Author$
*** $Source$
*** 
*** Description:
*** 
*** extern void LWN_Process_FF_Pragmas(WN* func_nd);
***
***   Scan for fission/fusion pragmas in the source and either insert proper
***   information into DO_LOOP_INFO structure or perform fission and fusion
***   as specified. Currently handles the following pragmas:
*** 
***       WN_PRAGMA_AGGRESSIVE_INNER_LOOP_FISSION
***       WN_PRAGMA_FISSION
***       WN_PRAGMA_FISSIONABLE
***       WN_PRAGMA_FUSE
***       WN_PRAGMA_FUSEABLE
***       WN_PRAGMA_NO_FISSION
***       WN_PRAGMA_NO_FUSION
***
***       WN_PRAGMA_INTERCHANGE
***       WN_PRAGMA_NO_INTERCHANGE
***       WN_PRAGMA_BLOCKING_SIZE
***       WN_PRAGMA_NO_BLOCKING
***       WN_PRAGMA_UNROLL
***       WN_PRAGMA_BLOCKABLE
*** 
*** extern void  LNO_Insert_Pragmas(func_nd)
***
***   For inner loops with required unroll data on the DO_LOOP_INFO, put
***   in appropriate pragmas for later on.
***
**/

#ifdef _KEEP_RCS_ID
static char *ff_pragmas_rcs_id = "$Source$ $Revision$";
#endif /* _KEEP_RCS_ID */

#include "wn.h"

extern void LWN_Process_FF_Pragmas(WN* func_nd);
extern void  LNO_Insert_Pragmas(WN* func_nd);

#endif // ff_pragmas_INCLUDED

