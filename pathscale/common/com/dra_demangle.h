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


#ifndef dra_demangle_INCLUDED
#define dra_demangle_INCLUDED

/* ====================================================================
 * ====================================================================
 *
 * Module: dra_demangle.h
 *
 * Revision history:
 *  20-Aug-96: Original Version, nenad
 *
 * Description: Interface for demangling utility functions
 *              needed in emitting user-understandable error
 *              messages by the (dsm_)prelinker, linker, and
 *              run-time error checking code generated by LNO.
 *
 * ====================================================================
 * ==================================================================== 
 */

#ifdef __cplusplus
extern "C" {
#endif

/* 
 * Extract only the function name
 */
extern char* DRA_Demangle_Func (const char* mangled_name);

/* 
 * Extract and demangle the argument list
 */
extern char* DRA_Demangle_Arglist (const char* mangled_name,
                                   const char dim_order);

/*
 * Extract and demangle both the function name and the argument list
 */
extern char* DRA_Demangle (const char* mangled_name,
                           const char dim_order);

#ifdef __cplusplus
}
#endif


/*
 * Acceptable dimension order specifiers 
 */

#define DRA_DIMS_ROWWISE    'C'  /* C as in C, C++  */
#define DRA_DIMS_COLUMNWISE 'F'  /* F as in Fortran */
#define DRA_DIMS_UNKNOWN   '\0'  /* Uninitialized   */


#endif /* dra_demangle_INCLUDED */
