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


#ifndef	w2op_INCLUDED
#define	w2op_INCLUDED

#include "topcode.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Given a WHIRL opcode, return the corresponding TOP.
 * This will only return machine ops, 
 * else TOP_UNDEFINED if not an exact correspondence.
 */
extern TOP OPCODE_To_TOP (OPCODE opcode);

/* Given a WHIRL node, return the corresponding TOP. 
 * This will only return machine ops, 
 * else TOP_UNDEFINED if not an exact correspondence.
 * (this handles more cases than OPCODE_To_TOP, cause it can look at kids).
 */
extern TOP WHIRL_To_TOP (WN *wn);

/* return whether MPY, DIV, or REM will be translated into shifts and adds */
extern BOOL Can_Do_Fast_Multiply (TYPE_ID mtype, INT64 val);
extern BOOL Can_Do_Fast_Divide (TYPE_ID mtype, INT64 val);
extern BOOL Can_Do_Fast_Remainder (TYPE_ID mtype, INT64 val);

/* When trying to convert a multiply or divide operation into a series
 * of shifts/adds/subtracts, there is some limit (cycles? ops?) at
 * which the conversion is not profitable.  Return that limit.
 */
extern INT Multiply_Limit ( BOOL is_64bit, INT64 val);
extern INT Divide_Limit ( BOOL is_64bit);

/* Return whether or not the immediate specified by <val> would be a valid
 * operand of the machine instruction generated by the whirl operator
 * <opr> with the immediate as the <whichkid> whirl operand.
 * The datatype of the operator is specified by <dtype> and if the
 * operation is an STID, <stid_st> specifies the symbol being stored to.
 */
extern BOOL Can_Be_Immediate(OPERATOR opr,
			     INT64 val,
			     TYPE_ID dtype,
			     INT whichkid,
			     ST *stid_st);

/* determine speculative execution taking into account eagerness level
 *
 */
extern BOOL TOP_Can_Be_Speculative (TOP opcode);
extern BOOL WN_Can_Be_Speculative (WN *wn, struct ALIAS_MANAGER *alias);
extern BOOL WN_Expr_Can_Be_Speculative (WN *wn, struct ALIAS_MANAGER *alias);
extern BOOL OPCODE_Can_Be_Speculative(OPCODE opcode);

#ifdef __cplusplus
}
#endif
#endif /* w2op_INCLUDED */
