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


/* $Header: /home/bos/bk/kpro64-pending/libF77/pow_di.c 1.7 04/12/21 14:58:03-08:00 bos@eng-25.internal.keyresearch.com $ */
#include "cmplrs/host.h"
#include <math.h>
#include "moremath.h"

extern double __powdi(double x, int32 n);
extern double __powdl(double x, int64 n);

/* By-reference versions for backward compatibility. */

double pow_di(double *ap, int32 *bp)
{
double pow;
pow=__powdi(*ap,*bp);
return pow;
}

double pow_dl(double *ap, int64 *bp)
{
double pow;
pow=__powdl(*ap,*bp);
return pow;
}
