/*
 * Copyright 2008, 2009 PathScale, LLC.  All Rights Reserved.
 */

/*
 * Copyright (C) 2007. QLogic Corporation. All Rights Reserved.
 */

/*
 * Copyright 2003, 2004, 2005, 2006 PathScale, Inc.  All Rights Reserved.
 */

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


/**
***		Intrinsics, Intrinsic Types, and Intrinsic Flags
***		------------------------------------------------
***
*** Description:
***
***	This interface describes all the intrinsic names, operators,
***	types associated with intrinsics, and properties associated with
***	intrinsics.
***
*** Reserved Prefixes:
***
***	INTRN		for INTRINSIC members only.
***
*** Exported types:
***
***	INTRINSIC
***
***	    An enumerated type.  The members are a partial set of all language
***	    defined intrinsics.  Those language intrinsics not included in this
***	    enumerated type are already present in WHIRL as OPC nodes.  There
***	    are usually two separate flavors of each fortran intrinsic - one
***	    named INTRN_XXX and one named INTRN_XXXe.  The former name
***	    represents the version called directly by generated code and usually
***	    has call by value semantics.  These intrinsics might eventually be
***	    turned into calls, be inlined or have direct code generated for
***	    them.  The INTRN_XXXe version is always an external routine with
***	    call by reference semantics.  It is needed to support passing an
***	    intrinsic function itself to another subprogram.
***
***	    All INTRINSICs are prefixed with INTRN.
***
*** Exported data:
***
***	    none
***
**/

#ifndef wintrinsic_INCLUDED
#define wintrinsic_INCLUDED "wintrinsic.h"

