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


/* USMID @(#) libu/ffio/bufa_parse.h	92.0	10/08/98 14:57:41 */

/*  BUFA LAYER ASSIGN PARSING DEFINITION */

#define NUM_BUFA_OPTS     0
#define NUM_BUFA_ALIAS    0

struct LAYER_DATA _bufa_data =
    {
         CLASS_BUFA ,
         FLD_EXT_TYPE,
         "bufa",
         "" ,
         0,
         0,
         NUM_BUFA_OPTS,
         1 ,
         NUM_NBUF_NUMERICS,
         NUM_BUFA_ALIAS ,
         NULL ,
         _nbuf_numerics ,
         NULL,
         NULL
    } ;
