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



#pragma ident "@(#) libfi/matrix/matmul_jc@.c	92.1	07/09/99 15:18:08"

#include "matmul.h"

/*
 * Name of this entry point
 */
#define NAME _MATMUL_JC
/*
 * Name of routine called do computation (if any)
 * Replace trailing @ with leading underscore when f90 is gen compiler for mpp.
 */
#if defined(_UNICOS)
#define SUBNAME JSGEMMX@
#elif defined(UNIX_NAMING_CONVENTION)
#define SUBNAME _JSGEMMX
#elif defined(BUILD_COMPILER_GNU) && defined(BUILD_OS_DARWIN)
#define SUBNAME underscore_jsgemmx_
#else
#define SUBNAME _jsgemmx_
#endif
/*
 * Type of constants alpha and beta
 */
#define RESULTTYPE	_f_real8

#if defined(_UNICOS)
#pragma _CRI duplicate _MATMUL_JC as MATMUL_JC@
#endif
void
NAME(DopeVectorType *RESULT, DopeVectorType *MATRIX_A, 
	DopeVectorType *MATRIX_B)
{
    void    SUBNAME();
    const RESULTTYPE   one =  (RESULTTYPE) 1.0;
    const RESULTTYPE   zero = (RESULTTYPE) 0.0;
    RESULTTYPE *Br, *Bi;	/* pointers to real and imaginary part of B */
    RESULTTYPE *Cr, *Ci;	/* pointers to real and imaginary part of C */
    MatrixDimenType matdimdata, *MATDIM;

        MATDIM = (MatrixDimenType *) &matdimdata;

    /*
     * Parse dope vectors, and perform error checking.
     */

    _premult(RESULT, MATRIX_A, MATRIX_B, MATDIM);

    /*
     * Do real and imaginary parts separately.
     */

    Br = (RESULTTYPE *) MATDIM->B;
    Bi = Br + 1;
    MATDIM->inc1b *= 2;
    MATDIM->inc2b *= 2;

    Cr = (RESULTTYPE *) MATDIM->C;
    Ci = Cr + 1;
    MATDIM->inc1c *= 2;
    MATDIM->inc2c *= 2;

    /*
     * Perform the matrix multiplication.
     */

    /* real part */
    SUBNAME(&MATDIM->m, &MATDIM->n, &MATDIM->k, &one, MATDIM->A, &MATDIM->inc1a,
    	    &MATDIM->inc2a, Br, &MATDIM->inc1b, &MATDIM->inc2b,
	    &zero, Cr, &MATDIM->inc1c, &MATDIM->inc2c);
    /* imaginary part */
    SUBNAME(&MATDIM->m, &MATDIM->n, &MATDIM->k, &one, MATDIM->A, &MATDIM->inc1a,
    	    &MATDIM->inc2a, Bi, &MATDIM->inc1b, &MATDIM->inc2b,
	    &zero, Ci, &MATDIM->inc1c, &MATDIM->inc2c);
    return;
}