#ifdef __cplusplus
extern "C" {
#endif
#ifdef TARG_ST
/* typedef */ enum {

  INTRINSIC_INVALID = -1,
  INTRINSIC_NONE = 0,
/* All intrinsic definations are moved to intrn_entry.def */
#define NEED_INTRN_ID
# include "intrn_entry.def"
#undef  NEED_INTRN_ID
  INTRINSIC_GENERAL_LAST,
#ifdef TARG_ST
#include "targ_wintrinsic.h"  /* should at least define INTRINSIC_TARG_LAST */
#else /* not an ST target */
  INTRINSIC_TARG_LAST  = INTRINSIC_GENERAL_LAST,
  INTRINSIC_LAST  = INTRINSIC_TARG_LAST,
#endif /* TARG_ST */

#if defined(TARG_SL)
  INTRN_SL_INTRN_BGN = INTRN_VBUF_OFFSET,
  INTRN_SL2_BEGIN = INTRN_C2_MVGR_R2G,
  INTRN_SL_INTRN_END = INTRN_VBUF_ABSOLUTE,
  INTRN_SL2_END = INTRN_VBUF_ABSOLUTE,
  INTRN_C3_INTRINSIC_BEGIN = INTRN_MUL_SHIFT_HI,
  INTRN_C3_INTRINSIC_END = INTRN_COPY_HI,
#endif

  INTRINSIC_FIRST = 1,

} /* INTRINSIC */;

#else
typedef enum 
{

  /* WARNING: The file common/com/intrn_info.cxx contains an array
   * intrn_info[INTRINSIC_LAST+1] whose elements are indexed using
   * the INTRISIC enum values.  Any changes to the list below must
   * be mirrored in intrn_info (including all the #ifdefs).
   */

  INTRINSIC_INVALID = -1,
  INTRINSIC_NONE = 0,
  INTRINSIC_FIRST = 1,

    /* F77 type conversions - INT, SHORT, LONG, REAL, FLOAT, SNGL, DBLE, CMPLX,
			      CHAR, ZEXT, etc. */

	/* All of these are already represented by existing WHIRL nodes.  F77
	   semantics preclude them from being passed, so external versions
	   aren't needed. */

    /* F77 '**' expressions */

	/* These functions are needed to support ** in expressions.  They are
	   not directly callable, so external versions aren't needed.  F77
	   semantic rules treat f**i specially.  Here are the details:

	   a**b   \  b
	         a \  I1,I2,I4  I8    F4   F8   FQ   C4   C8   CQ
	            +--------------------------------------------
	   I1,I2,I4 |    I4     I8    F4   F8   FQ   C4   C8   CQ
		 I8 |    I8     I8    F4   F8   FQ   C4   C8   CQ
		 F4 |   F4I4   F4I8   F4   F8   FQ   C4   C8   CQ
		 F8 |   F8I4   F8I8   F8   F8   FQ   C8   C8   CQ
		 FQ |   FQI4   FQI8   FQ   FQ   FQ   CQ   CQ   CQ
		 C4 |   C4I4   C4I8   C4   C8   CQ   C4   C8   CQ
		 C8 |   C8I4   C8I8   C8   C8   CQ   C8   C8   CQ
		 CQ |   CQI4   CQI8   CQ   CQ   CQ   CQ   CQ   CQ   */

  INTRN_I4EXPEXPR = INTRINSIC_FIRST,
  INTRN_I8EXPEXPR,
  INTRN_F4EXPEXPR,
  INTRN_F8EXPEXPR,
  INTRN_FQEXPEXPR,
  INTRN_F16EXPEXPR,
  INTRN_C4EXPEXPR,
  INTRN_C8EXPEXPR,
  INTRN_CQEXPEXPR,
  INTRN_C16EXPEXPR,
  INTRN_F4I4EXPEXPR,
  INTRN_F4I8EXPEXPR,
  INTRN_F8I4EXPEXPR,
  INTRN_F8I8EXPEXPR,
  INTRN_FQI4EXPEXPR,
  INTRN_FQI8EXPEXPR,
  INTRN_F16I4EXPEXPR,
  INTRN_F16I8EXPEXPR,
  INTRN_C4I4EXPEXPR,
  INTRN_C4I8EXPEXPR,
  INTRN_C8I4EXPEXPR,
  INTRN_C8I8EXPEXPR,
  INTRN_CQI4EXPEXPR,
  INTRN_CQI8EXPEXPR,
  INTRN_C16I4EXPEXPR,
  INTRN_C16I8EXPEXPR,

    /* F77 character relational expressions */

	/* These functions are needed to support character relational logical
	   expressions.  They are not directly callable, so external versions
	   aren't needed. */

  INTRN_CEQEXPR,
  INTRN_CNEEXPR,
  INTRN_CGEEXPR,
  INTRN_CGTEXPR,
  INTRN_CLEEXPR,
  INTRN_CLTEXPR,

    /* F77 substring 'var(e1:e2)' expressions */

	/* These functions are needed to support character substrings in
	   expressions.  They are not directly callable, so external versions
	   aren't needed.  */

  INTRN_SUBSTRINGEXPR,

    /* F77 concat '//' expressions */

	/* These functions are needed to support // in expressions.  They are
	   not directly callable, so external versions aren't needed.  */

  INTRN_CONCATEXPR,

    /* F77 character assignment statements */

	/* These functions are needed to support character assignment
	   statements.  The normal OPC_MLOAD and OPC_MSTORE don't handle the
	   blank padding F77 requires.  They are not directly callable, so
	   external versions aren't needed.  */

  INTRN_CASSIGNSTMT,


    /* F77 max/min intrinsics - MAX, AMAX, MIN, AMIN, etc. */

	/* All of these differ from the existing WHIRL nodes in that they
	   take an arbitrary number of arguments.  However; they can all be
	   transformed into a cascaded set of WHIRL OPC_MAX/MIN nodes with (if
	   needed) an OPC_CVT node on top.  F77 semantics preclude them from
	   being passed, so external versions aren't needed. */

    /* F77 abs intrinsics - ABS, etc. */

	/* All of these are already represented by existing WHIRL nodes.
	   However; F77 semantics allow them to be passed, so external versions
	   are needed. */

  INTRN_I2ABSe,
  INTRN_I4ABSe,
  INTRN_I8ABSe,
  INTRN_F4ABSe,
  INTRN_F8ABSe,
  INTRN_FQABSe,
  INTRN_F16ABSe,
  INTRN_F4C4ABS,
  INTRN_F4C4ABSe,
  INTRN_F8C8ABS,
  INTRN_F8C8ABSe,
  INTRN_FQCQABS,
  INTRN_FQCQABSe,
  INTRN_F16C16ABS,
  INTRN_F16C16ABSe,


    /* F77 mod intrinsics - MOD, etc. */

	/* The integer cases are already represented by existing WHIRL nodes.
	   F77 semantics allow them to be passed, so external versions are
	   needed. */

  INTRN_I2MODe,
  INTRN_I4MODe,
  INTRN_I8MODe,
  INTRN_F4MOD,
  INTRN_F4MODe,
  INTRN_F8MOD,
  INTRN_F8MODe,
  INTRN_FQMOD,
  INTRN_FQMODe,
  INTRN_F16MOD,
  INTRN_F16MODe,

    /* F77 sqrt intrinsics - SQRT, etc. */

	/* All of these are already represented by existing WHIRL nodes
	   However; F77 semantics allow them to be passed, so external versions
	   are needed. */

  INTRN_F4SQRTe,
  INTRN_F8SQRTe,
  INTRN_FQSQRTe,
  INTRN_F16SQRTe,
  INTRN_C4SQRTe,
  INTRN_C8SQRTe,
  INTRN_CQSQRTe,
  INTRN_C16SQRTe,

    /* F77 misc. math intrinsics - CONJG, DIM, PROD, SIGN, etc. */

	/* None of these have corresponding, single WHIRL nodes.  F77 semantics
	   allow them to be passed, so external versions are needed. */

  INTRN_C4CONJG,
  INTRN_C4CONJGe,
  INTRN_C8CONJG,
  INTRN_C8CONJGe,
  INTRN_CQCONJG,
  INTRN_CQCONJGe,
  INTRN_C16CONJG,
  INTRN_C16CONJGe,

  INTRN_I1DIM,
  INTRN_I2DIM,
  INTRN_I2DIMe,
  INTRN_I4DIM,
  INTRN_I4DIMe,
  INTRN_I8DIM,
  INTRN_I8DIMe,
  INTRN_F4DIM,
  INTRN_F4DIMe,
  INTRN_F8DIM,
  INTRN_F8DIMe,
  INTRN_FQDIM,
  INTRN_FQDIMe,
  INTRN_F16DIM,
  INTRN_F16DIMe,

  INTRN_F8F4PROD,
  INTRN_F8F4PRODe,
  INTRN_FQF8PROD,
  INTRN_FQF8PRODe,
  INTRN_F16F8PROD,
  INTRN_F16F8PRODe,

  INTRN_I1SIGN,
  INTRN_I2SIGN,
  INTRN_I2SIGNe,
  INTRN_I4SIGN,
  INTRN_I4SIGNe,
  INTRN_I8SIGN,
  INTRN_I8SIGNe,
  INTRN_F4SIGN,
  INTRN_F4SIGNe,
  INTRN_F8SIGN,
  INTRN_F8SIGNe,
  INTRN_FQSIGN,
  INTRN_FQSIGNe,
  INTRN_F16SIGN,
  INTRN_F16SIGNe,

    /* F77 misc. math intrinsics - AIMAG, AINT, ANINT, IDINT, NINT, etc. */

	/* Some of these are already represented by existing WHIRL nodes.
	   F77 semantics allow these to be passed, so external versions are
	   needed.
	   Note that some of these (e.g., IDINT) are explicitly disallowed
	   as actual arguments by the F77 standard, but are allowed
	   historically on our platform (as well as on others, e.g., DEC VAX).
	   */

  INTRN_F4IMAGe,
  INTRN_F8IMAGe,
  INTRN_FQIMAGe,
  INTRN_F16IMAGe,

  INTRN_F4AINT,
  INTRN_F4AINTe,
  INTRN_F8AINT,
  INTRN_F8AINTe,
  INTRN_FQAINT,
  INTRN_FQAINTe,
  INTRN_F16AINT,
  INTRN_F16AINTe,

  INTRN_I2F4INTe,
  INTRN_I4F4INTe,
  INTRN_I8F4INTe,

  INTRN_I2F8IDINTe,
  INTRN_I4F8IDINTe,
  INTRN_I8F8IDINTe,

  INTRN_I2FQIQINTe,
  INTRN_I4FQIQINTe,
  INTRN_I8FQIQINTe,

  INTRN_I2F16IQINTe,
  INTRN_I4F16IQINTe,
  INTRN_I8F16IQINTe,

  INTRN_I2F4NINT,
  INTRN_I2F4NINTe,
  INTRN_I4F4NINT,
  INTRN_I4F4NINTe,
  INTRN_I8F4NINT,
  INTRN_I8F4NINTe,

  INTRN_I2F8IDNINT,
  INTRN_I2F8IDNINTe,
  INTRN_I4F8IDNINT,
  INTRN_I4F8IDNINTe,
  INTRN_I8F8IDNINT,
  INTRN_I8F8IDNINTe,

  INTRN_I2FQIQNINT,
  INTRN_I2FQIQNINTe,
  INTRN_I4FQIQNINT,
  INTRN_I4FQIQNINTe,
  INTRN_I8FQIQNINT,
  INTRN_I8FQIQNINTe,

  INTRN_I2F16IQNINT,
  INTRN_I2F16IQNINTe,
  INTRN_I4F16IQNINT,
  INTRN_I4F16IQNINTe,
  INTRN_I8F16IQNINT,
  INTRN_I8F16IQNINTe,

  INTRN_F4ANINT,
  INTRN_F4ANINTe,
  INTRN_F8ANINT,
  INTRN_F8ANINTe,
  INTRN_FQANINT,
  INTRN_FQANINTe,
  INTRN_F16ANINT,
  INTRN_F16ANINTe,

    /* F77 bit intrisics - BNOT, BAND, BIOR, BXOR, BITS, BSET, BCLR, BTEST,
			   MVBITS, etc. */

	/* Some of these are already represented by existing WHIRL nodes.
	   F77 semantics allow them to be passed, so external versions are
	   needed. */

  INTRN_I2BNOTe,
  INTRN_I4BNOTe,
  INTRN_I8BNOTe,

  INTRN_I2BANDe,
  INTRN_I4BANDe,
  INTRN_I8BANDe,

  INTRN_I2BIORe,
  INTRN_I4BIORe,
  INTRN_I8BIORe,

  INTRN_I2BXORe,
  INTRN_I4BXORe,
  INTRN_I8BXORe,

  INTRN_I1BITS,
  INTRN_I2BITS,
  INTRN_I2BITSe,
  INTRN_I4BITS,
  INTRN_I4BITSe,
  INTRN_I8BITS,
  INTRN_I8BITSe,

  INTRN_I1BSET,
  INTRN_I2BSET,
  INTRN_I2BSETe,
  INTRN_I4BSET,
  INTRN_I4BSETe,
  INTRN_I8BSET,
  INTRN_I8BSETe,

  INTRN_I1BCLR,
  INTRN_I2BCLR,
  INTRN_I2BCLRe,
  INTRN_I4BCLR,
  INTRN_I4BCLRe,
  INTRN_I8BCLR,
  INTRN_I8BCLRe,

  INTRN_I1BTEST,
  INTRN_I2BTEST,
  INTRN_I2BTESTe,
  INTRN_I4BTEST,
  INTRN_I4BTESTe,
  INTRN_I8BTEST,
  INTRN_I8BTESTe,

  INTRN_I1MVBITS,
  INTRN_I2MVBITS,
  INTRN_I4MVBITS,
  INTRN_I8MVBITS,

    /* Fortran shift intrinsics - LSHIFT, RSHIFT, SHIFT, SHIFTC, etc. */

	/* Some of these are already represented by existing WHIRL nodes
	   F77 semantics allow them to be passed, so external versions are
	   needed. */

  INTRN_I1SHL,
  INTRN_I2SHL,

  INTRN_I1SHR,
  INTRN_I2SHR,

  INTRN_I1SHFT,
  INTRN_I2SHFT,
  INTRN_I2SHFTe,
  INTRN_I4SHFT,
  INTRN_I4SHFTe,
  INTRN_I8SHFT,
  INTRN_I8SHFTe,

  INTRN_I1SHFTC,
  INTRN_I2SHFTC,
  INTRN_I2SHFTCe,
  INTRN_I4SHFTC,
  INTRN_I4SHFTCe,
  INTRN_I8SHFTC,
  INTRN_I8SHFTCe,

    /* F77 character intrinsics - LEN, INDEX, LGE, LGT, LLE, LLT, etc. */

	/* None of these have corresponding, single WHIRL nodes.  F77 semantics
	   allow them to be passed, so external versions are needed. */

  INTRN_I4CLEN,
  INTRN_I4CLENe,

  INTRN_I4CINDEX,
  INTRN_I4CINDEXe,

  INTRN_CLGE,
  INTRN_CLGEe,

  INTRN_CLGT,
  INTRN_CLGTe,

  INTRN_CLLE,
  INTRN_CLLEe,

  INTRN_CLLT,
  INTRN_CLLTe,

    /* F77 transcendental intrinsics - EXP, LOG, LOG10, etc. */

	/* None of these have corresponding, single WHIRL nodes.  F77 semantics
	   allow them to be passed, so external versions are needed. */

  INTRN_F4EXP,
  INTRN_F4EXPe,
  INTRN_F8EXP,
  INTRN_F8EXPe,
  INTRN_FQEXP,
  INTRN_FQEXPe,
  INTRN_F16EXP,
  INTRN_F16EXPe,
  INTRN_C4EXP,
  INTRN_C4EXPe,
  INTRN_C8EXP,
  INTRN_C8EXPe,
  INTRN_CQEXP,
  INTRN_CQEXPe,
  INTRN_C16EXP,
  INTRN_C16EXPe,

  INTRN_F4LOG,
  INTRN_F4LOGe,
  INTRN_F8LOG,
  INTRN_F8LOGe,
  INTRN_FQLOG,
  INTRN_FQLOGe,
  INTRN_F16LOG,
  INTRN_F16LOGe,
  INTRN_C4LOG,
  INTRN_C4LOGe,
  INTRN_C8LOG,
  INTRN_C8LOGe,
  INTRN_CQLOG,
  INTRN_CQLOGe,
  INTRN_C16LOG,
  INTRN_C16LOGe,

  INTRN_F4LOG10,
  INTRN_F4LOG10e,
  INTRN_F8LOG10,
  INTRN_F8LOG10e,
  INTRN_FQLOG10,
  INTRN_FQLOG10e,
  INTRN_F16LOG10,
  INTRN_F16LOG10e,

    /* F77 trigonometic intrinsics - COS, SIN, CIS, TAN, COSD, SIND, TAND,
				     COSH, SINH, TANH, ACOS, ASIN, ATAN, ACOSD,
				     ASIND, ATAND, ATAN2, ATAN2D, etc. */

	/* None of these have corresponding, single WHIRL nodes.  F77 semantics
	   allow them to be passed, so external versions are needed. */

  INTRN_F4COS,
  INTRN_F4COSe,
  INTRN_F8COS,
  INTRN_F8COSe,
  INTRN_FQCOS,
  INTRN_FQCOSe,
  INTRN_F16COS,
  INTRN_F16COSe,
  INTRN_C4COS,
  INTRN_C4COSe,
  INTRN_C8COS,
  INTRN_C8COSe,
  INTRN_CQCOS,
  INTRN_CQCOSe,
  INTRN_C16COS,
  INTRN_C16COSe,

  INTRN_F4SIN,
  INTRN_F4SINe,
  INTRN_F8SIN,
  INTRN_F8SINe,
  INTRN_FQSIN,
  INTRN_FQSINe,
  INTRN_F16SIN,
  INTRN_F16SINe,
  INTRN_C4SIN,
  INTRN_C4SINe,
  INTRN_C8SIN,
  INTRN_C8SINe,
  INTRN_CQSIN,
  INTRN_CQSINe,
  INTRN_C16SIN,
  INTRN_C16SINe,

  INTRN_F4CIS,
  INTRN_F4CISe,
  INTRN_F8CIS,
  INTRN_F8CISe,
  INTRN_FQCIS,
  INTRN_FQCISe,
  INTRN_F16CIS,
  INTRN_F16CISe,

  INTRN_F4TAN,
  INTRN_F4TANe,
  INTRN_F8TAN,
  INTRN_F8TANe,
  INTRN_FQTAN,
  INTRN_FQTANe,
  INTRN_F16TAN,
  INTRN_F16TANe,

  INTRN_F4COSD,
  INTRN_F4COSDe,
  INTRN_F8COSD,
  INTRN_F8COSDe,
  INTRN_FQCOSD,
  INTRN_FQCOSDe,
  INTRN_F16COSD,
  INTRN_F16COSDe,

  INTRN_F4SIND,
  INTRN_F4SINDe,
  INTRN_F8SIND,
  INTRN_F8SINDe,
  INTRN_FQSIND,
  INTRN_FQSINDe,
  INTRN_F16SIND,
  INTRN_F16SINDe,

  INTRN_F4TAND,
  INTRN_F4TANDe,
  INTRN_F8TAND,
  INTRN_F8TANDe,
  INTRN_FQTAND,
  INTRN_FQTANDe,
  INTRN_F16TAND,
  INTRN_F16TANDe,

  INTRN_F4COSH,
  INTRN_F4COSHe,
  INTRN_F8COSH,
  INTRN_F8COSHe,
  INTRN_FQCOSH,
  INTRN_FQCOSHe,
  INTRN_F16COSH,
  INTRN_F16COSHe,

  INTRN_F4SINH,
  INTRN_F4SINHe,
  INTRN_F8SINH,
  INTRN_F8SINHe,
  INTRN_FQSINH,
  INTRN_FQSINHe,
  INTRN_F16SINH,
  INTRN_F16SINHe,

  INTRN_F4TANH,
  INTRN_F4TANHe,
  INTRN_F8TANH,
  INTRN_F8TANHe,
  INTRN_FQTANH,
  INTRN_FQTANHe,
  INTRN_F16TANH,
  INTRN_F16TANHe,

  INTRN_F4ACOS,
  INTRN_F4ACOSe,
  INTRN_F8ACOS,
  INTRN_F8ACOSe,
  INTRN_FQACOS,
  INTRN_FQACOSe,
  INTRN_F16ACOS,
  INTRN_F16ACOSe,

  INTRN_F4ASIN,
  INTRN_F4ASINe,
  INTRN_F8ASIN,
  INTRN_F8ASINe,
  INTRN_FQASIN,
  INTRN_FQASINe,
  INTRN_F16ASIN,
  INTRN_F16ASINe,

  INTRN_F4ATAN,
  INTRN_F4ATANe,
  INTRN_F8ATAN,
  INTRN_F8ATANe,
  INTRN_FQATAN,
  INTRN_FQATANe,
  INTRN_F16ATAN,
  INTRN_F16ATANe,

  INTRN_F4ACOSD,
  INTRN_F4ACOSDe,
  INTRN_F8ACOSD,
  INTRN_F8ACOSDe,
  INTRN_FQACOSD,
  INTRN_FQACOSDe,
  INTRN_F16ACOSD,
  INTRN_F16ACOSDe,

  INTRN_F4ASIND,
  INTRN_F4ASINDe,
  INTRN_F8ASIND,
  INTRN_F8ASINDe,
  INTRN_FQASIND,
  INTRN_FQASINDe,
  INTRN_F16ASIND,
  INTRN_F16ASINDe,

  INTRN_F4ATAND,
  INTRN_F4ATANDe,
  INTRN_F8ATAND,
  INTRN_F8ATANDe,
  INTRN_FQATAND,
  INTRN_FQATANDe,
  INTRN_F16ATAND,
  INTRN_F16ATANDe,

  INTRN_F4ATAN2,
  INTRN_F4ATAN2e,
  INTRN_F8ATAN2,
  INTRN_F8ATAN2e,
  INTRN_FQATAN2,
  INTRN_FQATAN2e,
  INTRN_F16ATAN2,
  INTRN_F16ATAN2e,

  INTRN_F4ATAN2D,
  INTRN_F4ATAN2De,
  INTRN_F8ATAN2D,
  INTRN_F8ATAN2De,
  INTRN_FQATAN2D,
  INTRN_FQATAN2De,
  INTRN_F16ATAN2D,
  INTRN_F16ATAN2De,

    /* F77 sizeof intrinsic - SIZEOF, etc. */

	/* All of these will have been converted to either constant nodes or
	   the LEN intrinsic. */

    /* F77 addressing intrinsics - %VAL, %REF, %LOC, etc. */

	/* All of these will have been converted to corresponding WHIRL
	   nodes. */

    /* C misc. intrinsics - currently, just alloca(). */

  INTRN_U4I4ALLOCA,
  INTRN_U8I8ALLOCA,

    /* F77 misc. intrinsics - MALLOC, FREE, DATE, ERRSNS, EXIT, TIME,
			      SECNDS, etc. */

	/* None of these have corresponding, single WHIRL nodes.  F77 semantics
	   do not allow them to be passed, so external versions aren't
	   needed. */

  INTRN_U4I4MALLOC,
  INTRN_U8I8MALLOC,

  INTRN_U4FREE,
  INTRN_U8FREE,

  INTRN_MDATE,
  INTRN_I1DATE,
  INTRN_I2DATE,
  INTRN_I4DATE,
  INTRN_I8DATE,

  INTRN_I1ERRSNS,
  INTRN_I2ERRSNS,
  INTRN_I4ERRSNS,
  INTRN_I8ERRSNS,

  INTRN_VEXIT,
  INTRN_I1EXIT,
  INTRN_I2EXIT,
  INTRN_I4EXIT,
  INTRN_I8EXIT,

  INTRN_TIME,

  INTRN_F4SECNDS,
  INTRN_F8SECNDS,

    /* Fortran pause/stop statement - PAUSE, STOP, etc. */

	/* These are made intrinsics so that the optimizer can make use of
	   their special properties. */

  INTRN_PAUSE,
  INTRN_STOP,

    /* Fortran 90 intrinsics */

  INTRN_F4I4RAN,
  INTRN_F4I8RAN,
  INTRN_F8I4RAN,
  INTRN_F8I8RAN,
  INTRN_FQI4RAN,
  INTRN_FQI8RAN,
  INTRN_F16I4RAN,
  INTRN_F16I8RAN,

    /* C intrinsics */

    /* C++ intrinsics */

    /* LNO generated intrinsics */

	/* These additional intrinsics are potentially generated by LNO. */

  INTRN_I4DIVFLOOR,
  INTRN_I8DIVFLOOR,
  INTRN_U4DIVFLOOR,
  INTRN_U8DIVFLOOR,

  INTRN_I4DIVCEIL,
  INTRN_I8DIVCEIL,
  INTRN_U4DIVCEIL,
  INTRN_U8DIVCEIL,

  INTRN_I4MODFLOOR,
  INTRN_I8MODFLOOR,
  INTRN_U4MODFLOOR,
  INTRN_U8MODFLOOR,

  INTRN_I4MODCEIL,
  INTRN_I8MODCEIL,
  INTRN_U4MODCEIL,
  INTRN_U8MODCEIL,

    /* Internal to alloca */
  INTRN_U4I4SETSTACKPOINTER,
  INTRN_U8I8SETSTACKPOINTER,
  INTRN_U4READSTACKPOINTER,
  INTRN_U8READSTACKPOINTER,

    /* OS kernel intrinsics */

  INTRN_ADD_AND_FETCH_I4,
  INTRN_SUB_AND_FETCH_I4,
  INTRN_OR_AND_FETCH_I4,
  INTRN_XOR_AND_FETCH_I4,
  INTRN_AND_AND_FETCH_I4,
  INTRN_NAND_AND_FETCH_I4,

  INTRN_FETCH_AND_ADD_I4,
  INTRN_FETCH_AND_SUB_I4,
  INTRN_FETCH_AND_OR_I4,
  INTRN_FETCH_AND_XOR_I4,
  INTRN_FETCH_AND_AND_I4,
  INTRN_FETCH_AND_NAND_I4,

  INTRN_ADD_AND_FETCH_I8,
  INTRN_SUB_AND_FETCH_I8,
  INTRN_OR_AND_FETCH_I8,
  INTRN_XOR_AND_FETCH_I8,
  INTRN_AND_AND_FETCH_I8,
  INTRN_NAND_AND_FETCH_I8,

  INTRN_FETCH_AND_ADD_I8,
  INTRN_FETCH_AND_SUB_I8,
  INTRN_FETCH_AND_OR_I8,
  INTRN_FETCH_AND_XOR_I8,
  INTRN_FETCH_AND_AND_I8,
  INTRN_FETCH_AND_NAND_I8,

  INTRN_LOCK_TEST_AND_SET_I4,
  INTRN_LOCK_TEST_AND_SET_I8,

  INTRN_LOCK_RELEASE_I4,
  INTRN_LOCK_RELEASE_I8,

  INTRN_VAL_COMPARE_AND_SWAP_I4,
  INTRN_VAL_COMPARE_AND_SWAP_I8,
  INTRN_BOOL_COMPARE_AND_SWAP_I4,
  INTRN_BOOL_COMPARE_AND_SWAP_I8,

  INTRN_SYNCHRONIZE,

  INTRN_RETURN_ADDRESS,

    /* F77 (internal only) I/O intrinsics */

  INTRN_U4I1ADRTMP,
  INTRN_U4I2ADRTMP,
  INTRN_U4I4ADRTMP,
  INTRN_U4I8ADRTMP,
  INTRN_U4F4ADRTMP,
  INTRN_U4F8ADRTMP,
  INTRN_U4FQADRTMP,
  INTRN_U4F16ADRTMP,
  INTRN_U4C4ADRTMP,
  INTRN_U4C8ADRTMP,
  INTRN_U4CQADRTMP,
  INTRN_U4C16ADRTMP,
  INTRN_U4VADRTMP,
  INTRN_U8I1ADRTMP,
  INTRN_U8I2ADRTMP,
  INTRN_U8I4ADRTMP,
  INTRN_U8I8ADRTMP,
  INTRN_U8F4ADRTMP,
  INTRN_U8F8ADRTMP,
  INTRN_U8FQADRTMP,
  INTRN_U8F16ADRTMP,
  INTRN_U8C4ADRTMP,
  INTRN_U8C8ADRTMP,
  INTRN_U8CQADRTMP,
  INTRN_U8C16ADRTMP,
  INTRN_U8VADRTMP,

  INTRN_I4VALTMP,
  INTRN_I8VALTMP,
  INTRN_U4VALTMP,
  INTRN_U8VALTMP,
  INTRN_F4VALTMP,
  INTRN_F8VALTMP,
  INTRN_FQVALTMP,
  INTRN_F16VALTMP,
  INTRN_C4VALTMP,
  INTRN_C8VALTMP,
  INTRN_CQVALTMP,
  INTRN_C16VALTMP,

    /* C intrinsics */

  INTRN_BCOPY,
  INTRN_BCMP,
  INTRN_BZERO,

  INTRN_MEMCCPY,
  INTRN_MEMCHR,
  INTRN_MEMCMP,
  INTRN_MEMCPY,
  INTRN_MEMMOVE,
  INTRN_MEMSET,

  INTRN_STRCMP,
  INTRN_STRNCMP,
  INTRN_STRCPY,
  INTRN_STRNCPY,
  INTRN_STRLEN,
  INTRN_STRCHR,
  INTRN_STRCAT,

  INTRN_PRINTF,
  INTRN_FPRINTF,
  INTRN_SPRINTF,
  INTRN_PRINTW,
  INTRN_SCANF,
  INTRN_FSCANF,
  INTRN_SSCANF,
  INTRN_FPUTC,
  INTRN_FPUTS,
  INTRN_FGETC,
  INTRN_FGETS,

  INTRN_F4VACOS,
  INTRN_F8VACOS,
  INTRN_F4VASIN,
  INTRN_F8VASIN,
  INTRN_F4VATAN,
  INTRN_F8VATAN,
  INTRN_F4VCOS,
  INTRN_F8VCOS,
  INTRN_F4VEXP,
  INTRN_F8VEXP,
  INTRN_F4VLOG,
  INTRN_F8VLOG,
  INTRN_F4VSIN,
  INTRN_F8VSIN,
  INTRN_F4VSQRT,
  INTRN_F8VSQRT,
  INTRN_F4VTAN,
  INTRN_F8VTAN,

  INTRN_NARY_ADD,
  INTRN_NARY_MPY,

    /* F77 intrinsics needed for -trapuv - MALLOC */

  INTRN_U4I4TRAPUV_MALLOC,
  INTRN_U8I8TRAPUV_MALLOC,

    /* F77 intrinsics needed for -check_bounds */

  INTRN_F77_BOUNDS_ERR,

    /* F77 intrinsics needed for DSM */

  INTRN_DSM_NUMTHREADS,
  INTRN_DSM_CHUNKSIZE,
  INTRN_DSM_THIS_CHUNKSIZE,
  INTRN_DSM_REM_CHUNKSIZE,
  INTRN_DSM_NUMCHUNKS,
  INTRN_DSM_THIS_THREADNUM,
  INTRN_DSM_DISTRIBUTION_BLOCK,
  INTRN_DSM_DISTRIBUTION_STAR,
  INTRN_DSM_ISRESHAPED,
  INTRN_DSM_ISDISTRIBUTED,
  INTRN_DSM_THIS_STARTINDEX,
  INTRN_DSM_DISTRIBUTION_CYCLIC,

    /* More OS kernel intrinsics -- added here so we wouldn't have to
       make an incompatible Whirl change */

  INTRN_MPY_AND_FETCH_I4,
  INTRN_MIN_AND_FETCH_I4,
  INTRN_MAX_AND_FETCH_I4,
  INTRN_FETCH_AND_MPY_I4,
  INTRN_FETCH_AND_MIN_I4,
  INTRN_FETCH_AND_MAX_I4,
  INTRN_MPY_AND_FETCH_I8,
  INTRN_MIN_AND_FETCH_I8,
  INTRN_MAX_AND_FETCH_I8,
  INTRN_FETCH_AND_MPY_I8,
  INTRN_FETCH_AND_MIN_I8,
  INTRN_FETCH_AND_MAX_I8,

  INTRN_ADD_AND_FETCH_F4,
  INTRN_SUB_AND_FETCH_F4,
  INTRN_OR_AND_FETCH_F4,
  INTRN_XOR_AND_FETCH_F4,
  INTRN_AND_AND_FETCH_F4,
  INTRN_NAND_AND_FETCH_F4,
  INTRN_MPY_AND_FETCH_F4,
  INTRN_MIN_AND_FETCH_F4,
  INTRN_MAX_AND_FETCH_F4,

  INTRN_FETCH_AND_ADD_F4,
  INTRN_FETCH_AND_SUB_F4,
  INTRN_FETCH_AND_OR_F4,
  INTRN_FETCH_AND_XOR_F4,
  INTRN_FETCH_AND_AND_F4,
  INTRN_FETCH_AND_NAND_F4,
  INTRN_FETCH_AND_MPY_F4,
  INTRN_FETCH_AND_MIN_F4,
  INTRN_FETCH_AND_MAX_F4,

  INTRN_ADD_AND_FETCH_F8,
  INTRN_SUB_AND_FETCH_F8,
  INTRN_OR_AND_FETCH_F8,
  INTRN_XOR_AND_FETCH_F8,
  INTRN_AND_AND_FETCH_F8,
  INTRN_NAND_AND_FETCH_F8,
  INTRN_MPY_AND_FETCH_F8,
  INTRN_MIN_AND_FETCH_F8,
  INTRN_MAX_AND_FETCH_F8,

  INTRN_FETCH_AND_ADD_F8,
  INTRN_FETCH_AND_SUB_F8,
  INTRN_FETCH_AND_OR_F8,
  INTRN_FETCH_AND_XOR_F8,
  INTRN_FETCH_AND_AND_F8,
  INTRN_FETCH_AND_NAND_F8,
  INTRN_FETCH_AND_MPY_F8,
  INTRN_FETCH_AND_MIN_F8,
  INTRN_FETCH_AND_MAX_F8,

  INTRN_LOCK_ACQUIRE_I4,
  INTRN_LOCK_ACQUIRE_I8,

    /* Start of F90 specific intrinsics */

    /* The next four are for allocations produced by the F90 lowerer, and
       are internal to the lowerer */
  INTRN_F90_STACKTEMPALLOC,
  INTRN_F90_HEAPTEMPALLOC,
  INTRN_F90_STACKTEMPFREE,
  INTRN_F90_HEAPTEMPFREE,

  INTRN_FIRST_F90_INTRINSIC,

  // F90 Intrinsics
  INTRN_F4EXPONENT = INTRN_FIRST_F90_INTRINSIC,
  INTRN_F8EXPONENT,
  INTRN_FQEXPONENT,
  INTRN_F16EXPONENT,
  INTRN_F4FRACTION,
  INTRN_F8FRACTION,
  INTRN_FQFRACTION,
  INTRN_F16FRACTION,
  INTRN_F4MODULO,
  INTRN_F8MODULO,
  INTRN_FQMODULO,
  INTRN_F16MODULO,
  INTRN_F4NEAREST,
  INTRN_F8NEAREST,
  INTRN_FQNEAREST,
  INTRN_F16NEAREST,
  INTRN_F4RRSPACING,
  INTRN_F8RRSPACING,
  INTRN_FQRRSPACING,
  INTRN_F16RRSPACING,
  INTRN_F4SCALE,
  INTRN_F8SCALE,
  INTRN_FQSCALE,
  INTRN_F16SCALE,
  INTRN_F4SET_EXPONENT,
  INTRN_F8SET_EXPONENT,
  INTRN_FQSET_EXPONENT,
  INTRN_F16SET_EXPONENT,
  INTRN_F4SPACING,
  INTRN_F8SPACING,
  INTRN_FQSPACING,
  INTRN_F16SPACING,
  INTRN_F4NEXTAFTER,
  INTRN_F8NEXTAFTER,
  INTRN_FQNEXTAFTER,
  INTRN_F16NEXTAFTER,
  INTRN_F4ISNAN,
  INTRN_F8ISNAN,
  INTRN_FQISNAN,
  INTRN_F16ISNAN,
  INTRN_F4SCALB,
  INTRN_F8SCALB,
  INTRN_FQSCALB,
  INTRN_F16SCALB,
  INTRN_F4IEEE_REMAINDER,
  INTRN_F8IEEE_REMAINDER,
  INTRN_FQIEEE_REMAINDER,
  INTRN_F16IEEE_REMAINDER,
  INTRN_F4LOGB,
  INTRN_F8LOGB,
  INTRN_FQLOGB,
  INTRN_F16LOGB,
  INTRN_F4ILOGB,
  INTRN_F8ILOGB,
  INTRN_FQILOGB,
  INTRN_F16ILOGB,
  INTRN_F4FPCLASS,
  INTRN_F8FPCLASS,
  INTRN_FQFPCLASS,
  INTRN_F16FPCLASS,
  INTRN_F4FINITE,
  INTRN_F8FINITE,
  INTRN_FQFINITE,
  INTRN_F16FINITE,
  INTRN_F4UNORDERED,
  INTRN_F8UNORDERED,
  INTRN_FQUNORDERED,
  INTRN_F16UNORDERED,
  INTRN_I1POPCNT,
  INTRN_I2POPCNT,
  INTRN_I4POPCNT,
  INTRN_I8POPCNT,
  INTRN_I1LEADZ,
  INTRN_I2LEADZ,
  INTRN_I4LEADZ,
  INTRN_I8LEADZ,
  INTRN_LENTRIM,
  INTRN_F90INDEX,
  INTRN_SCAN,
  INTRN_VERIFY,
  INTRN_ADJUSTL,
  INTRN_ADJUSTR,
  INTRN_GET_IEEE_EXCEPTIONS,
  INTRN_GET_IEEE_INTERRUPTS,
  INTRN_GET_IEEE_ROUNDING_MODE,
  INTRN_GET_IEEE_STATUS,
  INTRN_SET_IEEE_EXCEPTIONS,
  INTRN_SET_IEEE_EXCEPTION,
  INTRN_SET_IEEE_INTERRUPTS,
  INTRN_SET_IEEE_ROUNDING_MODE,
  INTRN_SET_IEEE_STATUS,
  INTRN_ENABLE_IEEE_INTERRUPT,
  INTRN_DISABLE_IEEE_INTERRUPT,
  INTRN_TEST_IEEE_EXCEPTION,
  INTRN_TEST_IEEE_INTERRUPT,
  INTRN_MATMUL,
  INTRN_SPREAD,
  INTRN_RESHAPE,
  INTRN_TRANSPOSE,
  INTRN_ALL,
  INTRN_ANY,
  INTRN_COUNT,
  INTRN_PRODUCT,
  INTRN_SUM,
  INTRN_EOSHIFT,
  INTRN_MAXVAL,
  INTRN_MINVAL,
  INTRN_MAXLOC,
  INTRN_MINLOC,
  INTRN_CSHIFT,
  INTRN_DOT_PRODUCT,
  INTRN_PACK,
  INTRN_UNPACK,
  INTRN_MERGE,
  INTRN_CHAR,
  INTRN_LAST_F90_INTRINSIC = INTRN_CHAR,


  INTRN_MP_IN_PARALLEL_REGION,
  INTRN_RT_ERR,
  INTRN_OMP_DO_WORKSHARING,
  INTRN_OMP_TEST_LOCK,
  INTRN_OMP_GET_NUM_THREADS,
  INTRN_OMP_GET_MAX_THREADS,
  INTRN_OMP_GET_THREAD_NUM,
  INTRN_OMP_GET_NUM_PROCS,
  INTRN_OMP_IN_PARALLEL,
  INTRN_OMP_GET_DYNAMIC,
  INTRN_OMP_GET_NESTED,

/* Hand-added F90 intrinsics */

  INTRN_I1IEEE_INT,
  INTRN_I2IEEE_INT,
  INTRN_I4IEEE_INT,
  INTRN_I8IEEE_INT,
  INTRN_F4IEEE_INT,
  INTRN_F8IEEE_INT,
  INTRN_FQIEEE_INT,
  INTRN_F16IEEE_INT,
  INTRN_F90BOUNDS_CHECK,

/* Two more intrisics used only by the F90 lowerer/front-end */
  INTRN_F90_DYNAMICTEMPALLOC,
  INTRN_F90_DYNAMICTEMPFREE,

/* More checking */
  INTRN_F90CONFORM_CHECK,

    /* End of intrinsics list */
/* More C intrinsics */

  INTRN_C_F4FLOOR,
  INTRN_C_F8FLOOR,
  INTRN_C_FQFLOOR,
  INTRN_C_F16FLOOR,
  INTRN_C_F4CEIL,
  INTRN_C_F8CEIL,
  INTRN_C_FQCEIL,
  INTRN_C_F16CEIL,
  INTRN_C_F4TRUNC,
  INTRN_C_F8TRUNC,
  INTRN_C_FQTRUNC,
  INTRN_C_F16TRUNC,

/* Super Computing ABI Intrinsics */

  INTRN_I4DSHIFTL,
  INTRN_I8DSHIFTL,
  INTRN_I4DSHIFTR,
  INTRN_I8DSHIFTR,
  INTRN_I4GBIT,
  INTRN_I8GBIT,
  INTRN_I4GBITS,
  INTRN_I8GBITS,
  INTRN_I4MASK,
  INTRN_I8MASK,
  INTRN_I4MASKL,
  INTRN_I8MASKL,
  INTRN_I4MASKR,
  INTRN_I8MASKR,
  INTRN_I4PBIT,
  INTRN_I8PBIT,
  INTRN_I4PBITS,
  INTRN_I8PBITS,
  INTRN_I4POPPAR,
  INTRN_I8POPPAR,
  INTRN_I4RTC,
  INTRN_I8RTC,

  INTRN_GETF_EXP,
  INTRN_SETF_EXP,
  INTRN_GETF_SIG,
  INTRN_SETF_SIG,

  INTRN_FMERGE_NS,
  INTRN_FMERGE_S,
  INTRN_FMERGE_SE,

  /* F90 stop */
  INTRN_STOP_F90,

  /* log10 */
  INTRN_F4VLOG10,
  INTRN_F8VLOG10,

  /* gcc divide intrinsics */
  INTRN_MODSI3,
  INTRN_UMODSI3,
  INTRN_DIVSI3,
  INTRN_UDIVSI3,
  INTRN_MODDI3,
  INTRN_UMODDI3,
  INTRN_DIVDI3,
  INTRN_UDIVDI3,
  INTRN_DIVSF3,
  INTRN_DIVDF3,

  /* gcc extensions */
  INTRN_I4FFS,

  /* linux intrinsics */
  INTRN_SINCOSF,
  INTRN_SINCOS,
  INTRN_SINCOSL,

#ifndef KEY

  INTRINSIC_LAST = INTRN_SINCOSL;

#else

  /* gcc stuff */
  INTRN_U4READFRAMEPOINTER,
  INTRN_U8READFRAMEPOINTER,
  INTRN_APPLY_ARGS,
  INTRN_APPLY,
  INTRN_RETURN,

  /* x86-64 vararg support */
  INTRN_VA_START,
  INTRN_SAVE_XMMS,

  /* builtins */
  INTRN_CONSTANT_P,

  /* C99 builtins */
  INTRN_ISGREATER,
  INTRN_ISGREATEREQUAL,
  INTRN_ISLESS,
  INTRN_ISLESSEQUAL,
  INTRN_ISLESSGREATER,
  INTRN_ISORDERED,
  INTRN_ISUNORDERED,

  /* Saturation arithmetic */
  INTRN_SUBSU2,
  INTRN_SUBSV16I2,

  /* g++ 3.4 builtins */
  INTRN_POPCOUNT,
  INTRN_PARITY,
  INTRN_CLZ,
  INTRN_CTZ64,
  INTRN_CLZ32,
  INTRN_CTZ,

  /* in GNU libm */
  INTRN_F4CBRT,
  INTRN_F8CBRT,

  INTRN_BUILTIN_EH_RETURN_DATA_REGNO,

  LAST_COMMON_ID = INTRN_BUILTIN_EH_RETURN_DATA_REGNO,

#ifdef TARG_X8664

  /* Shorter vector math functions */
  INTRN_V16F4SIN,
  INTRN_V16F4EXP,
  INTRN_V16F4EXPEXPR,
  INTRN_V16F4LOG,
  INTRN_V16F4COS,
  INTRN_V16F8SIN,
  INTRN_V16F8EXP,
  INTRN_V16F8LOG,
  INTRN_V16F8COS,
  INTRN_V16F8EXPEXPR,
  INTRN_V16F8LOG10,
  INTRN_V16F8SINCOS,

  /* Intrinsic to represent a complex C8 multiply (multiply and addsub) */
  INTRN_V16C8MPY_ADDSUB,

  /* Intrinsic to represent complex C8 conjugate */
  INTRN_V16C8CONJG,

  /* GNU x8664 builtins */
  INTRN_PADDSB,
  INTRN_PADDSW,
  INTRN_PSUBSB,
  INTRN_PSUBSW,
  INTRN_PADDUSB,
  INTRN_PADDUSW,
  INTRN_PSUBUSB,
  INTRN_PSUBUSW,
  INTRN_PMULLW,
  INTRN_PMULHW,
  INTRN_PAND,
  INTRN_PCMPEQB,
  INTRN_PCMPEQW,
  INTRN_PCMPEQD,
  INTRN_PCMPGTB,
  INTRN_PCMPGTW,
  INTRN_PCMPGTD,
	INTRN_PCMPEQB128,
	INTRN_PCMPEQW128,
	INTRN_PCMPEQD128,
	INTRN_PCMPGTB128,
	INTRN_PCMPGTW128,
	INTRN_PCMPGTD128,
  INTRN_PUNPCKHBW,
  INTRN_PUNPCKHWD,
  INTRN_PUNPCKHDQ,
  INTRN_PUNPCKLBW,
  INTRN_PUNPCKLWD,
  INTRN_PUNPCKLDQ,
  INTRN_PACKSSWB,
  INTRN_PACKSSDW,
  INTRN_PACKUSWB,
  INTRN_PACKSSWB128,
	INTRN_PACKSSDW128,
	INTRN_PACKUSWB128,
	INTRN_PUNPCKHBW128,
	INTRN_PUNPCKHWD128,
	INTRN_PUNPCKHDQ128,
	INTRN_PUNPCKHQDQ,
	INTRN_PAVGB128,
	INTRN_PAVGW128,
	INTRN_PUNPCKLBW128,
	INTRN_PUNPCKLWD128,
	INTRN_PUNPCKLDQ128,
	INTRN_PUNPCKLQDQ,
  INTRN_PMULHUW,
  INTRN_PMULHUW128,
  INTRN_PAVGB,
  INTRN_PAVGW,
  INTRN_PSADBW,
  INTRN_PSADBW128,
  INTRN_PMAXUB,
  INTRN_PMAXSW,
  INTRN_PMINUB,
  INTRN_PMINSW,
  INTRN_PMAXUB128,
  INTRN_PMAXSW128,
  INTRN_PMINUB128,
  INTRN_PMINSW128,
  INTRN_PMOVMSKB,
  INTRN_PMOVMSKB128,
  INTRN_MOVNTQ,
  INTRN_SFENCE,
  INTRN_COMIEQSS,
  INTRN_ADDPS,
  INTRN_SUBPS,
  INTRN_MULPS,
  INTRN_DIVPS,
  INTRN_ADDSS,
  INTRN_SUBSS,
  INTRN_MULSS,
  INTRN_DIVSS,
  INTRN_CMPEQPS,
  INTRN_CMPEQPD,
  INTRN_CMPLTPD,
  INTRN_CMPLEPD,
  INTRN_CMPGTPD,
  INTRN_CMPGEPD,
  INTRN_CMPNEQPD,
  INTRN_CMPNLTPD,
  INTRN_CMPNLEPD,
  INTRN_CMPNGTPD,
  INTRN_CMPNGEPD,
  INTRN_CMPORDPD,
  INTRN_CMPUNORDPD,
  INTRN_CMPLTPS,
  INTRN_CMPLEPS,
  INTRN_CMPGTPS,
  INTRN_CMPGEPS,
  INTRN_CMPUNORDPS,
  INTRN_CMPNEQPS,
  INTRN_CMPNLTPS,
  INTRN_CMPNLEPS,
  INTRN_CMPNGTPS,
  INTRN_CMPNGEPS,
  INTRN_CMPORDPS,
  INTRN_CMPEQSS,
  INTRN_CMPLTSS,
  INTRN_CMPLESS,
  INTRN_CMPUNORDSS,
  INTRN_CMPNEQSS,
  INTRN_CMPNLTSS,
  INTRN_CMPNLESS,
  INTRN_CMPORDSS,
  INTRN_MAXPS,
  INTRN_MAXSS,
  INTRN_MINPS,
  INTRN_MINSS,
  INTRN_ANDPS,
  INTRN_ANDNPS,
  INTRN_ORPS,
  INTRN_XORPS,
  INTRN_MOVSS,
  INTRN_MOVHLPS,
  INTRN_MOVLHPS,
  INTRN_UNPCKHPS,
  INTRN_UNPCKLPS,
  INTRN_RCPPS,
  INTRN_RSQRTPS,
  INTRN_SQRTPS,
  INTRN_RCPSS,
  INTRN_RSQRTSS,
  INTRN_SQRTSS,
  INTRN_SHUFPS,
  INTRN_EMMS,
  INTRN_CLFLUSH,
  INTRN_PADDQ,
  INTRN_PSUBQ,
  INTRN_UNIMP_PURE,
  INTRN_UNIMP,
  INTRN_LOADAPS,
  INTRN_STOREAPS,
  INTRN_COSL,
  INTRN_SINL,
  INTRN_TAN,
  INTRN_PSLLDQ,
  INTRN_PSLLW,
  INTRN_PSLLD,
  INTRN_PSLLQ,
  INTRN_PSRLW,
  INTRN_PSRLD,
  INTRN_PSRLQ,
  INTRN_PSRAW,
  INTRN_PSRAD,
  INTRN_V16F4SINH,
  INTRN_V16F4COSH,
  INTRN_V16F8SINH,
  INTRN_V16F8COSH,
  INTRN_MOVNTDQ,
  INTRN_LOADD,
  INTRN_MOVNTPS,
  INTRN_SSE_ZERO,
  INTRN_CLRTI,
  INTRN_PSHUFD,
  INTRN_LOADSS,
  INTRN_SIGNV16F4,
  INTRN_SIGNV16F8,
  INTRN_SHUFPD,
  INTRN_XORPD,
  INTRN_ANDPD,
  INTRN_ANDNPD,
  INTRN_ORPD,
  INTRN_STORELPD,
  INTRN_STOREHPD,
  INTRN_LOADLPD,
  INTRN_LOADHPD,
  INTRN_UNPCKLPD,
  INTRN_UNPCKHPD,
  INTRN_LFENCE,
  INTRN_MFENCE,
  INTRN_PSHUFW,
  INTRN_PSRLDQ,
  INTRN_LOADDQA,
  INTRN_LOADDQU,
  INTRN_STOREDQA,
  INTRN_STOREDQU,
  INTRN_VEC_INIT_V2SI,
  INTRN_VEC_EXT_V2SI,
  INTRN_PMADDWD,
  INTRN_PSLLW_MMX,
  INTRN_PSLLD_MMX,
  INTRN_PSRLW_MMX,
  INTRN_PSRLD_MMX,
  INTRN_PSRAW_MMX,
  INTRN_PSRAD_MMX,
  INTRN_PAND_MMX,
  INTRN_PANDN_MMX,
  INTRN_POR_MMX,
  INTRN_PXOR_MMX,
  INTRN_COMILTSS,
  INTRN_COMILESS,
  INTRN_COMIGTSS,
  INTRN_COMIGESS,
  INTRN_COMINEQSS,
  INTRN_COMIEQSD,
  INTRN_COMILTSD,
  INTRN_COMILESD,
  INTRN_COMIGTSD,
  INTRN_COMIGESD,
  INTRN_COMINEQSD,
  INTRN_CVTPI2PS,
  INTRN_CVTPS2PI,
  INTRN_CVTTPS2PI,
  INTRN_CVTPI2PD,
  INTRN_CVTPD2PI,
  INTRN_CVTTPD2PI,
  INTRN_CVTSI2SS,
  INTRN_CVTSI642SS,
  INTRN_CVTSS2SI,
  INTRN_CVTSS2SI64,
  INTRN_CVTTSS2SI,
  INTRN_CVTTSS2SI64,
  INTRN_CVTSI2SD,
  INTRN_CVTSI642SD,
  INTRN_CVTSD2SI,
  INTRN_CVTSD2SI64,
  INTRN_CVTTSD2SI,
  INTRN_CVTTSD2SI64,
  INTRN_CVTDQ2PS,
  INTRN_CVTPS2DQ,
  INTRN_CVTTPS2DQ,
  INTRN_CVTDQ2PD,
  INTRN_CVTPD2DQ,
  INTRN_CVTTPD2DQ,
  INTRN_CVTPD2PS,
  INTRN_CVTPS2PD,
  INTRN_CVTSD2SS,
  INTRN_CVTSS2SD,
  INTRN_LOADUPS,
  INTRN_STOREUPS,
  INTRN_LOADUPD,
  INTRN_STOREUPD,
  INTRN_LOADHPS,
  INTRN_STOREHPS,
  INTRN_LOADLPS,
  INTRN_STORELPS,
  INTRN_MOVMSKPS,
  INTRN_MOVMSKPD,
  INTRN_MASKMOVDQU,
  INTRN_MASKMOVQ,
  INTRN_MOVNTPD,
  INTRN_MOVNTI,
  INTRN_STMXCSR,
  INTRN_LDMXCSR,
  INTRN_MOVSD,
  INTRN_PSHUFLW,
  INTRN_PSHUFHW,
  INTRN_ADDSD,
  INTRN_SUBSD,
  INTRN_MULSD,
  INTRN_DIVSD,
  INTRN_CMPEQSD,
  INTRN_CMPLTSD,
  INTRN_CMPLESD,
  INTRN_CMPUNORDSD,
  INTRN_CMPNEQSD,
  INTRN_CMPNLTSD,
  INTRN_CMPNLESD,
  INTRN_CMPORDSD,
  INTRN_MAXSD,
  INTRN_MINSD,
  INTRN_SQRTSD,
  INTRN_F8F8I4EXPEXPR,
  INTRN_F4F4I4EXPEXPR,
  INTRN_FQFQI4EXPEXPR,
  INTRN_F16F16I4EXPEXPR,
  INTRN_EXPECT,
  INTRN_MOVNTSS,
  INTRN_MOVNTSD,
  INTRN_EXTRQI,
  INTRN_EXTRQ,
  INTRN_INSERTQI,
  INTRN_INSERTQ,
  INTRN_PADDD128,
  INTRN_PADDW128,
  INTRN_PADDQ128,
	INTRN_PADDUSB128,
	INTRN_PADDUSW128,
  INTRN_PADDSB128,
  INTRN_PADDSW128,
  INTRN_ADDSUBPS,
	INTRN_LDDQU,
	INTRN_MOVSHDUP,
	INTRN_MOVSLDUP,
	INTRN_HADDPD,
	INTRN_HADDPS,
	INTRN_HSUBPD,
	INTRN_HSUBPS,
  INTRN_ADDSUBPD,
	INTRN_PBLENDW128,
	INTRN_BLENDPD,
	INTRN_BLENDPS,
	INTRN_PBLENDVB128,
	INTRN_BLENDVPD,
	INTRN_BLENDVPS,
	INTRN_ROUNDPD,
	INTRN_ROUNDSD,
	INTRN_ROUNDPS,
	INTRN_ROUNDSS,
	INTRN_PCMPEQQ,
	INTRN_PMOVSXBW128,
	INTRN_PMOVSXBD128,
	INTRN_PMOVSXBQ128,
	INTRN_PMOVSXWD128,
	INTRN_PMOVSXWQ128,
	INTRN_PMOVSXDQ128,
	INTRN_PMOVZXBW128,
	INTRN_PMOVZXBD128,
	INTRN_PMOVZXBQ128,
	INTRN_PMOVZXWD128,
	INTRN_PMOVZXWQ128,
	INTRN_PMOVZXDQ128,
	INTRN_PACKUSDW128,
	INTRN_DPPD,
	INTRN_DPPS,
	INTRN_MPSADBW128,
	INTRN_PMULDQ128,
	INTRN_PMULLD128,
	INTRN_PEXTRB,
	INTRN_PEXTRD,
	INTRN_PEXTRQ,
	INTRN_EXTRACTPS,
	INTRN_PINSRB,
	INTRN_PINSRD,
	INTRN_PINSRQ,
	INTRN_INSERTPS,
	INTRN_PMAXSB128,
	INTRN_PMAXSD128,
	INTRN_PMAXUW128,
	INTRN_PMAXUD128,
	INTRN_PMINSB128,
	INTRN_PMINSD128,
	INTRN_PMINUW128,
	INTRN_PMINUD128,
	INTRN_PHMINPOSUW128,
  INTRN_PCMPISTRI128,
  INTRN_PCMPISTRM128,
  INTRN_PCMPESTRI128,
  INTRN_PCMPESTRM128,
  INTRN_PAND128,
  INTRN_PANDN128,
	INTRN_POR128,
  INTRN_PEXTRW64,
  INTRN_PEXTRW128,
  INTRN_PINSRW64,
  INTRN_PINSRW128,
  INTRN_V16F4LOG10,

  INTRINSIC_LAST  = INTRN_V16F4LOG10

#else // Anything target-independent needs to be added on all branches of
      // this ifdef.

  INTRN_TAN,
  INTRN_F8F8I4EXPEXPR,
  INTRN_F4F4I4EXPEXPR,
  INTRN_FQFQI4EXPEXPR,
  INTRN_F16F16I4EXPEXPR,
  INTRN_EXPECT,
  INTRN_FLOOR,
  INTRN_FLOORF,

  INTRINSIC_LAST = INTRN_FLOORF

#endif // TARG_X8664
#endif // KEY

} INTRINSIC;
#endif

#ifdef TARG_ST
typedef int INTRINSIC;
#define INTRINSIC_LAST	 INTRINSIC_COUNT

  /* [TB] extension support */
#define INTRINSIC_STATIC_COUNT INTRINSIC_TARG_LAST
#define INTRINSIC_STATIC_LAST INTRINSIC_TARG_LAST
BE_EXPORTED extern INTRINSIC INTRINSIC_COUNT;
#endif


#ifdef __cplusplus
}
#endif

#endif
