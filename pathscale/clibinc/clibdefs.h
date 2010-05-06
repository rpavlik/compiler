/* USMID @(#) clibinc/clibdefs.h	92.2	07/26/99 12:57:02 */
 
 
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

#ifndef _CLIBDEFS_H
#define _CLIBDEFS_H

/*
 * Note SV2 is part of _CRAY.
 */
#if	defined(_LITTLE_ENDIAN)		/* Little Endian IEEE */
#include <sys/cdefs.h>

#ifndef _WORD32
#define	_WORD32		1		/* sizeof(int) is 4 bytes/32 bits */
#endif

#elif	_CRAY

#include <sys/cdefs.h>

#elif defined(_ABSOFT)

#ifndef _WORD32
#define _WORD32		1		/* sizeof(int) is 4 bytes/32 bits */
#endif
#ifndef _ADDR32
#define _ADDR32		1		/* sizeof(int*) is 4 bytes/32 bits */
#endif
#ifndef _CRAYIEEE
#define _CRAYIEEE	1		/* IEEE arithmetic */
#endif

#include <sys/cdefs.h>

#elif	defined(__mips)

#include <sys/cdefs.h>

#if 	_MIPS_SZLONG == 32
#define	_WORD32		1		/* sizeof(long) is 4 bytes/32 bits */
#endif

#else	/* Solaris assumed */

#define	_SOLARIS	1
#define	_WORD32		1		/* sizeof(int) is 4 bytes/32 bits */
#define	_ADDR32		1		/* sizeof(int*) is 4 bytes/32 bits */
#define	_CRAYIEEE	1		/* IEEE arithmetic */

#if defined(__cplusplus)
#define __BEGIN_DECLS   extern "C" {
#define __END_DECLS     };
#define _DOTDOTDOT ...
#else
#define __BEGIN_DECLS
#define __END_DECLS
#define _DOTDOTDOT
#endif

#if defined(__STDC__) || defined(__cplusplus)
#define __(_A)  _A
#else
#define __(_A)  ()
#endif

#endif	/* Solaris */

#endif	/* ! _CLIBDEFS_H */
