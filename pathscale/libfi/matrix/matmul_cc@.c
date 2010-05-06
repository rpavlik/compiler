/*
 * Copyright 2004, 2005, 2006 PathScale, Inc.  All Rights Reserved.
 */

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


#pragma ident "@(#) libfi/matrix/matmul_cc@.c	92.1	07/09/99 15:18:08"

#include "matmul.h"

/*
 * Name of this entry point
 */

#define NAME _MATMUL_CC

/*
 * Fortran subroutine called to do computation:
 * On UNICOS systems with libsci, the first set of entry point names will
 *   be used to call libsci multitasked routines.
 * On non-UNICOS systems, the second set of entry point names will be used
 *   when the new U option is available to allow the name to retain the
 *   use of uppercase rather than being converted to lowercase letters.
 * Until the U option is available, the last set of entry point names will
 *   be used.
 */
#ifdef _UNICOS
#define SUBNAME1 CGEMMX@
#define SUBNAME2 CGEMVX@
#elif defined(UNIX_NAMING_CONVENTION)
#define SUBNAME1 _CGEMMX
#define SUBNAME2 _CGEMVX
#elif defined(BUILD_COMPILER_GNU) && defined(BUILD_OS_DARWIN)
#define SUBNAME1 underscore_cgemmx__
#define SUBNAME2 underscore_cgemvx__
#else
#define SUBNAME1 _cgemmx__
#define SUBNAME2 _cgemvx__
#endif

/*
 * Type of constants alpha and beta
 */
#define RESULTTYPE	_f_comp8

/* The entry point MATMUL_CC@ is used by CF77 on c90, ymp, cray-2, and xmp */
#ifdef _UNICOS
#pragma _CRI duplicate _MATMUL_CC as MATMUL_CC@
#endif
void
NAME(DopeVectorType * RESULT, DopeVectorType * MATRIX_A,
     DopeVectorType * MATRIX_B)
{
    void    SUBNAME1();
    void    SUBNAME2();
    const int       noconj = 0;
    MatrixDimenType matdimdata, *MATDIM;

#if COMPLEX_AS_SCALAR
    const RESULTTYPE one = (RESULTTYPE) 1.0;
    const RESULTTYPE zero = (RESULTTYPE) 0.0;
#else
    const RESULTTYPE one = {{ 1.0, 0.0 }};
    const RESULTTYPE zero = {{ 0.0, 0.0 }};
#endif

        MATDIM = (MatrixDimenType *) &matdimdata;

    /*
     * Parse dope vectors, and perform error checking.
     */

    _premult(RESULT, MATRIX_A, MATRIX_B, MATDIM);

    /*
     * Perform the matrix multiplication.
     */

    if (MATDIM->ndimb == 1) {
	/*
	 * y = Ax
	 */
	SUBNAME2(&noconj, &MATDIM->n1a, &MATDIM->n2a, &one, MATDIM->A,
	       &MATDIM->inc1a, &MATDIM->inc2a, MATDIM->B, &MATDIM->inc1b,
	       &zero, MATDIM->C, &MATDIM->inc1c);
	return;
    } else if (MATDIM->ndima == 1) {
	/*
	 * y = xB, equivalent to y' = B'x'
	 */
	SUBNAME2(&noconj, &MATDIM->n2b, &MATDIM->n1b, &one, MATDIM->B,
	       &MATDIM->inc2b,  &MATDIM->inc1b, MATDIM->A, &MATDIM->inc1a,
	       &zero, MATDIM->C, &MATDIM->inc1c);
	return;
    } else {
	/*
	 * C = AB (full matrix multiplication)
	 */
	SUBNAME1(&noconj, &noconj, &MATDIM->n1a, &MATDIM->n2b, &MATDIM->n2a,
		&one, MATDIM->A, &MATDIM->inc1a, &MATDIM->inc2a, MATDIM->B,
		&MATDIM->inc1b, &MATDIM->inc2b, &zero, MATDIM->C,
		&MATDIM->inc1c, &MATDIM->inc2c);
	return;
    }
}
