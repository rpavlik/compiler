/*
 * Copyright 2009 SiCortex, Inc.
 */

/*
 * Copyright (C) 2007, 2008 Pathscale, LLC.  All Rights Reserved.
 */

/*
 * Copyright (C) 2006, 2007. QLogic Corporation. All Rights Reserved.
 */

/* 
   Copyright 2003, 2004, 2005, 2006 PathScale, Inc.  All Rights Reserved.
   File modified October 9, 2003 by PathScale, Inc. to update Open64 C/C++ 
   front-ends to GNU 3.3.1 release.
 */

/* 
   Copyright (C) 2002 Tensilica, Inc.  All Rights Reserved.
   Revised to support Tensilica processors and to improve overall performance
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


/* translate gnu decl trees to symtab references */

extern "C"{
#include "gspin-wgen-interface.h"
}
#include <limits.h>
#include "defs.h"
#include "errors.h"

#include "symtab.h"
#include "strtab.h"
#include "wn.h"
#include "wgen_expr.h"
#include "wgen_decl.h"
#include "wgen_misc.h"
#include "wgen_dst.h"
#include "ir_reader.h"
#include "wgen_spin_symbol.h"
#include "wgen_stmt.h"
#include <map>
#include "erfe.h"
#ifdef TARG_X8664
#include <ctype.h>
#endif
//#include "tree_cmp.h"
#ifdef TARG_ST
#include "gccfe_targinfo_interface.h"
#endif

#ifdef TARG_ST
// [CG]: Helper functions for volatility attributes propagation.
// See comments in the implementation.
static int check_and_update_volatility(TY_IDX &ty_idx);
static void fixup_volatility(TY_IDX &ty_idx);
static void print_volatility(TY_IDX &ty_idx);
#endif

extern int pstatic_as_global;
extern BOOL flag_no_common;
extern gs_t decl_arguments;

extern void Push_Deferred_Function(gs_t);
extern char *WGEN_Tree_Node_Name(gs_t op);

#ifdef KEY
// =====================================================================
// bug 8346: A function's VLA argument types should only be expanded
// when necessary, to prevent size-expression-whirl from landing in an
// unintended location. If we attempt to expand such a type while
// generating a function's argument types, but are not expanding that
// specific function body, then we mark the TY as incomplete, to expand
// it later when we actually expand that function body.
// "expanding_function_definition" denotes when it is safe to process a
// function's VLA argument types.
// "processing_function_prototype" indicates when we are expanding
// function arguments (as opposed to other VLA variable occurrences).
// =====================================================================
BOOL processing_function_prototype = FALSE;
#endif

// Map duplicate gcc nodes that refer to the same function.
std::multimap<gs_t, gs_t> duplicate_of;
void
add_duplicates (gs_t newdecl, gs_t olddecl)
{
#ifdef TARG_ST
  duplicate_of.insert (std::pair<gs_t, gs_t>(newdecl, olddecl));
  duplicate_of.insert (std::pair<gs_t, gs_t>(olddecl, newdecl));
#else
	duplicate_of.insert (pair<gs_t, gs_t>(newdecl, olddecl));
	duplicate_of.insert (pair<gs_t, gs_t>(olddecl, newdecl));
#endif
}

// Remove all references to DECL from the map.
void
erase_duplicates (gs_t decl)
{
  int i, j;
  int count = duplicate_of.count (decl);

  for (i=0; i<count; i++) {
    std::multimap<gs_t, gs_t>::iterator iter = duplicate_of.find(decl);
    gs_t t = (*iter).second;

    // Erase entries with DECL as the data, i.e., <..., DECL>.
    int count2 = duplicate_of.count(t); 
    for (j=0; j<count2; j++) {
      std::multimap<gs_t, gs_t>::iterator iter2 = duplicate_of.find(t);
      gs_t t2 = (*iter2).second;
      if (t2 == decl) {
	duplicate_of.erase (iter2);
      }
    }

    // Erase entry with DECL as the key, i.e., <DECL, ...>.
    duplicate_of.erase (iter);
  }
}

static ST*
get_duplicate_st (gs_t decl)
{
  int count = duplicate_of.count (decl);

  for (int i=0; i<count; ++i) {
    std::multimap<gs_t, gs_t>::iterator iter = duplicate_of.find(decl);
    gs_t t = (*iter).second;
    // The node t could have been garbage-collected by gcc.  This is a crude
    // test to see if t is still valid.
    if (gs_tree_code(t) == GS_FUNCTION_DECL &&
	gs_decl_name(t) == gs_decl_name(decl) &&
	gs_decl_assembler_name_set_p(t) == gs_decl_assembler_name_set_p(decl) &&
	(!gs_decl_assembler_name_set_p(t) ||
	 gs_decl_assembler_name(t) == gs_decl_assembler_name(decl))) {
      // Return the ST previously allocated, if any.
      ST *st = DECL_ST(t);
      if (st != NULL)
        return st;
    }
    duplicate_of.erase (iter);
  }
  return NULL;
}

static char*
Get_Name (gs_t node)
{
	static UINT anon_num = 0;
	static char buf[64];

	if (node == NULL) {
		++anon_num;
		sprintf(buf, ".anonymous.%d", anon_num);
		return buf;
	}
	else if (gs_tree_code (node) == GS_IDENTIFIER_NODE)
		return ((char *) gs_identifier_pointer (node));
	else if (gs_tree_code (node) == GS_TYPE_DECL)
		// If type has a typedef-name, the TYPE_NAME is a TYPE_DECL.
#ifdef FE_GNU_4_2_0 // bug 14137
		if (gs_decl_name(node) == NULL) {
		  ++anon_num;
		  sprintf(buf, ".anonymous.%d", anon_num);
		  return buf;
		}
		else
#endif
		return ((char *) gs_identifier_pointer (gs_decl_name (node)));
	else
		FmtAssert(FALSE, ("Get_Name unexpected tree"));
		return NULL;
}

static void
dump_field(gs_t field)
{
#ifdef TARG_ST
  fprintf(stderr, "%s:  ", Get_Name(gs_decl_name(field)));
  fprintf(stderr, "%d\n", DECL_FIELD_ID(field));
#else
  printf("%s:  ", Get_Name(gs_decl_name(field)));
  printf("%d\n", DECL_FIELD_ID(field));
#endif
}
extern int wgen_pic;
//zwu
static ST* Trans_TLS(gs_t decl_node, ST* st)
{      
      //if(Gen_PIC_Call_Shared && begin_expand_stmt)
      if(begin_expand_stmt && ST_is_thread_local(st) && wgen_pic)
      {
      	ST* call_st = New_ST();
				ST_Init(call_st, Save_Str ("__tls_get_addr"),	CLASS_FUNC,SCLASS_EXTERN, EXPORT_PREEMPTIBLE, ST_type(call_st));
      	Set_ST_class(call_st, CLASS_FUNC);
      	//WN* call_wn = WN_Piccall(MTYPE_A8, MTYPE_A8, 1, call_st);
      	
      	TY_IDX ty_idx = ST_type(st);
      	TY_IDX idx;
			  TY &ptr_ty = New_TY (idx);
  			TY_Init (ptr_ty, Pointer_Size, KIND_POINTER, Pointer_Mtype, Save_Str ("anon_ptr."));                                                                               
  			ptr_ty.Set_pointed (ST_type(st));                                                                               
        
        WN* arg_wn = WN_Lda(Pointer_Mtype, ST_ofst(st), st);
        Set_ST_addr_passed(*st);
	      Set_ST_addr_saved(*st);
	      TY_IDX arg_ty_idx = idx;
	      Clear_TY_is_volatile(arg_ty_idx);
	      TYPE_ID arg_mtype  = TY_mtype(arg_ty_idx);
  			arg_wn = WN_CreateParm (Mtype_comparison (arg_mtype), arg_wn,  arg_ty_idx, WN_PARM_BY_VALUE);

				TYPE_ID ret_mtype = MTYPE_I8;
      	WN* call_wn = WN_Create (OPR_CALL, ret_mtype, MTYPE_V, 1);
      	WN_st_idx (call_wn) = ST_st_idx (call_st);
			  WN_kid(call_wn, 0) = arg_wn;
      	WN_Set_Call_Default_Flags(call_wn);

        WN* wn0 = WN_CreateBlock ();
        WN_INSERT_BlockLast (wn0, call_wn);
	  		WN* wn1 = WN_Ldid (ret_mtype, -1, Return_Val_Preg, ty_idx);        
        WN* wn  = WN_CreateComma (OPR_COMMA, WN_rtype (wn1), MTYPE_V, wn0, wn1);
        WGEN_Set_ST_Addr_Saved (wn);
				//lhs
				Clear_TY_is_volatile(ty_idx);
				TYPE_ID rtype = Widen_Mtype(TY_mtype(ty_idx));
				TYPE_ID desc = TY_mtype(ty_idx);

      	ST* dup_st = New_ST();
				ST_Init(dup_st, Save_Str2 ("_local_", ST_name(st)), CLASS_VAR,SCLASS_AUTO, EXPORT_LOCAL_INTERNAL, ST_type(st));
      	wn = WN_Iload(MTYPE_I8, 0, MTYPE_To_TY(MTYPE_I8), wn);
        wn = WN_Stid (desc, 0 , dup_st, ty_idx, wn, 0);
        
				WGEN_Stmt_Append(wn, Get_Srcpos());
				return dup_st;
      }
      else
      	return st;
	
}


// =================================================================
// KEY: If there is a vtable pointer, then number it as the first
// field in the record. GNU 4.x provides the same field for a vptr
// to a base class, and its inherited classes. So we consistenly
// number the vptr as field_id 1.
// =================================================================
gs_t
get_first_real_or_virtual_field (gs_t type_tree)
{
  // return vfield only if the type contains fields (bug 10787)
  // bug 11227: C_TYPE_INCOMPLETE_VARS for C is the same as TYPE_VFIELD,
  //            make sure we do not use it for C.
  if (lang_cplus && gs_type_fields(type_tree) && gs_type_vfield(type_tree))
    return gs_type_vfield(type_tree);

  return gs_type_fields(type_tree);
}

gs_t
get_virtual_field (gs_t type_tree)
{
  gs_t vfield;

  // return vfield only if the type contains fields (bug 10787)
  if (lang_cplus &&
      gs_type_fields(type_tree) &&
      (vfield = gs_type_vfield(type_tree)) != NULL)
    return vfield;
  return NULL;
}

gs_t
get_first_real_field (gs_t type_tree)
{
  gs_t field = gs_type_fields(type_tree);

  if (!field)
    return NULL;

  // If there is a pointer to the virtual function table, it is always at the
  // first field.
  if (field == gs_type_vfield(type_tree))
  {
    Is_True (lang_cplus, ("get_first_real_field: TYPE_VFIELD used for C"));
    return gs_tree_chain(field);
  }
  return field;
}

gs_t
next_real_field (gs_t type_tree, gs_t field)
{
  BOOL first_real_field = FALSE;

  if (field == gs_type_vfield(type_tree))
  {
#if 0 // bug 13102
    Is_True (lang_cplus, ("next_real_field: TYPE_VFIELD used for C"));
#endif
    first_real_field = TRUE; // return first real field
  }

  // If vptr is not in the list of fields, then return the first field
  if (first_real_field && field != gs_type_fields (type_tree))
    return gs_type_fields (type_tree);

  // Else return the next field.
  return gs_tree_chain (field);
}

static void
Do_Base_Types (gs_t type_tree)
{
  gs_t binfo = gs_type_binfo(type_tree);
  gs_t basetypes = binfo ? gs_binfo_base_binfos(binfo) : 0;
  gs_t list;
  if (basetypes) {
    for (list = basetypes; gs_code(list) != EMPTY; list = gs_operand(list, 1))
      (void) Get_TY (gs_binfo_type(gs_operand(list, 0)));
  }
}

size_t 
Roundup (size_t offset, int alignment)
{
  return (offset % alignment) ? offset + alignment - offset % alignment
			      : offset;
}

size_t
Type_Size_Without_Vbases (gs_t type_tree)
{
  gs_t field;
  gs_t last_field_decl = 0;

  for (field = get_first_real_or_virtual_field(type_tree);
       field;
       field = next_real_field (type_tree, field)) {
    if (gs_tree_code(field) == GS_FIELD_DECL)
      last_field_decl = field;
  }

  if (last_field_decl == 0)
    return 0;

  return
    gs_get_integer_value (gs_decl_field_offset(last_field_decl)) +
    gs_get_integer_value (gs_decl_field_bit_offset(last_field_decl)) / BITSPERBYTE +
    gs_get_integer_value (gs_decl_size(last_field_decl)) / BITSPERBYTE;
} 

#ifndef TARG_ST
bool
is_empty_base_class (gs_t type_tree)
{
  gs_t field = gs_type_fields(type_tree);
  return gs_tree_code(field) == GS_TYPE_DECL && gs_tree_chain(field) == 0;
}
#endif
#ifdef TARG_ST
/* ====================================================================
 *   initialize_arb ()
 *
 *   Setup ARB HANDLE's stride, lower and upper bounds.
 * ====================================================================
 */
static void
initialize_arb (
  ARB_HANDLE arb,              // ARB HANDLE to initialize
  gs_t dim_tree,               // corresponding dimension tree
  TY_IDX *ty_dim0,             // element TY_IDX
  INT64 *bitsize               // element size
)
{

  // TODO: determine how to set hosted and flow dependent ??
  //       anything else ??
  BOOL hosted = FALSE;
  BOOL flow_dependent = FALSE;
  BOOL variable_size = FALSE;

  // determine if we have variable size array dimension.
  // if it is a constant size, calculate the bitsize
  if (gs_type_size(dim_tree) == NULL) {
    // incomplete structs have 0 size
    FmtAssert(gs_tree_code(dim_tree) == GS_ARRAY_TYPE ||
	      gs_tree_code(dim_tree) == GS_UNION_TYPE ||
	      gs_tree_code(dim_tree) == GS_RECORD_TYPE,
	      ("type_size NULL for non ARRAY/RECORD"));
    *bitsize = 0;
  }
  else {
    if (gs_tree_code(gs_type_size(dim_tree)) == GS_INTEGER_CST) {
      // constant size, update the bitsize
      if (gs_tree_code(dim_tree) == GS_INTEGER_TYPE)
	*bitsize = gs_n(gs_type_precision(dim_tree));
      else
	*bitsize = gs_get_integer_value(gs_type_size(dim_tree));
    }
    else {
      // variable size:
      if (gs_tree_code(dim_tree) == GS_ARRAY_TYPE)
	DevWarn ("Encountered VLA at line %d", lineno);
      else
	Fail_FmtAssertion ("VLA at line %d not currently implemented", lineno);
      variable_size = TRUE;
    }
  }

  ARB_Init (arb, 0, 0, 0);

  // update stride - the argument is the size in bytes of the 
  // current dim. If stride isn't constant size becomes 0.

  if (gs_type_size(gs_tree_type(dim_tree))) {
    if (gs_tree_code(gs_type_size(gs_tree_type(dim_tree))) == GS_INTEGER_CST) {
      Set_ARB_const_stride (arb);
      Set_ARB_stride_val (arb, 
	gs_get_integer_value (gs_type_size(gs_tree_type(dim_tree)))/BITSPERBYTE);
    }
    /* bug 8346 */
    else if (!expanding_function_definition &&
	     processing_function_prototype) {
	Set_ARB_const_stride (arb);
	// dummy stride val 4
	Set_ARB_stride_val (arb, 4);
	Set_TY_is_incomplete (*ty_dim0);
    }
    else {
      WN *swn;
      swn = WGEN_Expand_Expr (gs_type_size(gs_tree_type(dim_tree)));
      if (WN_operator (swn) != OPR_LDID) {
        TY_IDX    ty_idx  = Get_TY (gs_tree_type(gs_type_size(gs_tree_type (dim_tree))));
        ST *st = Gen_Temp_Symbol (ty_idx, "__save_expr");
#ifdef FE_GNU_4_2_0
	WGEN_add_pragma_to_enclosing_regions (WN_PRAGMA_LOCAL, st);
#endif
        WGEN_Set_ST_Addr_Saved (swn);
        swn = WN_Stid (TY_mtype (ty_idx), 0, st, ty_idx, swn);
        WGEN_Stmt_Append (swn, Get_Srcpos());
        swn = WN_Ldid (TY_mtype (ty_idx), 0, st, ty_idx);
      }
      if (WN_opcode (swn) == OPC_U4I4CVT ||
	  WN_opcode (swn) == OPC_U5I5CVT ||
	  WN_opcode (swn) == OPC_U8I8CVT) {
	swn = WN_kid0 (swn);
      }
      FmtAssert (WN_operator (swn) == OPR_LDID,
			       ("stride operator for VLA not LDID"));
      ST *st = WN_st (swn);
      TY_IDX ty_idx = ST_type (st);
      WN *wn = WN_CreateXpragma (WN_PRAGMA_COPYIN_BOUND,
						   (ST_IDX) NULL, 1);
      WN_kid0 (wn) = WN_Ldid (TY_mtype (ty_idx), 0, st, ty_idx);
      WGEN_Stmt_Append (wn, Get_Srcpos());
      Clear_ARB_const_stride (arb);
      Set_ARB_stride_var (arb, (ST_IDX) ST_st_idx (st));
      Clear_TY_is_incomplete (*ty_dim0); /* bug 8346 */
    }
  }
  else {
    // incomplete type
    Set_ARB_stride_var (arb, 0);
  }

  // update the lower bound: it's always 0 for C arrays

  Set_ARB_const_lbnd (arb);
  Set_ARB_lbnd_val (arb, 0);

  // update the upper bound:
  //
  // If the bound isn't a constant, the FE puts it
  // into a temp, so the temp just has to be addressed.

  if (gs_type_size(dim_tree)
      && gs_type_max_value(gs_type_domain(dim_tree))) {
    if (gs_tree_code(gs_type_max_value(gs_type_domain(dim_tree))) == GS_INTEGER_CST) {
      Set_ARB_const_ubnd (arb);
      Set_ARB_ubnd_val (arb, 
          gs_get_integer_value(gs_type_max_value(gs_type_domain(dim_tree))));
    }
    /* bug 8346 */
    else if (!expanding_function_definition &&
	     processing_function_prototype) {
	Set_ARB_const_ubnd (arb);
	// dummy upper bound 8
	Set_ARB_ubnd_val (arb, 8);
	Set_TY_is_incomplete (*ty_dim0);
    }
    else {
      WN *uwn = WGEN_Expand_Expr(gs_type_max_value(gs_type_domain(dim_tree)));
      if (WN_opcode (uwn) == OPC_U4I4CVT ||
	  WN_opcode (uwn) == OPC_U5I5CVT ||
	  WN_opcode (uwn) == OPC_U8I8CVT) {
	uwn = WN_kid0 (uwn);
      }
      /* (cbr) support for vla */
      ST *st;
      TY_IDX ty_idx;
      if (WN_operator (uwn) != OPR_LDID) {
	ty_idx = Get_TY(gs_tree_type(gs_type_max_value(gs_type_domain(dim_tree))));
        st = Gen_Temp_Symbol (ty_idx, "__save_vla");
        TYPE_ID mtype = TY_mtype (ty_idx);
        WGEN_Set_ST_Addr_Saved (uwn);     
        uwn = WN_Stid (mtype, 0, st, ty_idx, uwn);
        WGEN_Stmt_Append (uwn, Get_Srcpos());    
        uwn = WN_Ldid (mtype, 0, st, ty_idx);
      }
      else {
        st = WN_st (uwn);
        ty_idx = ST_type (st);
      }
      WN *wn = WN_CreateXpragma (WN_PRAGMA_COPYIN_BOUND,
						   (ST_IDX) NULL, 1);
      WN_kid0 (wn) = WN_Ldid (TY_mtype (ty_idx), 0, st, ty_idx);
      WGEN_Stmt_Append (wn, Get_Srcpos());
      Clear_ARB_const_ubnd (arb);
      Set_ARB_ubnd_var (arb, ST_st_idx (st));
    }
  }
  else {
    Clear_ARB_const_ubnd (arb);
    Set_ARB_ubnd_val (arb, 0);
  }

#if 0 // [SC] Does not work for multiple dimensions.
  // This code first initializes the variable that holds the
  // array size to be the size in bits, because that is what
  // gcc expects.
  // It then overwrites the value of the variable with the
  // size in bytes, because that is what the open64 alloca code
  // expects (wfe_decl.cxx/WFE_Alloca_ST).
  // Unfortunately, the variable may be referenced again in a tree
  // generated by gcc for the size of an outer dimension, when
  // there are multiple dimensions.
  // In that tree it is expected to be the size in bits, but
  // because it has been overwritten with the byte size, it contains
  // the wrong value.
  // I have removed this code, and fixed WFE_Alloca_ST to
  // convert the bit size to bytes.
  
  if (variable_size) {
    WN *swn, *wn;
    swn = WFE_Expand_Expr(TYPE_SIZE(dim_tree));
    if (TY_size(*ty_dim0)) {
      if (WN_opcode (swn) == OPC_U4I4CVT ||
	  WN_opcode (swn) == OPC_U5I5CVT ||
	  WN_opcode (swn) == OPC_U8I8CVT) {
	swn = WN_kid0 (swn);
      }
      FmtAssert (WN_operator (swn) == OPR_LDID,
					("size operator for VLA not LDID"));
      ST *st = WN_st (swn);
      TY_IDX ty_idx = ST_type (st);
      TYPE_ID mtype = TY_mtype (ty_idx);
      swn = WN_Div (mtype, swn, WN_Intconst (mtype, BITSPERBYTE));
      wn = WN_Stid (mtype, 0, st, ty_idx, swn);
      WFE_Stmt_Append (wn, Get_Srcpos());
    }
  }
#endif

  return;
}

/* ====================================================================
 *   mk_array_dimension
 *
 *   Walk the array_tree and collect data.
 * ====================================================================
 */
static BOOL
mk_array_dimension (
  gs_t array_tree,             // current GNU tree
  INT32 *dim,                  // current dimension
  ARB_HANDLE *array_dims,      // fill this out
  TY_IDX *ty_dim0,             // return TY_IDX of element
  INT32 *array_rank,           // return dimensions
  INT64 *bitsize               // return size of current dimension
)
{
  // Save the TY associated with dim=0, as it's needed for
  // TY_etype of the TY_ARI.

  switch (gs_tree_code(array_tree)) {
      
    case GS_ARRAY_TYPE:
      // If the array is of type array, walk through it 
      // If we encountered something else than what we accept as
      // basic array type, we will need to make a la gcc, i.e.
      // an array of array of array ...
      // Do it after unwinding the recursion stack.
      (*array_rank)++;
      if (!mk_array_dimension (gs_tree_type(array_tree),
			       dim, array_dims, ty_dim0,
			       array_rank, bitsize)) {
	if (*dim == *array_rank-1) {
	  // we haven't succeded and we'll just do as usual
	  //
	  // assumes 1 dimension
	  // nested arrays are treated as arrays of arrays
	  *ty_dim0 = Get_TY(gs_tree_type(array_tree));
	  *array_rank = 1;
	  *dim = 0;
	}
	else {
	  // continue unwinding the recursion stack
	  (*dim)++;
	  return FALSE;
	}
      }
      break;

    default:

      // got to the bottom of it, initialize the basic induction case:
      *ty_dim0 = Get_TY(array_tree);

      return TRUE;
  }

  // Make a new array dimension. 
  // The dimensions are processed in order 1->rank of array.
  //
  // NOTE: from crayf90/sgi/cwh_types.cxx fei_array_dimen().

  ARB_HANDLE p;

  if (*dim == 0) {
     *array_dims = New_ARB();
     p = *array_dims;
  } else {
     p = New_ARB();
  }

  initialize_arb (p, array_tree, ty_dim0, bitsize);

  // increment the dimension:
  (*dim)++;

  return TRUE;
}

/* ====================================================================
 *   GNU_TO_WHIRL_mk_array_type
 *
 *   Gets a gcc tree, returns the TY_IDX or NULL_IDX if the array is
 *   not "good".
 *
 *   idx is non-zero only for RECORD and UNION, when there is 
 *   forward declaration ??. so it is 0 here ?
 * ====================================================================
 */
static TY_IDX
GNU_TO_WHIRL_mk_array_type (
  gs_t type_tree, 
  TY_IDX idx
)
{
  // For now only ARRAY_TYPE is treated:
  Is_True((gs_tree_code(type_tree) == GS_ARRAY_TYPE),
	              ("GNU_TO_WHIRL_mk_array_type: expected GS_ARRAY_TYPE"));

  // initialize:
  ARB_HANDLE array_dims;
  INT32 array_rank = 0;
  TY_IDX ty_dim0 = TY_IDX_ZERO;
  INT64 bitsize = 0;
  INT32 dim = 0;

  // We prefer first walk the tree and make sure that what's under
  // there is OK. There may be cases when it is advantegeous to
  // leave the thing as array of array of array etc.
  mk_array_dimension (type_tree, 
		      &dim,        // current dimension
		      &array_dims, // fill this out
		      &ty_dim0,    // return TY_IDX of element
		      &array_rank, // return dimensions
		      &bitsize);   // return size of current dimension

  FmtAssert(ty_dim0 != TY_IDX_ZERO, ("something went wrong ??"));

  Set_ARB_first_dimen (array_dims[0]);
  Set_ARB_last_dimen (array_dims[array_rank-1]);
  for (UINT i = 0; i < array_rank; ++i) {
    Set_ARB_dimension (array_dims[i], array_rank-i);
  }

#ifdef KEY /* bug 8346 */
  TY &ty = (idx == TY_IDX_ZERO) ? New_TY(idx) : Ty_Table[idx];
  Clear_TY_is_incomplete (idx);
#else
  TY &ty = New_TY (idx);
#endif
  TY_Init (ty, bitsize/BITSPERBYTE, KIND_ARRAY, MTYPE_M, 
			Save_Str(Get_Name(gs_type_name(type_tree))));
  Set_TY_etype (ty, ty_dim0);
  Set_TY_align (idx, TY_align(ty_dim0));
  Set_TY_arb (ty, array_dims[0]);

  return idx;
}
#endif /* TARG_ST */


// look up the attribute given by attr_name in the attribute list
gs_t 
lookup_attribute(const char *attr_name, gs_t attr_list)
{
  gs_t nd;
  for (nd = attr_list; nd; nd = gs_tree_chain(nd)) {
    Is_True(gs_tree_code(nd) == GS_TREE_LIST,
	    ("lookup_attributes: TREE_LIST node not found")); 
    gs_t attr = gs_tree_purpose(nd);
    if (is_attribute(attr_name, attr))
      return nd;
  }
  return NULL;
}

// idx is non-zero only for RECORD and UNION, when there is forward declaration
extern TY_IDX
Create_TY_For_Tree (gs_t type_tree, TY_IDX idx)
{

	if(gs_tree_code(type_tree) == GS_ERROR_MARK)
	   return idx;

	TY_IDX orig_idx = idx;
	if(gs_tree_code_class(type_tree) != GS_TCC_TYPE) {
	  DevWarn("Bad tree class passed to Create_TY_For_Tree %c",
		gs_tree_code_class(type_tree));
          return idx;
	}


#ifdef KEY
	UINT align = gs_type_align(type_tree) / BITSPERBYTE;
#endif
	// for typedefs get the information from the base type
	if (gs_type_name(type_tree) &&
	    idx == 0 &&
	    (gs_tree_code(type_tree) == GS_RECORD_TYPE ||
	     gs_tree_code(type_tree) == GS_UNION_TYPE) &&
	    gs_tree_code(gs_type_name(type_tree)) == GS_TYPE_DECL &&
	    gs_type_main_variant(type_tree) != type_tree) {
		idx = Get_TY (gs_type_main_variant(type_tree));
		if (gs_type_readonly(type_tree))
			Set_TY_is_const (idx);
		if (gs_type_volatile(type_tree))
			Set_TY_is_volatile (idx);
#ifdef KEY
		if (gs_type_restrict(type_tree))
			Set_TY_is_restrict (idx);
		Set_TY_align (idx, align); // bug 10533
#endif
		TYPE_TY_IDX(type_tree) = idx;
		if(Debug_Level >= 2) {
#ifdef KEY // bug 11782
#ifdef TARG_ST
		  defer_DST_type(type_tree, idx, 0);
#else                  
		  defer_DST_type(type_tree, idx, orig_idx);
#endif
#else
		  DST_INFO_IDX dst = Create_DST_type_For_Tree(type_tree,
			idx,orig_idx);
		  TYPE_DST_IDX(type_tree) = dst;
#endif
	        }
#ifdef TARG_ST
		if (TYPE_FIELD_IDS_USED_KNOWN(gs_type_main_variant(type_tree))) {
		  SET_TYPE_FIELD_IDS_USED(type_tree,
					  GET_TYPE_FIELD_IDS_USED(gs_type_main_variant(type_tree)));
		}
#else
		TYPE_FIELD_IDS_USED(type_tree) =
			TYPE_FIELD_IDS_USED(gs_type_main_variant(type_tree));
#endif
		return idx;
	}

	TYPE_ID mtype;
	INT64 tsize;
	BOOL variable_size = FALSE;
	gs_t type_size = gs_type_size(type_tree);
#ifndef KEY
	UINT align = gs_type_align(type_tree) / BITSPERBYTE;
#endif
	if (type_size == NULL) {
		// incomplete structs have 0 size.  Similarly, 'void' is
                // an incomplete type that can never be completed.
#ifdef TARG_ST
		FmtAssert(gs_tree_code(type_tree) == GS_ARRAY_TYPE 
			|| gs_tree_code(type_tree) == GS_ENUMERAL_TYPE
			|| gs_tree_code(type_tree) == GS_UNION_TYPE
			|| gs_tree_code(type_tree) == GS_RECORD_TYPE
			|| gs_tree_code(type_tree) == GS_LANG_TYPE
			|| gs_tree_code(type_tree) == GS_FUNCTION_TYPE
			|| gs_tree_code(type_tree) == GS_VOID_TYPE
			|| gs_tree_code(type_tree) == GS_TEMPLATE_TYPE_PARM,
			  ("Create_TY_For_Tree: type_size NULL for non ARRAY/RECORD/VOID, type is %d",
                           (int) gs_tree_code(type_tree)));
#else
		FmtAssert(gs_tree_code(type_tree) == GS_ARRAY_TYPE 
			|| gs_tree_code(type_tree) == GS_ENUMERAL_TYPE
			|| gs_tree_code(type_tree) == GS_UNION_TYPE
			|| gs_tree_code(type_tree) == GS_RECORD_TYPE
			|| gs_tree_code(type_tree) == GS_LANG_TYPE
			|| gs_tree_code(type_tree) == GS_FUNCTION_TYPE
			|| gs_tree_code(type_tree) == GS_VOID_TYPE,
			  ("Create_TY_For_Tree: type_size NULL for non ARRAY/RECORD/VOID, type is %d",
                           (int) gs_tree_code(type_tree)));
#endif
		tsize = 0;
	}
	else {
		if (gs_tree_code(type_size) != GS_INTEGER_CST) {
			if (gs_tree_code(type_tree) == GS_ARRAY_TYPE)
				DevWarn ("Encountered VLA at line %d", lineno);
			else
#if !defined KEY || defined TARG_ST
				Fail_FmtAssertion ("VLA at line %d not currently implemented", lineno);
#else
			// bugs 943, 11277, 10506
			{
			  // Should use ErrMsg (or something similar) instead.
			  printf("pathcc: variable-length structure not yet implemented\n");
			  exit(2);
			}
#endif
			variable_size = TRUE;
			tsize = 0;
		}
		else
#ifdef KEY		// bug 3045
			tsize = (gs_get_integer_value(type_size) + BITSPERBYTE - 1)
				  / BITSPERBYTE;
#else
			tsize = gs_get_integer_value(type_size) / BITSPERBYTE;
#endif
	}
	switch (gs_tree_code(type_tree)) {
	case GS_VOID_TYPE:
	case GS_LANG_TYPE: // unknown type
		idx = MTYPE_To_TY (MTYPE_V);	// use predefined type
		break;
	case GS_BOOLEAN_TYPE:
	case GS_INTEGER_TYPE:
	case GS_OFFSET_TYPE:
		switch (tsize) {
		case 1:  mtype = MTYPE_I1;  break;
		case 2:  mtype = MTYPE_I2;  break;
		case 4:  mtype = MTYPE_I4;  break;
#ifdef TARG_ST
                  /* (cbr) */
                case 5:  mtype = MTYPE_I5; break;
#endif
		case 8:  mtype = MTYPE_I8;  break;
#if !defined(TARG_X8664) && !defined(TARG_MIPS)  // Bug 12358
#ifdef _LP64
		case 16:  mtype = MTYPE_I8; break;
#endif /* _LP64 */
#else 
	        // needed for compiling variable length array
		// as in gcc.c-torture/execute/920929-1.c
		// we need to fix the rest of the compiler 
		// with _LP64 but seems to work fine without.	
		case 16:  mtype = MTYPE_I8; break;
#endif /* KEY */
		default:  FmtAssert(FALSE,
                                    ("Get_TY unexpected size %d", tsize));
		}
#ifdef TARG_ST
                /* (cbr) make sure boolean is signed as Boolean_type in whirl */
		if (gs_tree_code(type_tree) != GS_BOOLEAN_TYPE)
#endif
		if (gs_decl_unsigned(type_tree)) {
			mtype = MTYPE_complement(mtype);
		}
#ifndef TARG_ST
// [SC] Handle may_alias more generally below, since it could also appear for
// enum and record types, not just scalars.
#ifdef KEY
		if (lookup_attribute("may_alias",gs_type_attributes(type_tree)))
		{
		  // bug 9975: Handle may_alias attribute, we need to create
		  // a new type to which we can attach the flag.
		  TY &ty = New_TY (idx);
		  TY_Init (ty, tsize, KIND_SCALAR, mtype, 
		           Save_Str(Get_Name(gs_type_name(type_tree))) );
		  Set_TY_no_ansi_alias (ty);
 		} else
#endif
#endif
		idx = MTYPE_To_TY (mtype);	// use predefined type
#ifdef TARG_X8664
		/* At least for -m32, the alignment is not the same as the data
		   type's natural size. (bug#2932)
		*/
		if( TARGET_64BIT )
#endif // TARG_X8664
		  Set_TY_align (idx, align);
		break;
	case GS_CHAR_TYPE:
		mtype = (gs_decl_unsigned(type_tree) ? MTYPE_U1 : MTYPE_I1);
		idx = MTYPE_To_TY (mtype);	// use predefined type
		break;
	case GS_ENUMERAL_TYPE:
#ifdef KEY
		switch (tsize) {
		  case 1: // bug 14445
		        mtype = (gs_decl_unsigned(type_tree) ? MTYPE_U1 :
		                                               MTYPE_I1);
		        break;
		  case 2: // bug 14445
		        mtype = (gs_decl_unsigned(type_tree) ? MTYPE_U2 :
		                                               MTYPE_I2);
		        break;
#ifdef TARG_ST
		  case 4:
		        mtype = (gs_decl_unsigned(type_tree) ? MTYPE_U4 :
				 MTYPE_I4);
			break;
		  case 5:
		        mtype = (gs_decl_unsigned(type_tree) ? MTYPE_U5 :
				 MTYPE_I5);
			break;
#endif
		  case 8: // bug 500
		        mtype = (gs_decl_unsigned(type_tree) ? MTYPE_U8 :
		                                               MTYPE_I8);
		        break;
		  default:
		        mtype = (gs_decl_unsigned(type_tree) ? MTYPE_U4 :
		                                               MTYPE_I4);
		}
#else
		mtype = (gs_decl_unsigned(type_tree) ? MTYPE_U4 : MTYPE_I4);
#endif
		idx = MTYPE_To_TY (mtype);	// use predefined type
		break;
	case GS_REAL_TYPE:
		switch (tsize) {
		case 4:  mtype = MTYPE_F4; break;
		case 8:  mtype = MTYPE_F8; break;
#ifdef TARG_X8664
		case 12: mtype = MTYPE_FQ; break;
#endif
#if defined(TARG_MIPS) || defined(TARG_IA32) || defined(TARG_X8664)
		case 16: mtype = MTYPE_FQ; break;
#endif /* TARG_MIPS */
		default:  FmtAssert(FALSE, ("Get_TY unexpected size"));
		}
		idx = MTYPE_To_TY (mtype);	// use predefined type
		break;
	case GS_COMPLEX_TYPE:
		switch (tsize) {
		case 2: 
		case 4: ErrMsg (EC_Unsupported_Type, "Complex integer");
		case  8:  mtype = MTYPE_C4; break;
		case 16:  mtype = MTYPE_C8; break;
#ifdef TARG_X8664
		case 24:  mtype = MTYPE_CQ; break;
#endif
#if defined(TARG_MIPS) || defined(TARG_IA32) || defined(TARG_X8664)
		case 32: mtype = MTYPE_CQ; break;
#endif /* TARG_MIPS */
		default:  FmtAssert(FALSE, ("Get_TY unexpected size"));
		}
		idx = MTYPE_To_TY (mtype);	// use predefined type
		break;
	case GS_POINTER_TYPE:
		if (gs_type_ptrmem_p(type_tree)) {
			// pointer to member
			idx = Be_Type_Tbl(Pointer_Size == 8 ? MTYPE_I8 : MTYPE_I4);
			break;
		}
		/* FALLTHRU */
	case GS_REFERENCE_TYPE:
		idx = Make_Pointer_Type (Get_TY (gs_tree_type(type_tree)));
		Set_TY_align (idx, align);
		break;
	case GS_ARRAY_TYPE:
#ifdef TARG_ST
	  // This makes multi-dimensional arrays.
	  idx = GNU_TO_WHIRL_mk_array_type (type_tree, idx);
	  /* (cbr) set type align */
	  if (align)
	    Set_TY_align (idx, align);
#else
		{	// new scope for local vars
#if defined( KEY)  /* bug 8346 */
		TY &ty = (idx == TY_IDX_ZERO) ? New_TY(idx) : Ty_Table[idx];
		Clear_TY_is_incomplete (idx);
#else
		TY &ty = New_TY (idx);
#endif
		TY_Init (ty, tsize, KIND_ARRAY, MTYPE_M, 
			Save_Str(Get_Name(gs_type_name(type_tree))) );
		Set_TY_etype (ty, Get_TY (gs_tree_type(type_tree)));
		Set_TY_align (idx, TY_align(TY_etype(ty)));
		// assumes 1 dimension
		// nested arrays are treated as arrays of arrays
		ARB_HANDLE arb = New_ARB ();
		ARB_Init (arb, 0, 0, 0);
		Set_TY_arb (ty, arb);
		Set_ARB_first_dimen (arb);
		Set_ARB_last_dimen (arb);
		Set_ARB_dimension (arb, 1);
		if (gs_type_size(gs_tree_type(type_tree)) == 0)
			break; // anomaly:  type will never be needed

		// =================== Array stride ======================
		if (gs_tree_code(gs_type_size(gs_tree_type(type_tree))) == GS_INTEGER_CST) {
			Set_ARB_const_stride (arb);
			Set_ARB_stride_val (arb, 
				gs_get_integer_value (gs_type_size(gs_tree_type(type_tree))) 
				/ BITSPERBYTE);
		}
#ifdef KEY /* bug 8346 */
		else if (!expanding_function_definition &&
		         processing_function_prototype)
		{
			Set_ARB_const_stride (arb);
			// dummy stride val 4
			Set_ARB_stride_val (arb, 4);
			Set_TY_is_incomplete (idx);
		}
#endif
		else {
			WN *swn;
			swn = WGEN_Expand_Expr (gs_type_size(gs_tree_type(type_tree)));
			if (WN_opcode (swn) == OPC_U4I4CVT ||
			    WN_opcode (swn) == OPC_U8I8CVT) {
				swn = WN_kid0 (swn);
			}
#ifdef KEY
			// In the event that swn operator is not 
			// OPR_LDID, save expr node swn 
			// and use LDID of that stored address as swn.
			// Copied from Wfe_Save_Expr in wfe_expr.cxx
			if (WN_operator (swn) != OPR_LDID) {
			  TY_IDX    ty_idx  = 
			    Get_TY (gs_tree_type (type_size));
			  TYPE_ID   mtype   = TY_mtype (ty_idx);
			  ST       *st;
			  st = Gen_Temp_Symbol (ty_idx, "__save_expr");
#ifdef FE_GNU_4_2_0
			  WGEN_add_pragma_to_enclosing_regions (WN_PRAGMA_LOCAL, st);
#endif
			  WGEN_Set_ST_Addr_Saved (swn);
			  swn = WN_Stid (mtype, 0, st, ty_idx, swn);
			  WGEN_Stmt_Append (swn, Get_Srcpos());
			  swn = WN_Ldid (mtype, 0, st, ty_idx);
			}
#endif /* KEY */
			FmtAssert (WN_operator (swn) == OPR_LDID,
				("stride operator for VLA not LDID"));
			ST *st = WN_st (swn);
			TY_IDX ty_idx = ST_type (st);
			WN *wn = WN_CreateXpragma (WN_PRAGMA_COPYIN_BOUND,
						   (ST_IDX) NULL, 1);
			WN_kid0 (wn) = WN_Ldid (TY_mtype (ty_idx), 0, st, ty_idx);
			WGEN_Stmt_Append (wn, Get_Srcpos());
			Clear_ARB_const_stride (arb);
			Set_ARB_stride_var (arb, (ST_IDX) ST_st_idx (st));
#ifdef KEY /* bug 8346 */
			Clear_TY_is_incomplete (idx);
#endif
		}

		// ================= Array lower bound =================
		Set_ARB_const_lbnd (arb);
		Set_ARB_lbnd_val (arb, 0);

		// ================= Array upper bound =================
		if (type_size) {
#ifdef KEY
		    // For Zero-length arrays, TYPE_MAX_VALUE tree is NULL
		    if (!gs_type_max_value (gs_type_domain (type_tree))) {
			Set_ARB_const_ubnd (arb);
			Set_ARB_ubnd_val (arb, 0xffffffff);
		    } else
#endif /* KEY */
		    if (gs_tree_code(gs_type_max_value (gs_type_domain (type_tree))) ==
			GS_INTEGER_CST) {
			Set_ARB_const_ubnd (arb);
			Set_ARB_ubnd_val (arb, gs_get_integer_value (
				gs_type_max_value (gs_type_domain (type_tree)) ));
		    }
#ifdef KEY /* bug 8346 */
		    else if (!expanding_function_definition &&
		             processing_function_prototype) {
			Set_ARB_const_ubnd (arb);
			// dummy upper bound 8
			Set_ARB_ubnd_val (arb, 8);
			Set_TY_is_incomplete (idx);
		    }
#endif
		    else {
			WN *uwn = WGEN_Expand_Expr (gs_type_max_value (gs_type_domain (type_tree)) );
			if (WN_opcode (uwn) == OPC_U4I4CVT ||
			    WN_opcode (uwn) == OPC_U8I8CVT) {
				uwn = WN_kid0 (uwn);
			}
			ST *st;
			TY_IDX ty_idx;
			WN *wn;
			if (WN_operator (uwn) != OPR_LDID) {
				ty_idx = Get_TY (gs_tree_type (gs_type_max_value (gs_type_domain (type_tree)) ) );
				st = Gen_Temp_Symbol (ty_idx, "__vla_bound");
#ifdef FE_GNU_4_2_0
			  	WGEN_add_pragma_to_enclosing_regions (WN_PRAGMA_LOCAL, st);
#endif
				wn = WN_Stid (TY_mtype (ty_idx), 0, st, ty_idx, uwn);
				WGEN_Stmt_Append (wn, Get_Srcpos());
			}
			else {
				st = WN_st (uwn);
				ty_idx = ST_type (st);
			}
			wn = WN_CreateXpragma (WN_PRAGMA_COPYIN_BOUND, (ST_IDX) NULL, 1);
			WN_kid0 (wn) = WN_Ldid (TY_mtype (ty_idx), 0, st, ty_idx);
			WGEN_Stmt_Append (wn, Get_Srcpos());
			Clear_ARB_const_ubnd (arb);
			Set_ARB_ubnd_var (arb, ST_st_idx (st));
#ifdef KEY /* bug 8346 */
			Clear_TY_is_incomplete (idx);
#endif
		    }
		}
		else {
			Clear_ARB_const_ubnd (arb);
			Set_ARB_ubnd_val (arb, 0);
		}

		// ==================== Array size ====================
		if (variable_size) {
#ifdef KEY /* bug 8346 */
		   if (!expanding_function_definition &&
		       processing_function_prototype) {
		     Set_TY_is_incomplete (idx);
		   }
		   else
#endif
		   {
			WN *swn, *wn;
			swn = WGEN_Expand_Expr (type_size);
			if (TY_size(TY_etype(ty))) {
				if (WN_opcode (swn) == OPC_U4I4CVT ||
				    WN_opcode (swn) == OPC_U8I8CVT) {
					swn = WN_kid0 (swn);
				}
#ifdef KEY
				// In the event that swn operator is not 
				// OPR_LDID, save expr node swn 
				// and use LDID of that stored address as swn.
				// Copied from Wfe_Save_Expr in wfe_expr.cxx
				if (WN_operator (swn) != OPR_LDID) {
				  TY_IDX    ty_idx  = 
				    Get_TY (gs_tree_type (type_size));
				  TYPE_ID   mtype   = TY_mtype (ty_idx);
				  ST       *st;
				  st = Gen_Temp_Symbol (ty_idx, "__save_expr");
#ifdef FE_GNU_4_2_0
			  	  WGEN_add_pragma_to_enclosing_regions (WN_PRAGMA_LOCAL, st);
#endif
				  WGEN_Set_ST_Addr_Saved (swn);
				  swn = WN_Stid (mtype, 0, st, ty_idx, swn);
				  WGEN_Stmt_Append (swn, Get_Srcpos());
				  swn = WN_Ldid (mtype, 0, st, ty_idx);
				}
#endif /* KEY */
				FmtAssert (WN_operator (swn) == OPR_LDID,
					("size operator for VLA not LDID"));
				ST *st = WN_st (swn);
				TY_IDX ty_idx = ST_type (st);
				TYPE_ID mtype = TY_mtype (ty_idx);
				swn = WN_Div (mtype, swn, WN_Intconst (mtype, BITSPERBYTE));
				wn = WN_Stid (mtype, 0, st, ty_idx, swn);
				WGEN_Stmt_Append (wn, Get_Srcpos());
			}
#ifdef KEY /* bug 8346 */
			Clear_TY_is_incomplete (idx);
#endif
		   }
		}
		} // end array scope
#endif /* TARG_ST */
		break;
	case GS_RECORD_TYPE:
	case GS_UNION_TYPE:
		{	// new scope for local vars

		TY &ty = (idx == TY_IDX_ZERO) ? New_TY(idx) : Ty_Table[idx];
#ifdef KEY
		// Must create DSTs in the order that the records are declared,
		// in order to preserve their scope.  Bug 4168.
		if (Debug_Level >= 2)
		  defer_DST_type(type_tree, idx, orig_idx);
#ifndef TARG_ST
		// [SC] C++ language standard requires that empty classes
		// have non-zero size, so I do not think it is valid to
		// set tsize = 0 here.

		// GCC 3.2 pads empty structures with a fake 1-byte field.
		// These structures should have tsize = 0.
		if (tsize != 0 &&
		    // is_empty_class assumes non-null CLASSTYPE_SIZE
		    // check if it has lang-specific data
		    gs_type_lang_specific(type_tree) &&
		    // check if it has its base version set
		    gs_classtype_as_base(type_tree) &&
		    gs_classtype_size(type_tree) &&
		    gs_is_empty_class(type_tree))
			tsize = 0;
#endif
#endif	// KEY
		TY_Init (ty, tsize, KIND_STRUCT, MTYPE_M, 
			Save_Str(Get_Name(gs_type_name(type_tree))) );
		if (gs_tree_code(type_tree) == GS_UNION_TYPE) {
			Set_TY_is_union(idx);
		}
#ifdef KEY
		if (gs_aggregate_value_p(type_tree)) {
			Set_TY_return_in_mem(idx);
		}
#endif
		if (align == 0) align = 1;	// in case incomplete type
		Set_TY_align (idx, align);
		// set idx now in case recurse thru fields
		TYPE_TY_IDX(type_tree) = idx;
#ifdef TARG_ST
                /* (cbr) mark non-pods */
                if (gs_type_lang_specific (type_tree) &&
                    gs_tree_addressable (type_tree) && 
                    gs_classtype_non_pod_p (type_tree)) 

                  Set_TY_is_non_pod(idx);
#endif               
		Do_Base_Types (type_tree);

		// Process nested structs and static data members first

                for (gs_t field =  get_first_real_or_virtual_field (type_tree);
                          field;
                          field = next_real_field(type_tree, field)) {
		  	Set_TY_content_seen(idx); // bug 10851
                        if (gs_tree_code(field) == GS_TYPE_DECL ||
			    gs_tree_code(field) == GS_FIELD_DECL) {
                                gs_t field_type = gs_tree_type(field);
				if ((gs_tree_code(field_type) == GS_RECORD_TYPE ||
				     gs_tree_code(field_type) == GS_UNION_TYPE) &&
                                    field_type != type_tree) {
#ifdef KEY
					// Defer typedefs within class
					// declarations to avoid circular
					// declaration dependences.  See
					// example in bug 5134.
                                        if (gs_tree_code(field) == GS_TYPE_DECL)
					  defer_decl(field_type);
                                        else
#endif
                                        Get_TY(field_type);
				}
                        }
#ifdef KEY	// Defer expansion of static vars until all the fields in
		// _every_ struct are laid out.  Consider this code (see
		// bug 3044):
		//  struct A
		//    struct B *p
		//  struct B
		//    static struct A *q = ...	// static data member with
		//                              // initializer
		// We cannot expand static member vars while expanding the
		// enclosing stuct, for the following reason:  Expansion of
		// struct A leads to expansion of p, which leads to the
		// expansion of struct B, which leads to the expansion of q and
		// q's initializer.  The code that expands the initializer goes
		// through the fields of struct A, but these fields are not yet
		// completely defined, and this will cause kg++fe to die.
		//
		// The solution is the delay all static var expansions until
		// the very end.
			else if (gs_tree_code(field) == GS_VAR_DECL)
				defer_decl(field);
#else
			else if (gs_tree_code(field) == GS_VAR_DECL)
				WGEN_Expand_Decl(field, TRUE);
#endif
			else if (gs_tree_code(field) == GS_TEMPLATE_DECL)
				WGEN_Expand_Decl(field, TRUE);
	        }

  		Set_TY_fld (ty, FLD_HANDLE());
		FLD_IDX first_field_idx = Fld_Table.Size ();
		gs_t field;
		gs_t method = gs_type_methods(type_tree);
		FLD_HANDLE fld;
		INT32 next_field_id = 1;

#ifdef KEY
		// In GCC 4, the same tree node representing a vtable ptr field
		// can appear in different derived classes.  As a result,
		// DECL_FIELD_ID(field) can't be used to map its field ID.  As
		// a fix, always allocate field ID 1 to the vtable ptr field.
		// Do this before allocating IDs to any other field.
		gs_t vfield = get_virtual_field(type_tree); 
		if (vfield) {
		  Is_True(gs_tree_code(vfield) == GS_FIELD_DECL,
			  ("Create_TY_For_Tree: bad vfield code"));
		  Is_True(gs_decl_name(vfield) &&
			  !strncmp(Get_Name(gs_decl_name(vfield)),"_vptr", 5),
			  ("Create_TY_For_Tree: bad vfield name"));
		  // The vfield field ID is either not set, or was set to 1.
		  Is_True(DECL_FIELD_ID(vfield) <= 1,
			  ("Create_TY_For_Tree: invalid vfield field ID"));

		  DECL_FIELD_ID(vfield) = next_field_id;	// must be 1
#ifdef TARG_ST
		  next_field_id +=
		    GET_TYPE_FIELD_IDS_USED(gs_tree_type(vfield)) + 1;
#else
		  next_field_id += TYPE_FIELD_IDS_USED(gs_tree_type(vfield)) +1;
#endif
		  fld = New_FLD ();
		  FLD_Init(fld, Save_Str(Get_Name(gs_decl_name(vfield))), 
			   0, // type
			   gs_get_integer_value(gs_decl_field_offset(vfield))
			    + gs_get_integer_value(gs_decl_field_bit_offset(vfield))
			    / BITSPERBYTE);
		}
#endif

		// Generate an anonymous field for every direct, nonempty,
		// nonvirtual base class.  

		INT32 offset = 0;
		INT32 anonymous_fields = 0;
#ifndef KEY	// g++'s class.c already laid out the base types.  Bug 11622.
		gs_t type_binfo, basetypes;
		if ((type_binfo = gs_type_binfo(type_tree)) != NULL &&
		    (basetypes = gs_binfo_base_binfos(type_binfo)) != NULL) {
		  gs_t list;
		  for (list = basetypes; gs_code(list) != EMPTY;
		       list = gs_operand(list, 1)) {
		    gs_t binfo = gs_operand(list, 0);
		    gs_t basetype = gs_binfo_type(binfo);
		    offset = Roundup (offset,
				    gs_type_align(basetype) / BITSPERBYTE);
		    if (!is_empty_base_class(basetype) || 
			!gs_binfo_virtual_p(binfo)) {
		      ++next_field_id;
		      ++anonymous_fields;
		      next_field_id += TYPE_FIELD_IDS_USED(basetype);
		      fld = New_FLD();
		      FLD_Init (fld, Save_Str(Get_Name(0)), 
				Get_TY(basetype), offset);
		      offset += Type_Size_Without_Vbases (basetype);
#ifdef KEY
// temporary hack for a bug in gcc
// Details: From layout_class_type(), it turns out that for this
// type, gcc is apparently sending wrong type info, they have 2 fields
// each 8 bytes in a 'record', with the type size == 8 bytes also!
// So we take care of it here...
		      if (offset > tsize)
			{
			    tsize = offset;
			    Set_TY_size (ty, tsize);
			}
#endif // KEY
		    }
		  }
		}
#endif // KEY

		// Assign IDs to real fields.  The vtable ptr field is already
		// assigned ID 1.
		for (field = get_first_real_field(type_tree); 
			field;
			field = next_real_field(type_tree, field) )
		{
			if (gs_tree_code(field) == GS_TYPE_DECL) {
				continue;
			}
			if (gs_tree_code(field) == GS_CONST_DECL) {
				DevWarn ("got CONST_DECL in field list");
				continue;
			}
			if (gs_tree_code(field) == GS_VAR_DECL) {
				continue;	
			}
			if (gs_tree_code(field) == GS_TEMPLATE_DECL) {
				continue;
			}

#ifdef TARG_ST
                        /* (cbr) it's possible not to have any layout info, e.g 
                         we belong to a template */
                        if (!gs_decl_field_offset(field)) {
                          continue;
                        }
#endif
			// Either the DECL_FIELD_ID is not yet set, or is
			// already set to the same field ID.  The latter
			// happens when GCC 4 duplicates the type tree and the
			// same field node appears in both type nodes.
			Is_True(DECL_FIELD_ID(field) == 0 ||
				DECL_FIELD_ID(field) == next_field_id,
				("Create_TY_For_Tree: field ID already set"));

			DECL_FIELD_ID(field) = next_field_id;
			next_field_id += 
#ifdef TARG_ST
			  GET_TYPE_FIELD_IDS_USED(gs_tree_type(field)) + 1;
#else
			  TYPE_FIELD_IDS_USED(gs_tree_type(field)) + 1;
#endif
			fld = New_FLD ();
			FLD_Init (fld, Save_Str(Get_Name(gs_decl_name(field))), 
				0, // type
				gs_get_integer_value(gs_decl_field_offset(field)) +
				gs_get_integer_value(gs_decl_field_bit_offset(field))
					/ BITSPERBYTE);
		}

#ifdef TARG_ST
		SET_TYPE_FIELD_IDS_USED(type_tree, next_field_id - 1);
#else
		TYPE_FIELD_IDS_USED(type_tree) = next_field_id - 1;
#endif
  		FLD_IDX last_field_idx = Fld_Table.Size () - 1;
		if (last_field_idx >= first_field_idx) {
			Set_TY_fld (ty, FLD_HANDLE (first_field_idx));
			Set_FLD_last_field (FLD_HANDLE (last_field_idx));
		}

		// now set the fld types.
		fld = TY_fld(ty);
#ifdef KEY
		// Handle the vtable ptr field if it exists.
		if (vfield) {
		  Is_True(gs_tree_code(gs_tree_type(vfield)) == GS_POINTER_TYPE,
		  ("Create_TY_For_Tree: vtable ptr should be GS_POINTER_TYPE"));

		  // As mentioned below, don't expand pointer-type fields to
		  // avoid circular dependences.  Defer expanding the field
		  // type.
		  fld = TY_fld(ty);
		  TY_IDX p_idx = Make_Pointer_Type(MTYPE_To_TY(MTYPE_U8),FALSE);
		  Set_FLD_type(fld, p_idx);
		  defer_field(vfield, fld);
		  fld = FLD_next(fld);
		}
#endif
		// first skip the anonymous fields, whose types are already
		// set.
		while (anonymous_fields--)
		  fld = FLD_next(fld);

		for (field = get_first_real_field(type_tree);
		     /* ugly hack follows; traversing the fields isn't
                        the same from run-to-run. fwa? */
			field && fld.Entry();
			field = next_real_field(type_tree, field))
		{
#ifdef KEY
			const  int FLD_BIT_FIELD_SIZE   = 64;
#endif
			if (gs_tree_code(field) == GS_TYPE_DECL)
				continue;
			if (gs_tree_code(field) == GS_CONST_DECL)
				continue;
			if (gs_tree_code(field) == GS_VAR_DECL)
				continue;
			if (gs_tree_code(field) == GS_TEMPLATE_DECL)
				continue;
#ifdef TARG_ST
                        /* (cbr) it's possible not to have any layout info, e.g 
                         we belong to a template */
                        if (!gs_decl_field_offset(field)) {
                          continue;
                        }
#endif
#ifdef KEY
			// Don't expand the field's type if it's a pointer
			// type, in order to avoid circular dependences
			// involving member object types and base types.  See
			// example in bug 4954.  
			if (gs_tree_code(gs_tree_type(field)) == GS_POINTER_TYPE) {
				// Defer expanding the field's type.  Put in a
				// generic pointer type for now.
				TY_IDX p_idx =
				  Make_Pointer_Type(MTYPE_To_TY(MTYPE_U8),
						    FALSE);
				Set_FLD_type(fld, p_idx);
				defer_field(field, fld);
				fld = FLD_next(fld);
				continue;
			}
#endif
			TY_IDX fty_idx = Get_TY(gs_tree_type(field));
#ifdef TARG_ST
			  // [CG]: For structure fields, the qualifiers are
			  // on the field nodes (not on the field node type).
			  // Thus we get them explicitly.
			  if (gs_tree_this_volatile(field)) Set_TY_is_volatile (fty_idx);
			  if (gs_tree_readonly(field)) Set_TY_is_const (fty_idx);
			  if (gs_type_restrict(field)) Set_TY_is_restrict (fty_idx);
#endif
			if ((TY_align (fty_idx) > align) || (TY_is_packed (fty_idx)))
				Set_TY_is_packed (ty);
#if 1 // wgen bug 10470
			if (! gs_tree_this_volatile(field))
			  Clear_TY_is_volatile (fty_idx);
#endif
			Set_FLD_type(fld, fty_idx);

			if ( ! gs_decl_bit_field(field)
#if 1 // wgen bug 10901
			  	&& gs_tree_code(gs_tree_type(field)) != GS_RECORD_TYPE
			  	&& gs_tree_code(gs_tree_type(field)) != GS_UNION_TYPE
#endif
			  	&& gs_decl_size(field) // bug 10305
				&& gs_get_integer_value(gs_decl_size(field)) > 0
#ifdef KEY
// We don't handle bit-fields > 64 bits. For an INT field of 128 bits, we
// make it 64 bits. But then don't set it as FLD_IS_BIT_FIELD.
				&& gs_get_integer_value(gs_decl_size(field)) <= 
				   FLD_BIT_FIELD_SIZE
				// bug 2401
				&& TY_size(Get_TY(gs_tree_type(field))) != 0
#endif
				&& gs_get_integer_value(gs_decl_size(field))
				  != (TY_size(Get_TY(gs_tree_type(field))) 
					* BITSPERBYTE) )
			{
#ifdef KEY
			        FmtAssert( gs_get_integer_value(gs_decl_size(field)) <=
					   FLD_BIT_FIELD_SIZE,
					   ("field size too big") );
#endif
				// for some reason gnu doesn't set bit field
				// when have bit-field of standard size
				// (e.g. int f: 16;).  But we need it set
				// so we know how to pack it, because 
				// otherwise the field type is wrong.
#ifndef TARG_ST
	      // [CG]: Does not need a devwarn
				DevWarn("field size %lld doesn't match type size %" SCNd64, 
					gs_get_integer_value(gs_decl_size(field)),
					TY_size(Get_TY(gs_tree_type(field)))
						* BITSPERBYTE );
#endif
				gs_set_decl_bit_field(field, 1);
			}
			if (gs_decl_bit_field(field)) {
				Set_FLD_is_bit_field (fld);
				// bofst is remaining bits from byte offset
				Set_FLD_bofst (fld, 
					gs_get_integer_value(
						gs_decl_field_bit_offset(field))
						% BITSPERBYTE);
				Set_FLD_bsize (fld, gs_get_integer_value(
                                                           gs_decl_size(field)));
			}
			fld = FLD_next(fld);
		}

#ifndef KEY	// Don't expand methods by going through TYPE_METHODS,
		// because:
		//   1) It is incorrect to translate all methods in
		//      TYPE_METHODS to WHIRL because some of the methods are
		//      never used, and generating the assembly code for them
		//      might lead to undefined symbol references.  Instead,
		//      consult the gxx_emitted_decls list, which has all the
		//      functions (including methods) that g++ has ever emitted
		//      to assembly.
		//   2) Expanding the methods here will cause error when the
		//      methods are for a class B that appears as a field in an
		//      enclosing class A.  When Get_TY is run for A, it will
		//      call Get_TY for B in order to calculate A's field ID's.
		//      (Need Get_TY to find B's TYPE_FIELD_IDS_USED.)  If
		//      Get_TY uses the code below to expand B's methods, it
		//      will lead to error because the expansion requires the
		//      field ID's of the enclosing record (A), and these field
		//      ID's are not yet defined.

		// process methods
		if (!Enable_WGEN_DFE) {
		if (cp_type_quals(type_tree) == TYPE_UNQUALIFIED) {
			while (method != NULL_TREE) {
				WGEN_Expand_Decl (method, TRUE);
				method = TREE_CHAIN(method);
			}
		}
		}
#endif	// KEY
		} //end record scope
		break;
	case GS_METHOD_TYPE:
		//DevWarn ("Encountered METHOD_TYPE at line %d", lineno);
	case GS_FUNCTION_TYPE:
		{	// new scope for local vars
		gs_t arg;
		INT32 num_args, i;
#ifdef KEY /* bug 8346 */
		TY &ty = (idx == TY_IDX_ZERO) ? New_TY(idx) : Ty_Table[idx];
		Clear_TY_is_incomplete (idx);
#else
		TY &ty = New_TY (idx);
#endif
		TY_Init (ty, 0, KIND_FUNCTION, MTYPE_UNKNOWN, 0); 
		Set_TY_align (idx, 1);
		TY_IDX ret_ty_idx;
		TY_IDX arg_ty_idx;
		uint32_t tylist_idx;

		// allocate TYs for return as well as parameters
		// this is needed to avoid mixing TYLISTs if one
		// of the parameters is a pointer to a function

		ret_ty_idx = Get_TY(gs_tree_type(type_tree));
		for (arg = gs_type_arg_types(type_tree);
		     arg;
		     arg = gs_tree_chain(arg))
		{
		  arg_ty_idx = Get_TY(gs_tree_value(arg));
#ifdef KEY /* bug 8346 */
		  if (TY_is_incomplete (arg_ty_idx) ||
		      (TY_kind(arg_ty_idx) == KIND_POINTER &&
		       TY_is_incomplete(TY_pointed(arg_ty_idx))))
		    Set_TY_is_incomplete (idx);
#endif
		}

		// if return type is pointer to a zero length struct
		// convert it to void
		if (!WGEN_Keep_Zero_Length_Structs    &&
		    TY_mtype (ret_ty_idx) == MTYPE_M &&
		    TY_size (ret_ty_idx) == 0) {
			// zero length struct being returned
		  	DevWarn ("function returning zero length struct at line %d", lineno);
			ret_ty_idx = Be_Type_Tbl (MTYPE_V);
		}

#ifdef KEY
		// If the front-end adds the fake first param, then convert the
		// function to return void.
		if (TY_return_in_mem(ret_ty_idx)) {
#ifndef TARG_ST
		  // [SC] The function return type should still be
		  // specified truthfully, the presence of TY_return_to_param
		  // is sufficient to indicate in the whirl that the return
		  // is via a parameter.
		  ret_ty_idx = Be_Type_Tbl (MTYPE_V);
#endif
		  Set_TY_return_to_param(idx);		// bugs 2423 2424
		}
#endif
		Set_TYLIST_type (New_TYLIST (tylist_idx), ret_ty_idx);
		Set_TY_tylist (ty, tylist_idx);
		for (num_args = 0, arg = gs_type_arg_types(type_tree);
		     arg;
		     num_args++, arg = gs_tree_chain(arg))
		{
			arg_ty_idx = Get_TY(gs_tree_value(arg));
			Is_True (!TY_is_incomplete (arg_ty_idx) ||
			          TY_is_incomplete (idx),
				  ("Create_TY_For_Tree: unexpected TY flag"));
			if (!WGEN_Keep_Zero_Length_Structs    &&
			    TY_mtype (arg_ty_idx) == MTYPE_M &&
			    TY_size (arg_ty_idx) == 0) {
				// zero length struct passed as parameter
				DevWarn ("zero length struct encountered in function prototype at line %d", lineno);
			}
			else
				Set_TYLIST_type (New_TYLIST (tylist_idx), arg_ty_idx);
		}
		if (num_args)
		{
			Set_TY_has_prototype(idx);
			if (arg_ty_idx != Be_Type_Tbl(MTYPE_V))
			{
				Set_TYLIST_type (New_TYLIST (tylist_idx), 0);
				Set_TY_is_varargs(idx);
			}
			else
				Set_TYLIST_type (Tylist_Table [tylist_idx], 0);
		}
		else
			Set_TYLIST_type (New_TYLIST (tylist_idx), 0);
#ifdef TARG_X8664
		if (!TARGET_64BIT && !TY_is_varargs(idx))
		{
		  // Ignore m{sse}regparm and corresponding attributes at -m64.
		  if (SSE_Reg_Parm ||
		      lookup_attribute("sseregparm",
		                       gs_type_attributes(type_tree)))
		    Set_TY_has_sseregister_parm (idx);
		  if (gs_t attr = lookup_attribute("regparm",
		      gs_type_attributes(type_tree)))
		  {
		    gs_t value = gs_tree_value (attr);
		    Is_True (gs_tree_code(value) == GS_TREE_LIST,
		             ("Expected TREE_LIST"));
		    value = gs_tree_value (value);
		    if (gs_tree_code(value) == GS_INTEGER_CST)
		      Set_TY_register_parm (idx, gs_get_integer_value (value));
		  }
		  else if (Reg_Parm_Count)
		    Set_TY_register_parm (idx, Reg_Parm_Count);
		}
#endif
		} // end FUNCTION_TYPE scope
		break;
#ifdef TARG_X8664
        // x86 gcc vector types
        case GS_VECTOR_TYPE:
                {
		char *p = gs_type_mode(type_tree);
		idx = 0;
		if (strcmp(p, "BLK") == 0) {
		  TY_IDX elem_ty = Get_TY(gs_tree_type(type_tree));
		  TYPE_ID elem_mtype = TY_mtype(elem_ty);
		  switch (gs_n(gs_type_precision(type_tree))) {
		    case 2: if (elem_mtype == MTYPE_I4)
		    	      idx = MTYPE_To_TY(MTYPE_V8I4);
			    else if (elem_mtype == MTYPE_F4)
		    	      idx = MTYPE_To_TY(MTYPE_V8F4);
		    	    else if (elem_mtype == MTYPE_I8)
		    	      idx = MTYPE_To_TY(MTYPE_V16I8);
			    else if (elem_mtype == MTYPE_F8)
		    	      idx = MTYPE_To_TY(MTYPE_V16F8);
			    break;
		    case 4: if (elem_mtype == MTYPE_I4)
		    	      idx = MTYPE_To_TY(MTYPE_V16I4);
			    else if (elem_mtype == MTYPE_F4)
		    	      idx = MTYPE_To_TY(MTYPE_V16F4);
		    	    else if (elem_mtype == MTYPE_I2)
		    	      idx = MTYPE_To_TY(MTYPE_M8I2);
			    break;
		    case 8: if (elem_mtype == MTYPE_I1)
		    	      idx = MTYPE_To_TY(MTYPE_M8I1);
		    	    else if (elem_mtype == MTYPE_I2)
		    	      idx = MTYPE_To_TY(MTYPE_V16I2);
			    break;
		    case 16: if (elem_mtype == MTYPE_I1)
		    	       idx = MTYPE_To_TY(MTYPE_V16I1);
			     break;
		    default:
		      Fail_FmtAssertion ("Get_TY: unexpected vector type element count");
		  }
		}
		else { // use string emcoded in TYPE_MODE
		  if (toupper(*p++) != 'V') {
		    if (gs_type_name(type_tree)) {
		      p = gs_identifier_pointer(gs_decl_name(gs_type_name(type_tree)));
		      if (toupper(*p++) != 'V') 
			Fail_FmtAssertion("Get_TY: NYI");
		    }
		    else Fail_FmtAssertion("Get_TY: NYI");
		  }
		  int num_elems = strtol(p, &p, 10);
		  if (strncasecmp(p, "DI", 2) == 0) 
		    idx = MTYPE_To_TY(MTYPE_V16I8);
		  else if (strncasecmp(p, "DF", 2) == 0) 
		    idx = MTYPE_To_TY(MTYPE_V16F8);
		  else if (strncasecmp(p, "SI", 2) == 0) {
		    if (num_elems == 2)
		      idx = MTYPE_To_TY(MTYPE_V8I4);
		    else if (num_elems == 4)
		      idx = MTYPE_To_TY(MTYPE_V16I4);
		  }
		  else if (strncasecmp(p, "SF", 2) == 0) {
		    if (num_elems == 2)
		      idx = MTYPE_To_TY(MTYPE_V8F4);
		    else if (num_elems == 4)
		      idx = MTYPE_To_TY(MTYPE_V16F4);
		  }
		  else if (strncasecmp(p, "HI", 2) == 0) {
		    if (num_elems == 4)
		      idx = MTYPE_To_TY(MTYPE_M8I2);
		    else if (num_elems == 8)
		      idx = MTYPE_To_TY(MTYPE_V16I2);
		  }
		  else if (strncasecmp(p, "QI", 2) == 0) {
		    if (num_elems == 8)
		      idx = MTYPE_To_TY(MTYPE_M8I1);
		    else if (num_elems == 16)
		      idx = MTYPE_To_TY(MTYPE_V16I1);
		  }
		}
		if (idx == 0)
		  Fail_FmtAssertion ("Get_TY: unexpected vector type");
                }
                break;
#endif // TARG_X8664
#if defined (TARG_ST)
        case GS_TEMPLATE_TYPE_PARM:
          break;
#endif               
#ifdef TARG_MIPS
        // MIPS paired single
        case GS_VECTOR_TYPE:
                {
		char *p = gs_type_mode(type_tree);
		idx = 0;
		if (strcmp(p, "BLK") == 0) {
		  TY_IDX elem_ty = Get_TY(gs_tree_type(type_tree));
		  TYPE_ID elem_mtype = TY_mtype(elem_ty);
		  switch (gs_n(gs_type_precision(type_tree))) {
		    case 2: if (elem_mtype == MTYPE_I4)
			      idx = MTYPE_To_TY(MTYPE_V8I4);
			    else if (elem_mtype == MTYPE_F4)
		    	      idx = MTYPE_To_TY(MTYPE_V8F4);
			    break;
		    default:
		      Fail_FmtAssertion ("Get_TY: unexpected vector type element count");
		  }
		}
		else { // use string emcoded in TYPE_MODE
		  if (toupper(*p++) != 'V') {
		    if (gs_type_name(type_tree)) {
		      p = gs_identifier_pointer(gs_decl_name(gs_type_name(type_tree)));
		      if (toupper(*p++) != 'V') 
			Fail_FmtAssertion("Get_TY: NYI");
		    }
		    else Fail_FmtAssertion("Get_TY: NYI");
		  }
		  int num_elems = strtol(p, &p, 10);
		  if (strncasecmp(p, "SI", 2) == 0) {
		    if (num_elems == 2)
		      idx = MTYPE_To_TY(MTYPE_V8I4);
		  }
		  else if (strncasecmp(p, "SF", 2) == 0) {
		    if (num_elems == 2)
		      idx = MTYPE_To_TY(MTYPE_V8F4);
		  }
		}
		if (idx == 0)
		  Fail_FmtAssertion ("Get_TY: unexpected vector type");
                }
                break;
#endif // TARG_MIPS
	default:
		FmtAssert(FALSE, ("Get_TY unexpected tree_type"));
	}
#ifdef TARG_ST
        /* (cbr) */
        if (gs_type_nothrow_p (type_tree))
          Set_TY_is_nothrow (idx);
#endif
	if (gs_type_readonly(type_tree))
		Set_TY_is_const (idx);
	if (gs_type_volatile(type_tree))
		Set_TY_is_volatile (idx);
#ifdef TARG_ST
	// [CG]: Fixup volatility for structure fields.
	fixup_volatility(idx);
	// (cbr) handle may_alias attribute
	if (lookup_attribute ("may_alias", gs_type_attributes (type_tree))) {
	  // [CL] don't apply the attribute to the predefined type
	  idx = Copy_TY(idx);
	  Set_TY_no_ansi_alias(idx);
	}
#endif
#ifdef KEY
	if (gs_type_restrict(type_tree))
		Set_TY_is_restrict (idx);
#endif
	TYPE_TY_IDX(type_tree) = idx;
        if(Debug_Level >= 2) {
#ifdef KEY
	  // DSTs for records were entered into the defer list in the order
	  // that the records are declared, in order to preserve their scope.
	  // Bug 4168.
	  if (gs_tree_code(type_tree) != GS_RECORD_TYPE &&
	      gs_tree_code(type_tree) != GS_UNION_TYPE &&
	      // Bugs 8346, 11819: Insert a TY for DST processing only
	      // when the TY is complete to ensure that when the DST info
	      // are created, the TY will be valid.
	      !TY_is_incomplete(idx) &&
	      !(TY_kind(idx) == KIND_POINTER &&
	        TY_is_incomplete(TY_pointed(idx)))) {
	    // Defer creating DST info until there are no partially constructed
	    // types, in order to prevent Create_DST_type_For_Tree from calling
	    // Get_TY, which in turn may use field IDs from partially created
	    // structs.  Such fields IDs are wrong.  Bug 5658.
	     defer_DST_type(type_tree, idx, orig_idx);

            //bug 14715: add deferred dst types so far after function type creation 
            if(gs_tree_code(type_tree)==GS_FUNCTION_TYPE ||
               gs_tree_code(type_tree)==GS_METHOD_TYPE)
                add_deferred_DST_types(); 
	  }
#else
          DST_INFO_IDX dst =
            Create_DST_type_For_Tree(type_tree,
              idx,orig_idx);
          TYPE_DST_IDX(type_tree) = dst;
#endif
        }

	return idx;
}

void 
Create_DST_For_Tree (gs_t decl_node, ST* st)
{
  DST_INFO_IDX dst =
    Create_DST_decl_For_Tree(decl_node,st);
  DECL_DST_IDX(decl_node) = dst; 
  return;
}

#if 1 // wgen
// if there is a PARM_DECL with the same name (as opposed to same node), 
// use the ST created for it
ST *
Search_decl_arguments(char *name)
{
  gs_t p;
  if (name) {
    for (p = decl_arguments; p; p = gs_tree_chain(p)) {
      if (gs_decl_name(p) == NULL) // matches with any parm with null name (sanity32.C)
	return DECL_ST(p);
      if (strcmp(name, (char *) gs_identifier_pointer(gs_decl_name(p))) == 0)
	return DECL_ST(p);
    }
  }
  else { // search for an argument with no name
    for (p = decl_arguments; p; p = gs_tree_chain(p)) {
      if (gs_decl_name(p) == NULL)
	return DECL_ST(p);
    }
  }
  return NULL;
}
#endif

#ifdef KEY // bug 12668
static BOOL
Has_label_decl(gs_t init)
{
  if (gs_tree_code(init) == GS_LABEL_DECL)
    return TRUE;
  if (gs_tree_code(init) == GS_ADDR_EXPR)
    return Has_label_decl(gs_tree_operand(init,0));
#ifdef FE_GNU_4_2_0 // bug 12699
  if (gs_tree_code(init) == GS_NOP_EXPR)
    return Has_label_decl(gs_tree_operand(init,0));
#endif
  if (gs_tree_code(init) == GS_CONSTRUCTOR) {
#ifdef FE_GNU_4_2_0
    INT length = gs_constructor_length(init);
    gs_t element_value;
    for (INT idx = 0; idx < length; idx++) {
      element_value = gs_constructor_elts_value(init, idx);
      if (Has_label_decl(element_value))
	return TRUE;
    }
#else
    gs_t nd;
    for (nd = gs_constructor_elts(init); nd; nd = gs_tree_chain(nd)) {
      if (Has_label_decl(gs_tree_value(nd)))
	return TRUE;
    }
#endif
  }
  return FALSE;
}
#endif
#ifdef TARG_ST
/** 
  *  This function resets attributes based "properties" on variable.
  * it is used by Get_ST(). It is run at each call of get_st to
  * guarantee proper properties in case of forward declarations.
  * 
  * @param decl_node 
  */
void
set_variable_attributes(gs_t decl_node, ST *st)
  {
    TY_IDX ty_idx = Get_TY(gs_tree_type(decl_node));
    if (gs_decl_user_align (gs_tree_type(decl_node))) {
      UINT align = gs_decl_align_unit(decl_node);
      if (align) {
        Set_TY_align (ty_idx, align);
        Set_ST_type (st, ty_idx);
      }
    }
    
    if (gs_decl_user_align (decl_node)) {
      UINT align = gs_decl_align_unit(decl_node);
      if (align) {
        Set_TY_align (ty_idx, align);
        Set_ST_type (st, ty_idx);
      }
    }
#ifdef TARG_STxP70
    // (cbr) memory_space support
    gs_t attr;
    ST_MEMORY_SPACE kind=ST_MEMORY_DEFAULT;
    attr = lookup_attribute ("memory", gs_decl_attributes (decl_node));
    if (attr) {
      attr = gs_tree_value (gs_tree_value (attr));
      FmtAssert (gs_tree_code (attr) == GS_STRING_CST, ("Malformed memory attribute"));
      if (!strcmp (gs_tree_string_pointer (attr), "da")) {
        kind = ST_MEMORY_DA;
      } else if (!strcmp (gs_tree_string_pointer (attr), "sda")) {
        kind = ST_MEMORY_SDA;
      } else if (!strcmp (gs_tree_string_pointer (attr), "tda")) {
        kind = ST_MEMORY_TDA;
      } else if (!strcmp (gs_tree_string_pointer (attr), "none")) {
        kind = ST_MEMORY_NONE;
      } else {
        FmtAssert (FALSE, ("Malformed tls_model attribute"));
      }
    } 
    Set_ST_memory_space (*st, kind);
#endif        
    if (gs_decl_section_name (decl_node) && // 
	!ST_has_named_section (st)) { // [TTh] Insure that attributes are updated
                                      // only once to avoid multiple memory
                                      // allocation for same variable
      SYMTAB_IDX level = ST_level(st);
      if (gs_tree_code (decl_node) == GS_FUNCTION_DECL)
        level = GLOBAL_SYMTAB;
      ST_ATTR_IDX st_attr_idx;
      ST_ATTR&    st_attr = New_ST_ATTR (level, st_attr_idx);
      ST_ATTR_Init (st_attr, ST_st_idx (st), ST_ATTR_SECTION_NAME,
                    Save_Str (gs_tree_string_pointer (gs_decl_section_name (decl_node))));
      Set_ST_has_named_section (st);
    }
  }
#endif  // TARG_ST

ST*
Create_ST_For_Tree (gs_t decl_node)
{
  TY_IDX     ty_idx;
  ST*        st = NULL;
  char      *name;
  char	    tempname[32];
  ST_SCLASS  sclass;
  ST_EXPORT  eclass;
  SYMTAB_IDX level;
  static INT anon_count = 0;
  BOOL anon_st = FALSE;

#ifdef TARG_ST
  // If the decl is a weakref to another decl, then return the ST
  // for the other decl.
  // But we must indicate that this is a weakref, to ensure that
  // the symbol is marked weak if all references are weak.
  if (lookup_attribute ("weakref", gs_decl_attributes (decl_node))) {
    gs_t target = gs_decl_alias_target(decl_node);
    FmtAssert (target != NULL, ("Missing alias target for weakref"));
    return Get_ST (target);
  }
#endif

  // If the decl is a function decl, and there are duplicate decls for the
  // function, then use a ST already allocated for the function, if such ST
  // exists.
  if (gs_tree_code(decl_node) == GS_FUNCTION_DECL) {
    st = get_duplicate_st (decl_node);
    if (st) {
      set_DECL_ST(decl_node, st);
      return st;
    }
  }

  // For variables with asm register assignments, don't use the assembler
  // names because they are of the form "%rbx".
  if (gs_tree_code(decl_node) == GS_RESULT_DECL) {
    sprintf(tempname, ".result_decl_%d", gs_decl_uid(decl_node));
    name = tempname;
    anon_st = TRUE;
  }
  else if ((gs_tree_code(decl_node) == GS_FUNCTION_DECL ||
            gs_tree_code(decl_node) == GS_PARM_DECL ||
            (gs_tree_code(decl_node) == GS_VAR_DECL &&
             gs_decl_asmreg(decl_node) != -1)) &&
           gs_decl_name(decl_node) != 0)
    name = (char *) gs_identifier_pointer (gs_decl_name (decl_node));
  else if (gs_decl_name (decl_node) && gs_decl_assembler_name (decl_node))
    name = (char *) gs_identifier_pointer (gs_decl_assembler_name (decl_node));
  else if (!lang_cplus && gs_decl_name (decl_node))
    name = (char *) gs_identifier_pointer (gs_decl_name (decl_node));
#ifdef TARG_ST
  else if (gs_decl_name (decl_node)) {
    name = (char *) gs_identifier_pointer (gs_decl_name (decl_node));
  }
#endif
  else {
#ifdef TARG_ST
    /* (cbr) avoid clash with user naming */
    sprintf(tempname, ".anon%d", ++anon_count);
#else
    sprintf(tempname, "anon%d", ++anon_count);
#endif
    name = tempname;
    anon_st = TRUE;
  }

#ifdef KEY
  BOOL guard_var = FALSE;
  // See if variable is a guard variable.
  if (strncmp("_ZGV", name, 4) == 0) {
    guard_var = TRUE;
  }
#endif

  switch (gs_tree_code(decl_node)) {

    case GS_FUNCTION_DECL:
      {
        if (Enable_WFE_DFE) {
          gs_t body = gs_decl_saved_tree(decl_node);
          if (gs_decl_thunk_p(decl_node) &&
              gs_tree_code(gs_cp_decl_context(decl_node)) != GS_NAMESPACE_DECL)
            Push_Deferred_Function (decl_node);
/*
          else
          if (DECL_TINFO_FN_P(decl_node))
            Push_Deferred_Function (decl_node);
*/
          else
          if (body != NULL && !gs_decl_external(decl_node) &&
              (gs_decl_template_info(decl_node) == NULL              ||
               gs_decl_friend_pseudo_template_instantiation(decl_node) ||
               gs_decl_template_instantiated(decl_node)              ||
               gs_decl_template_specialization(decl_node))) {
            Push_Deferred_Function (decl_node);
          }
        }

#ifdef KEY /* bug 8346 */
        Is_True (!processing_function_prototype,
                 ("Create_ST_For_Tree: processing another function prototype?"));
        processing_function_prototype = TRUE;
        TY_IDX func_ty_idx = Get_TY(gs_tree_type(decl_node));
        processing_function_prototype = FALSE;
#else
        TY_IDX func_ty_idx = Get_TY(gs_tree_type(decl_node));
#endif

        sclass = SCLASS_EXTERN;
#ifdef TARG_ST
	eclass = Get_Export_Class_For_Tree(decl_node, CLASS_FUNC, sclass);
	if (gs_decl_context (decl_node)
	    && gs_tree_code (gs_decl_context (decl_node)) == GS_FUNCTION_DECL) {
	  // Nested function.
	  level = PU_lexical_level (Get_ST (gs_decl_context (decl_node))) +1;
	} else {
	  level = GLOBAL_SYMTAB + 1;
	}
#else
        eclass = gs_tree_public(decl_node) || gs_decl_weak(decl_node) ?
		   EXPORT_PREEMPTIBLE				:
		   EXPORT_LOCAL;
#if 0	// no longer putting the ST in the local symbol table of the parent
	if (gs_decl_context(decl_node) != NULL &&
	    gs_tree_code(gs_decl_context(decl_node)) == GS_FUNCTION_DECL)
	  level  = CURRENT_SYMTAB; // a nested function
	else
#endif
        level  = GLOBAL_SYMTAB+1;
#endif
        PU_IDX pu_idx;
        PU&    pu = New_PU (pu_idx);

        PU_Init (pu, func_ty_idx, level);

#if defined KEY && ! defined TARG_ST
	// [SC] Put nested functions in the global symbol table.
        st = New_ST (level - 1);
#else
        st = New_ST (GLOBAL_SYMTAB);
#endif

        // Fix bug # 34, 3356
// gcc sometimes adds a '*' and itself handles it this way while outputing
	char *p;
	if (gs_decl_assembler_name(decl_node) == NULL)
	  p = name;
	else p  = gs_identifier_pointer (gs_decl_assembler_name (decl_node));
	if (*p == '*')
	  p++;
        ST_Init (st, Save_Str(p),
                 CLASS_FUNC, sclass, eclass, TY_IDX (pu_idx));
	if (gs_decl_thunk_p(decl_node) &&
            gs_tree_code(gs_cp_decl_context(decl_node)) != GS_NAMESPACE_DECL &&
	    eclass != EXPORT_LOCAL &&
	    eclass != EXPORT_LOCAL_INTERNAL)
	  Set_ST_is_weak_symbol(st);
#ifdef TARG_ST
        /* (cbr) allow writable const in here */
	if (gs_decl_name (decl_node)
	    && ! strncmp (gs_identifier_pointer (gs_decl_name (decl_node)),
			  "__static_initialization_and_destruction", 39)) {
          Set_PU_static_initializer (pu);          
	}

	if (gs_decl_uninlinable (decl_node)) {
	  Set_PU_no_inline (pu);
	}
          /* (cbr) handle used attribute */
	if (lookup_attribute ("used", gs_decl_attributes(decl_node))) {
	  Set_PU_no_delete (Pu_Table [ST_pu (st)]);            
	  Set_ST_is_used(st);
	}
	/* [TB] handle optimization function level attributes */
	gs_t attr = lookup_attribute("optsize", gs_decl_attributes(decl_node));
	PU_OPTLEVEL optsize = 
	  (PU_OPTLEVEL) (attr ? gs_n(gs_tree_value(attr)) : PU_OPTLEVEL_UNDEF);
        attr = lookup_attribute("optperf", gs_decl_attributes(decl_node));
			      
	Set_PU_size_opt (Pu_Table [ST_pu (st)], optsize);
	PU_OPTLEVEL optperf = 
	  (PU_OPTLEVEL) (attr ? gs_n(gs_tree_value(attr)) : PU_OPTLEVEL_UNDEF);
// 	Set_PU_perf_opt (Pu_Table [ST_pu (st)], optperf);
#endif

      }
      break;

#ifdef KEY
    case GS_RESULT_DECL: // bug 3878
#if 0
    // wgen clean-up: These codes, needed to handle gimplified GNU tree
       should not be needed any more.
      if (TY_return_in_mem
          (Get_TY
           (gs_tree_type
            (gs_tree_type
             (Current_Function_Decl()) ) ) ) )
      {
        // We should have already set up the first formal for holding
        // the return object.
        WN *first_formal = WN_formal(Current_Entry_WN(), 0);
        if (!get_DECL_ST(decl_node))
          set_DECL_ST(decl_node, WN_st(first_formal));
        return get_DECL_ST(decl_node);
      }
      // fall through
#endif
#endif
    case GS_PARM_DECL:
    case GS_VAR_DECL:
      {
        if (gs_tree_code(decl_node) == GS_PARM_DECL) {
#ifdef KEY
	  // wgen fix for C++ and also for C, as in bug 8346.
	  if (decl_arguments) {
	    st = Search_decl_arguments(gs_decl_name(decl_node) ? name : NULL);
	    if (st) {
	      set_DECL_ST(decl_node, st); // created now
	      return st;
	    }
	  }
#endif
          sclass = SCLASS_FORMAL;
          eclass = EXPORT_LOCAL;
          level = CURRENT_SYMTAB;
        }
        else {
          if (gs_decl_context (decl_node) == 0 			     ||
	      gs_tree_code (gs_decl_context (decl_node)) == GS_NAMESPACE_DECL ||
 	      gs_tree_code (gs_decl_context (decl_node)) == GS_RECORD_TYPE ) {
#ifdef TARG_ST
	    if (gs_decl_register(decl_node)) {
	      sclass = SCLASS_FSTATIC;
	      eclass = Get_Export_Class_For_Tree(decl_node, CLASS_VAR, sclass);
	    } else
#endif
            if (gs_tree_public (decl_node)) {
	      // GCC 3.2
	      if (gs_decl_external(decl_node) ||
		  (gs_decl_lang_specific(decl_node) &&
		   gs_decl_really_extern(decl_node)))
		sclass = SCLASS_EXTERN;
	      else
	      if (gs_decl_initial(decl_node))
#ifdef TARG_ST
                /* (cbr) fix */
		sclass = SCLASS_DGLOBAL;
#else
		sclass = SCLASS_UGLOBAL;
#endif
	      else if (gs_tree_static(decl_node)) {
#ifdef KEY
// bugs 340, 3717
		if (flag_no_common || !gs_decl_common (decl_node) ||
		    (!lang_cplus /* bug 14187 */ &&
		     gs_decl_section_name (decl_node) /* bug 14181 */))
#else
		if (flag_no_common)
#endif
		  sclass = SCLASS_UGLOBAL;
		else
		  sclass = SCLASS_COMMON;
	      }
	      else
              	sclass = SCLASS_EXTERN;
	      eclass = Get_Export_Class_For_Tree(decl_node, CLASS_VAR, sclass);
            }
            else {
              	sclass = SCLASS_FSTATIC;
		eclass = Get_Export_Class_For_Tree(decl_node, CLASS_VAR, sclass);
            }
            level = GLOBAL_SYMTAB;
          }
          else {
#ifdef KEY
	    // .gnu.linkonce.b is .bss with DECL_ONE_ONLY set.  Bug 10876.
	    gs_t section_name = gs_decl_section_name(decl_node);
	    if (section_name &&
		!strncmp(gs_tree_string_pointer(section_name),
			 ".gnu.linkonce.", 14)) {
	      if (!strncmp(gs_tree_string_pointer(section_name),
			   ".gnu.linkonce.b.", 16)
	          // bug 13054
	          || !strncmp(gs_tree_string_pointer(section_name),
	                      ".gnu.linkonce.sb.", 17)) {
		sclass = SCLASS_UGLOBAL;
		level  = GLOBAL_SYMTAB;
		eclass = Get_Export_Class_For_Tree(decl_node, CLASS_VAR, sclass);
#ifdef TARG_ST
	      } else if (!strncmp(gs_tree_string_pointer(section_name),
				  ".gnu.linkonce.d.", 16)
			 || strncmp(gs_tree_string_pointer(section_name),
				    ".gnu.linkonce.sd.", 17)) {
		sclass = SCLASS_DGLOBAL;
		level = GLOBAL_SYMTAB;
		eclass = Get_Export_Class_For_Tree(decl_node, CLASS_VAR, sclass);
#endif

	      } else {
		// Add support as needed.
		Fail_FmtAssertion("Create_ST_For_Tree: %s section NYI",
				  gs_tree_string_pointer(section_name));
	      }
	    }
	    // bug 13090 and 13245
	    // Bug 13047 shows that the gnu42 front-end (specifically
	    // the gcc/g++ part) behaves differently when built on a gnu3
	    // system, than when built on a gnu4 system. If the compiler
	    // is built on a gnu4 system, default_unique_section() in
	    // varasm.c will never generate a linkonce section because
	    // starting GNU42, this also depends on whether the host
	    // compiling system has COMDAT groups.
	    else if (section_name &&
	             (!strncmp(gs_tree_string_pointer(section_name),
			       ".sbss.", 6) ||
		      !strncmp(gs_tree_string_pointer(section_name),
			       ".bss.", 5))) {
	      sclass = SCLASS_UGLOBAL;
	      level  = GLOBAL_SYMTAB;
	      eclass = Get_Export_Class_For_Tree(decl_node, CLASS_VAR, sclass);
	    }
	    else
#endif
#ifdef TARG_ST
                /* (cbr) static data member or static data in function member 
                   has external linkage */
            if (gs_decl_context (decl_node) && gs_tree_static (decl_node) &&
                gs_type_needs_constructing (gs_tree_type (decl_node)) &&
                gs_decl_comdat (gs_decl_context (decl_node))) {
              gs_t init = gs_decl_initial (decl_node);
              // (cbr) will be constructed at runtime. reserve space only
              if (!init)
                sclass = SCLASS_COMMON;
              else
                sclass = SCLASS_EXTERN;                

	      level  = GLOBAL_SYMTAB;
	      eclass = Get_Export_Class_For_Tree(decl_node, CLASS_VAR, sclass);
            }
            else
#endif
            if (gs_decl_external(decl_node) || gs_decl_weak (decl_node)) {
	      sclass = SCLASS_EXTERN;
	      level  = GLOBAL_SYMTAB;
	      eclass = Get_Export_Class_For_Tree(decl_node, CLASS_VAR, sclass);
            }
#ifdef KEY
	    // Bug 8652: If GNU marks it as COMMON, we should the same.
	    else if (!flag_no_common && gs_tree_static (decl_node) &&
	             gs_decl_common (decl_node)) {
	      sclass = SCLASS_COMMON;
	      level = GLOBAL_SYMTAB;
	      eclass = Get_Export_Class_For_Tree(decl_node, CLASS_VAR, sclass);
	    }
#endif
            else {
	      if (gs_tree_static (decl_node)) {
		sclass = SCLASS_PSTATIC;
		if (pstatic_as_global
#ifdef KEY // bug 12668
		    && ! (gs_decl_initial(decl_node) && 
		    	  !gs_decl_external(decl_node) &&
			  Has_label_decl(gs_decl_initial(decl_node)))
#endif
		   )
			level = GLOBAL_SYMTAB;
		else
			level = CURRENT_SYMTAB;
              }
              else {
		sclass = SCLASS_AUTO;
#ifdef TARG_ST
		FmtAssert (gs_decl_context (decl_node),
			   ("Missing decl_context for auto variable"));
		level = PU_lexical_level (Get_ST (gs_decl_context (decl_node)));
#else
	 	level = DECL_SYMTAB_IDX(decl_node) ?
			DECL_SYMTAB_IDX(decl_node) : CURRENT_SYMTAB;
#endif
              }
              eclass = EXPORT_LOCAL;
            }
          }
        }

	gs_t attr;
	// Make g++ guard variables global in order to make them weak.  Ideally
	// guard variables should be "common", but for some reason the back-end
	// currently can't handle C++ commons.  As a work around, make the
	// guard variables weak.  Since symtab_verify.cxx don't like weak
	// locals, make the guard variables global.
	if (guard_var) {
	  level = GLOBAL_SYMTAB;
	  sclass = SCLASS_UGLOBAL;
	  eclass = EXPORT_PREEMPTIBLE;
	}
#ifdef KEY
	// bug 14967
	else if (eclass == EXPORT_PREEMPTIBLE &&
	         (attr = lookup_attribute ("visibility",
	                                   gs_decl_attributes(decl_node)))) {
	  gs_t value = gs_tree_value (attr);
	  Is_True (gs_tree_code(value) == GS_TREE_LIST, ("Expected TREE_LIST"));
	  value = gs_tree_value (value);
	  if (gs_tree_code(value) == GS_STRING_CST) {
	    const char *visibility = gs_tree_string_pointer(value);
	    if (!strcmp (visibility, "internal"))
	      eclass = EXPORT_INTERNAL;
	    else if (!strcmp (visibility, "hidden"))
	      eclass = EXPORT_HIDDEN;
	    else if (!strcmp (visibility, "protected"))
	      eclass = EXPORT_PROTECTED;
	  }
	}
#endif


	// The tree under DECL_ARG_TYPE(decl_node) could reference decl_node.
	// If that's the case, the Get_TY would create the ST for decl_node.
	// As a result, call GET_TY first, then check if the ST is already
	// created, and create ST only if it isn't created.
        ty_idx = Get_TY (gs_tree_type(decl_node));
	st = DECL_ST(decl_node);
	if (st)
	  return st;
        st = New_ST (level);
        if (TY_kind (ty_idx) == KIND_ARRAY &&
            gs_tree_static (decl_node) &&
            gs_decl_initial (decl_node) == FALSE &&
            TY_size (ty_idx) == 0) {
	  Set_TY_size (ty_idx, TY_size (Get_TY (gs_tree_type (gs_tree_type (decl_node)))));
        }
#if !defined KEY || defined TARG_ST
// bug 3735: the compiler cannot arbitrarily change the alignment of
// individual structures
	if (TY_mtype (ty_idx) == MTYPE_M &&
#ifdef TARG_ST
            // [SC] Alignment of parameters is fixed by the ABI, and cannot
            // be changed by Aggregate_Alignment.
            gs_tree_code(decl_node) != GS_PARM_DECL &&
#endif
	    Aggregate_Alignment > 0 &&
	    Aggregate_Alignment > TY_align (ty_idx))
	  Set_TY_align (ty_idx, Aggregate_Alignment);
#endif // !KEY
#if defined (TARG_ST)
        /* (cbr) pass it through memory */
        if (gs_tree_code(decl_node) == GS_PARM_DECL &&
            gs_tree_addressable(gs_tree_type(decl_node))) {
          ty_idx = Make_Pointer_Type(ty_idx);
        }
#endif
	// qualifiers are set on decl nodes
	if (gs_tree_readonly(decl_node))
		Set_TY_is_const (ty_idx);
	if (gs_tree_this_volatile(decl_node))
		Set_TY_is_volatile (ty_idx);
#if 1 // wgen bug 10470
	else Clear_TY_is_volatile (ty_idx);
#endif
#ifdef KEY
        // Handle aligned attribute (bug 7331)
#ifdef TARG_ST
	if (gs_type_user_align (gs_tree_type(decl_node)))
	  Set_TY_align (ty_idx, gs_type_align(gs_tree_type(decl_node))/BITSPERBYTE);
#endif
        if (gs_decl_user_align (decl_node))
          Set_TY_align (ty_idx, gs_decl_align_unit(decl_node));
        // NOTE: we do not update the ty_idx value in the TYPE_TREE. So
        // if any of the above properties are set, the next time we get into
        // Get_ST, the ty_idx in the TYPE_TREE != ty_idx in st. The solution
        // is either to update TYPE_TREE now, or compare the ty_idx_index
        // in Get_ST (instead of ty_idx). Currently we do the latter
#endif // KEY
#ifdef TARG_ST
	if (gs_type_restrict(gs_tree_type(decl_node)))
	  Set_TY_is_restrict (ty_idx);
#endif
	// bug 34 3356 10892: gcc sometimes adds a '*' and itself handles it 
	//   this way while outputing
	char *p = name;
	if (*p == '*')
	  p++;
        ST_Init (st, Save_Str(p), CLASS_VAR, sclass, eclass, ty_idx);
#ifdef KEY
#ifdef FE_GNU_4_2_0
#ifdef TARG_ST
	if (gs_tree_code (decl_node) == GS_VAR_DECL &&
	    gs_decl_thread_local (decl_node)) {
	  int gcc_tls_model = gs_n(gs_decl_tls_model (decl_node));
	  // Translate from gcc enum tls_model to open64 enum ST_TLS_MODEL.
	  static const enum ST_TLS_MODEL o64_tls_model[] = {
	    ST_TLS_MODEL_GLOBAL_DYNAMIC,
	    ST_TLS_MODEL_GLOBAL_DYNAMIC,
	    ST_TLS_MODEL_LOCAL_DYNAMIC,
	    ST_TLS_MODEL_INITIAL_EXEC,
	    ST_TLS_MODEL_LOCAL_EXEC };
	  Set_ST_is_thread_private (st);
	  Set_ST_tls_model (*st, o64_tls_model[gcc_tls_model]);
	}
#else
	if (gs_tree_code (decl_node) == GS_VAR_DECL &&
	    // Bug 12968: just checking for threadprivate flag is not
	    // sufficient, because for C the flag is basically
	    // gs_decl_lang_flag_3.
	    gs_decl_thread_local (decl_node) &&
	    ((!lang_cplus && gs_c_decl_threadprivate_p (decl_node)) ||
	     (lang_cplus && gs_cp_decl_threadprivate_p (decl_node))))
	  Set_ST_is_thread_private (st);
#endif
	// bug 15192: automatic variables in parallel scope may not always
	// have anon name, but they still need to be marked private.
	if (gs_tree_code (decl_node) == GS_VAR_DECL &&
	    (sclass == SCLASS_AUTO || anon_st))
	  WGEN_add_pragma_to_enclosing_regions (WN_PRAGMA_LOCAL, st);
#endif

        if (gs_decl_size_unit (decl_node) &&
            gs_tree_code (gs_decl_size_unit (decl_node)) != GS_INTEGER_CST)
        {
            // if this is the first alloca, save sp.
            int idx;
            if (!Set_Current_Scope_Has_Alloca (idx))
            {
              ST * save_st = WGEN_Alloca_0 ();
              Set_Current_Scope_Alloca_St (save_st, idx);
            }
            WN * size = WGEN_Expand_Expr (gs_decl_size_unit (decl_node));
            // mimic WGEN_Alloca_ST
            ST * alloca_st = New_ST (CURRENT_SYMTAB);
            ST_Init (alloca_st, Save_Str (name),
                       CLASS_VAR, SCLASS_AUTO, EXPORT_LOCAL,
                                  Make_Pointer_Type (ty_idx, FALSE));
            Set_ST_is_temp_var (alloca_st);
            Set_ST_pt_to_unique_mem (alloca_st);
            Set_ST_base_idx (st, ST_st_idx (alloca_st));
            WN *wn  = WN_CreateAlloca (size);
            wn = WN_Stid (Pointer_Mtype, 0, alloca_st, ST_type (alloca_st), wn);
            WGEN_Stmt_Append (wn, Get_Srcpos());
            Set_PU_has_alloca (Get_Current_PU());
	    // For kids 1..n of DEALLOCA
            Add_Current_Scope_Alloca_St (alloca_st, idx);
        }
#endif // KEY
        if (gs_tree_code(decl_node) == GS_PARM_DECL) {
		Set_ST_is_value_parm(st);
        }
#ifdef TARG_ST
        /* (cbr) mark readonly vars const */
        if (sclass != SCLASS_AUTO && 
            gs_tree_code(decl_node) == GS_VAR_DECL &&
            (gs_tree_code(gs_tree_type(decl_node)) != GS_REFERENCE_TYPE &&
             !gs_type_needs_constructing (gs_tree_type (decl_node))) &&
            gs_type_readonly(decl_node) &&
	    gs_decl_initial (decl_node) &&
	    gs_tree_constant (gs_decl_initial (decl_node)) &&
	    ! gs_tree_side_effects (decl_node))
          Set_ST_is_const_var (st);

	/* [CM] also mark readonly linkonce const (fix bug #28227) 
	   See also osprey/gccfe/gnu/varasm.c/categorize_decl_for_section()
	 */
	if (sclass == SCLASS_DGLOBAL && 
	    gs_decl_one_only (decl_node) && 
	    gs_type_readonly(decl_node) &&
	    !gs_tree_side_effects (decl_node) &&
	    gs_decl_initial (decl_node) &&
	    gs_tree_constant (gs_decl_initial (decl_node)))
	  Set_ST_is_const_var (st);

	// [CL] handle used attribute
	if (sclass != SCLASS_AUTO && 
            gs_tree_code(decl_node) == GS_VAR_DECL &&
	    lookup_attribute ("used", gs_decl_attributes(decl_node))) {
	  Set_ST_is_used(st);
	}
#endif
      }
      break; 
    default:
      {
        Fail_FmtAssertion ("Create_ST_For_Tree: unexpected tree type %s",
			   WGEN_Tree_Node_Name(decl_node));
      }
      break;
  }

  set_DECL_ST(decl_node, st); // created now

  // If VAR_DECL has a non-zero DECL_ASMREG, then DECL_ASMREG-1 is the register
  // number assigned by an "asm".
  if (gs_tree_code(decl_node) == GS_VAR_DECL && gs_decl_register(decl_node) &&
      gs_decl_asmreg(decl_node) != -1) {
#ifndef TARG_ST
    // [SC] For ST, Map_Reg_To_Preg is declared in fe_loader.h
    extern PREG_NUM Map_Reg_To_Preg []; // defined in common/com/arch/config_targ.cxx
#endif
    int reg = gs_decl_asmreg(decl_node);
#ifdef TARG_ST
    PREG_NUM preg = GCCTARG_Map_Reg_To_Preg()[reg];
#else
    PREG_NUM preg = Map_Reg_To_Preg [reg];
#endif
    FmtAssert (preg >= 0,
               ("mapping register %d to preg failed\n", reg));
    TY_IDX ty_idx = ST_type (st);
    Set_TY_is_volatile (ty_idx);
    Set_ST_type (st, ty_idx);
    Set_ST_assigned_to_dedicated_preg (st);
    ST_ATTR_IDX st_attr_idx;
    ST_ATTR&    st_attr = New_ST_ATTR (level, st_attr_idx);
    ST_ATTR_Init (st_attr, ST_st_idx (st), ST_ATTR_DEDICATED_REGISTER, preg);
  }

  if (gs_tree_code(decl_node) == GS_VAR_DECL) {
    if (gs_decl_context(decl_node) &&
	gs_tree_code(gs_decl_context(decl_node)) == GS_RECORD_TYPE) {
      Get_TY(gs_decl_context(decl_node));
    }
    if (gs_decl_thread_local(decl_node)
#ifdef FE_GNU_4_2_0
        // Bug 12891: threadprivate variables are also marked thread-local
        // by GNU, but we don't want to tell our backend such variables are
        // thread-local.
        &&  ((!lang_cplus && !gs_c_decl_threadprivate_p(decl_node)) ||
             (lang_cplus && !gs_cp_decl_threadprivate_p(decl_node)))
#endif
       ) {
      Set_ST_is_thread_local(st);
      st = Trans_TLS(decl_node, st);
    }
  }

  if (Enable_WFE_DFE) {
#ifdef TARG_ST
    /* (cbr) ? */
    if (gs_tree_code(decl_node) == GS_FUNCTION_DECL &&
#else
    if (gs_tree_code(decl_node) == GS_VAR_DECL &&
#endif
        level == GLOBAL_SYMTAB &&
        !gs_decl_external (decl_node) &&
        gs_decl_initial (decl_node)) {
      Push_Deferred_Function (decl_node);
    }
  }

#ifdef TARG_STxP70
    // (cbr) memory_space support
    gs_t attr;
    ST_MEMORY_SPACE kind=ST_MEMORY_DEFAULT;
    attr = lookup_attribute ("memory", gs_decl_attributes (decl_node));
    if (attr) {
      attr = gs_tree_value (gs_tree_value (attr));
      FmtAssert (gs_tree_code (attr) == GS_STRING_CST, ("Malformed memory attribute"));
      if (!strcmp (gs_tree_string_pointer (attr), "da"))
	kind = ST_MEMORY_DA;
      else if (!strcmp (gs_tree_string_pointer (attr), "sda"))
	kind = ST_MEMORY_SDA;
      else if (!strcmp (gs_tree_string_pointer (attr), "tda"))
	kind = ST_MEMORY_TDA;
      else if (!strcmp (gs_tree_string_pointer (attr), "none"))
	kind = ST_MEMORY_NONE;
      else
	FmtAssert (FALSE, ("Malformed memory attribute"));
    } 
    Set_ST_memory_space (*st, kind);
#endif
  if (gs_decl_weak      (decl_node) &&
      (!gs_decl_external (decl_node)
#ifdef KEY
       // Make weak symbols for:
       //   extern "C" int bar() __attribute__ ((weak, alias("foo")))
       // Bug 3841.
       || gs_decl_alias_target(decl_node))
#endif
      ) {
    Set_ST_is_weak_symbol (st);
  }

#ifdef KEY
  // Make all symbols referenced in cleanup code and try handler code weak.
  // This is to work around an implementation issue where kg++fe always emit
  // the code in a cleanup or try handler, regardless of whether such code is
  // emitted by g++.  If the code calls a function foo that isn't emitted by
  // g++ into the RTL, then foo won't be tagged as needed, and the WHIRL for
  // foo won't be genterated.  This leads to undefined symbol at link-time.
  //
  // The correct solution is to mimick g++ and generate the cleanup/handler
  // code only if the region can generate an exception.  g++ does this in
  // except.c by checking for "(flag_non_call_exceptions ||
  // region->may_contain_throw)".  This checking isn't done in kg++fe because
  // the equivalent of "region->may_contain_throw" isn't (yet) implemented.
  // For now, work around the problem by making all symbols refereced in
  // cleanups and try handlers as weak.
  if (make_symbols_weak) {
    if (eclass != EXPORT_LOCAL &&
	eclass != EXPORT_LOCAL_INTERNAL &&
	// Don't make symbol weak if it is defined in current file.  Workaround
	// for SLES 8 linker.  Bug 3758.
	WEAK_WORKAROUND(st) != WEAK_WORKAROUND_dont_make_weak &&
	// Don't make builtin functions weak.  Bug 9534.
	!(gs_tree_code(decl_node) == GS_FUNCTION_DECL &&
#ifdef TARG_ST
         (gs_decl_built_in(decl_node)
	  // Don't make C++ runtime support functions weak (Codex bug #83532)
	  // These function are seen as the only compiler generated function 
	  // with external linkage and "C" linkage
	  // Example : all __cxa_ functions in libsupc++/libstdc++
	  // Refer to gnu/cp/decl.c::build_library_fn_1
	  || (gs_decl_artificial(decl_node) && gs_decl_extern_c_p(decl_node))))) {
#else
	  gs_decl_built_in(decl_node))) {
#endif
      Set_ST_is_weak_symbol (st);
      WEAK_WORKAROUND(st) = WEAK_WORKAROUND_made_weak;
    }
  }
  // See comment above about guard variables.
  else if (guard_var) {
    Set_ST_is_weak_symbol (st);
    Set_ST_init_value_zero (st);
    Set_ST_is_initialized (st);
  }
#endif
#ifdef TARG_ST
    /* linkage status is not yet known
    if (DECL_DEFER_OUTPUT (decl_node) &&
        DECL_INLINE (decl_node) && TREE_PUBLIC (decl_node)) {
      DECL_COMDAT (decl_node) = 1;
      DECL_ONE_ONLY (decl_node) = 1;
    }
 */

    if (gs_decl_comdat (decl_node)) {
      Set_ST_is_comdat (st);
    }

    /* (cbr) merge into linkonce */
    if (gs_decl_one_only (decl_node)) {
      if (sclass == SCLASS_COMMON)
        Set_ST_sclass (st, SCLASS_UGLOBAL);
    }
#endif

  if (gs_decl_section_name (decl_node)) {
#ifndef TARG_ST
    DevWarn ("section %s specified for %s",
             gs_tree_string_pointer (gs_decl_section_name (decl_node)),
             ST_name (st));
#endif
    if (gs_tree_code (decl_node) == GS_FUNCTION_DECL)
      level = GLOBAL_SYMTAB;
    ST_ATTR_IDX st_attr_idx;
    ST_ATTR&    st_attr = New_ST_ATTR (level, st_attr_idx);
    ST_ATTR_Init (st_attr, ST_st_idx (st), ST_ATTR_SECTION_NAME,
                  Save_Str (gs_tree_string_pointer (gs_decl_section_name (decl_node))));
    #ifdef TARG_ST
    Set_ST_has_named_section (st);
#else
    if (!lang_cplus) // bug 14187
      Set_ST_has_named_section (st);
#endif
  }
  #ifdef TARG_ST
  /* (cbr) const/pure attributes */
  /* Warning: 
     gcc pure maps to open64 no_side_effect
     gcc const maps to open64 pure
  */
  if (gs_tree_code (decl_node) == GS_FUNCTION_DECL) {
    if (gs_tree_readonly (decl_node)) {
      Set_PU_is_pure (Pu_Table [ST_pu (st)]);
    }
    if (gs_decl_is_pure (decl_node)) {
      Set_PU_no_side_effects (Pu_Table [ST_pu (st)]);
    }
  }
#endif

  if(Debug_Level >= 2) {
    // Bug 559
    if (ST_sclass(st) != SCLASS_EXTERN) {
      // Add DSTs for all types seen so far.
      add_deferred_DST_types();

      DST_INFO_IDX dst = Create_DST_decl_For_Tree(decl_node,st);
      DECL_DST_IDX(decl_node) = dst;
    }
  }

#ifdef KEY
  // Bug 11352: For C++, expand any initializations decl_node may have,
  // after the ST is fully formed. The decl may be in gxx_emitted_decls
  // list, and WGEN_Process_Var_Decl may have decided not to expand it.
  // Now that we are here, we are sure to need any initialization it
  // may have.
  if (lang_cplus && gs_tree_code(decl_node) == GS_VAR_DECL &&
      !expanded_decl(decl_node))
    WGEN_Expand_Decl(decl_node, TRUE);
#endif

  return st;
}
#ifdef TARG_ST
// [CG]
// fixup_volatility()
//
// Function that propagate volatile attributes
// to all possibly overlapping fileds in a struct/union
// where some fields are volatile.
// This function must be called at the end of the
// struct/union type creation, when all the fields
// are set up.
//
// The rational for this comes from a bug referenced
// as 1-5-0-B/ddts/18793.
// The problem is that in WOPT SSA form, the 
// volatile accesses are not considered at all
// in memory dependency chains.
// The consequence of this is:
//   If two memory access overlaps and one and only
//   one of them is marked volatile, the DSE/SPRE/DCE
//   in WOPT will generate unexpected code.
// This can appear when an union has some fields marked
// volatile and the other not.
// The solutions to this are:
// 1. Reengineer the WOPT SSA form to handle this case.
// 2. Fix DSE/SPRE/DCE optimizations to not remove stores
// that access aggregates where a member is volatile. It
// seems sufficient for the original problem, but not 
// terribly scalable.
// 3. Arrange the WHIRL such that overlapping fields in
// unions are all marked volatile and have the front-end
// generate volatile access for all accesses to this 
// volatile fileds.
//
// I choose to implement solution 3 as it is the most
// straightforward and the most localized changed.
//
// The solution is implemented in two steps:
// 1. The fixup_volatility() in tree_symtab.cxx sets the
// volatile flag for the fields of a structure that may
// overlap some other volatile field.
// 2. The function_update_volatile_accesses() in 
// wfe_decl.cxx arrange for having all memory accesses
// to a volatile field to be marked volatile. The reason
// for this step is that the translator inspects the GCC
// tree for marking volatility, not the field type. Thus
// we have some additional accesses (due to the 1st step)
// to mark as volatile.
//
static void
set_aggregate_fields_volatile(TY_IDX &ty_idx)
{
  if (TY_kind(ty_idx) == KIND_STRUCT) {
    if (!TY_fld (ty_idx).Is_Null ()) {
      FLD_ITER fld_iter = Make_fld_iter (TY_fld (ty_idx));
      do {
	TY_IDX fld_ty = FLD_type (fld_iter);
	set_aggregate_fields_volatile(fld_ty);
	Set_FLD_type(fld_iter, fld_ty);
      } while (!FLD_last_field (fld_iter++));
    }
  } else {
    Set_TY_is_volatile (ty_idx);
  }
}

static int
check_and_update_volatility(TY_IDX &ty_idx)
{
  int volatility = FALSE;
  if (TY_is_volatile(ty_idx)) volatility = TRUE;
  else if (TY_kind(ty_idx) == KIND_STRUCT) {
    if (!TY_fld (ty_idx).Is_Null ()) {
      FLD_ITER fld_iter = Make_fld_iter (TY_fld (ty_idx));
      do {
	TY_IDX fld_ty = FLD_type (fld_iter);
	if (check_and_update_volatility(fld_ty)) {
	  Set_FLD_type(fld_iter, fld_ty);
	  volatility = TRUE;
	  break;
	}
      } while (!FLD_last_field (fld_iter++));
    }
    if (TY_is_union(ty_idx) &&
	volatility) {
      set_aggregate_fields_volatile (ty_idx);
    }
  }
  return volatility;
}

static void
fixup_volatility(TY_IDX &ty_idx)
{
  check_and_update_volatility(ty_idx);
}

static void
print_volatility(TY_IDX &ty_idx)
{
  char info[4] = {0,0,0,0};
  char *p = info;
  printf("TY: %s: ", TY_name(ty_idx));
  if (TY_is_volatile(ty_idx)) printf("V");
  printf("\n");
  if (TY_kind(ty_idx) == KIND_STRUCT) {
    if (TY_is_union(ty_idx)) printf("UNION");
    else printf("STRUCT");
    printf(" FIELDS {\n");
    
    if (!TY_fld (ty_idx).Is_Null ()) {
      FLD_ITER fld_iter = Make_fld_iter (TY_fld (ty_idx));
      do {
	TY_IDX fld_ty = FLD_type (fld_iter);
	printf(" %s {\n", FLD_name(fld_iter));
	print_volatility(fld_ty);
	printf(" }\n");
      } while (!FLD_last_field (fld_iter++));
    }
    printf(" }\n");
  }
}
#endif
/*
 * The following functions are for symbols' visibility management.
 * The visibility is an attribute of a symbol declaration/definition.
 * The visibility apply to functions and to variables.
 * In this interface visibility is an int type defined with the 
 * following values (ref. elf.h):
 * -1:	undefined
 *  0:	STV_DEFAULT
 *  1:	STV_INTERNAL
 *  2:	STV_HIDDEN
 *  3:	STV_PROTECTED
 *
 * The export class is defined in symtab_defs.h and contains the visibility
 * in addition to the storeage class.
 *
 * Function list:
 *  int Get_Default_Visibility(tree decl_node, ST_class st_class , ST_SCLASS sclass)
 *	Gets the default visibility for the node. It may be specified
 *	by the user command line switch -fvisibility=...
 *	This command line switch do not apply to external definitions.
 *	The default value is STV_DEFAULT.
 *
 *  void Set_Default_Visibility(int visibility)
 *	Sets the default visibility.
 *
 *  int	Get_Visibility_From_Attribute (tree decl_node, ST_class st_class , ST_SCLASS sclass)
 *	Gets the visibility from a specified visibility attribute on the
 *	declaration tree.
 *	Returns -1 if no visibility attribute is specified.
 *
 *  int	Get_Visibility_From_Specification_File (tree decl_node, ST_class st_class , ST_SCLASS sclass)
 *	Gets the visibility for the symbol corresponding to the declaration
 *	node from the visibility specification file.
 *	Returns -1 if no specifcation file was given or if the name of the
 *	symbol is not matched by the specification file.
 *
 *  int Get_Visibility_For_Tree (tree decl_node, ST_class st_class , ST_SCLASS sclass)
 *	Gets the symbol visibility for a particular tree decl./def.
 *	(tree code must be VAR_DECL or FUNCTION_DECL).
 *	The symbol visibility is determined in the following order:
 *	1. node's visibility as returned by 
 *	   Get_Visibility_From_Attribute(),
 *	2. otherwise, symbol name visibility as returned by 
 *	   Get_Visibility_from_Specification_File(),
 *	3. otherwise, default visibility as returned by 
 *	   Get_Default_Visibility().
 *	
 *  ST_EXPORT Adjust_Export_Class_Visibility(ST_EXPORT default_eclass, int sv)
 *	Adjusts the export class to the given visibility.
 *
 *  ST_EXPORT Get_Export_Class_For_Tree (tree decl_node, ST_class st_class , ST_SCLASS sclass)
 *	Gets the export class from a declaration tree as follow:
 *	1. gets scope for tree (global or local symbol),
 *	2. gets visibility as returned by Get_Visibility_For_Tree(),
 *	3. returns export class as given by 
 *	   Get_Export_Class_From_Scope_And_Visibility().
 *
 */
#include "vspec.h"
#include "vspec_parse.h"
#include "erglob.h"

#ifndef STV_DEFAULT
#define	STV_DEFAULT	0
#define	STV_INTERNAL	1
#define	STV_HIDDEN	2
#define	STV_PROTECTED	3
#endif

static int default_visibility = STV_DEFAULT;

static int
Get_Default_Visibility(gs_t decl_node, ST_CLASS st_class, ST_SCLASS sclass)
{
  if (ENV_Symbol_Visibility != STV_DEFAULT) 
    default_visibility = ENV_Symbol_Visibility;
  /* Default visibility only apply to definitions, not
     external declarations. */
  if ((st_class == CLASS_FUNC ||
       st_class == CLASS_VAR) &&
      sclass != SCLASS_EXTERN) {
    return default_visibility;
  }
  return STV_DEFAULT;
}

static void
Set_Default_Visibility(int visibility)
{
  default_visibility = visibility;
}

static int
Get_Visibility_From_Attribute (gs_t decl_node, ST_CLASS st_class, ST_SCLASS sclass)
{
  /* This is the ordering in gcc:
     enum gcc_symbol_visibility
     {
     VISIBILITY_DEFAULT,
     VISIBILITY_PROTECTED,
     VISIBILITY_HIDDEN,
     VISIBILITY_INTERNAL
     };
  */
  static const int o64_vis [] = {
    STV_DEFAULT, STV_PROTECTED, STV_HIDDEN, STV_INTERNAL };
  int sv = -1;
  if (gs_decl_visibility_specified (decl_node))
    sv = o64_vis[gs_n(gs_decl_visibility (decl_node))];
  return sv;
}

static int
Get_Visibility_From_Specification_File(gs_t decl_node, ST_CLASS st_class, ST_SCLASS sclass)
{
  int sv = -1;
  const char *name;
  static vspec_pattern_list_t *vspec;

  /* (cbr) temporary variables don't have visibility attribute */
  if (!gs_decl_assembler_name_set_p (decl_node))
    return sv;

  FmtAssert(gs_decl_assembler_name(decl_node) != NULL, 
	    ("Get_Visibility_From_Specification_File(): Unnamed symbol declaration"));

  if (ENV_Symbol_Visibility_Spec_Filename == NULL) return sv;

  /* We take the assembler name for matching. */
  name = gs_identifier_pointer (gs_decl_assembler_name (decl_node));
  /* Parse specification file if not done. */
  if (vspec == NULL) {
    vspec = vspec_parse(ENV_Symbol_Visibility_Spec_Filename);
    if (vspec == NULL) {
      ErrMsg(EC_VisSpec_File, vspec_parse_strerr());
      ENV_Symbol_Visibility_Spec_Filename = NULL;
      return sv;
    }
  }
  
  sv = (int)vspec_pattern_list_match(vspec, name);
  return sv;
}

static int
Get_Visibility_For_Tree (gs_t decl_node, ST_CLASS st_class , ST_SCLASS sclass)
{
  int sv = STV_DEFAULT;
  if (st_class == CLASS_FUNC || st_class == CLASS_VAR) {
    sv = Get_Visibility_From_Attribute(decl_node, st_class, sclass);
    if (sv != -1) return sv;
    sv = Get_Visibility_From_Specification_File(decl_node, st_class, sclass);
    if (sv != -1) return sv;
    sv = Get_Default_Visibility(decl_node, st_class, sclass);
  }
  return sv;
}

static ST_EXPORT
Adjust_Export_Class_Visibility(ST_EXPORT default_eclass, int sv)
{
  ST_EXPORT eclass;
  switch(default_eclass) {
  case EXPORT_LOCAL:
  case EXPORT_LOCAL_INTERNAL:
    /* Local symbol. */
    if (sv == STV_INTERNAL) eclass = EXPORT_LOCAL_INTERNAL;
    else eclass = EXPORT_LOCAL;
    break;
  default:
    /* Global symbol. */
    switch (sv) {
    case STV_INTERNAL: eclass = EXPORT_INTERNAL; break;
    case STV_HIDDEN: eclass = EXPORT_HIDDEN; break;
    case STV_PROTECTED: eclass = EXPORT_PROTECTED; break;
    default: eclass = EXPORT_PREEMPTIBLE; break;
    }
  } 
  return eclass;
}

ST_EXPORT
Get_Export_Class_For_Tree (gs_t decl_node, ST_CLASS st_class, ST_SCLASS sclass)
{
  ST_EXPORT eclass;
  int sv;
  FmtAssert(gs_tree_code(decl_node) == GS_VAR_DECL ||
	    gs_tree_code(decl_node) == GS_FUNCTION_DECL, 
	    ("Get_Export_Scope_For_Tree unexpected tree"));
  FmtAssert(st_class == CLASS_FUNC ||
	    st_class == CLASS_VAR, 
	    ("Get_Export_Scope_For_Tree unexpected symbol class"));
  
  sv = Get_Visibility_For_Tree(decl_node, st_class, sclass);

  if (gs_tree_code(decl_node) == GS_FUNCTION_DECL) {
    if (gs_tree_public(decl_node)) eclass = EXPORT_PREEMPTIBLE;
    else eclass = EXPORT_LOCAL;
  } else if (gs_tree_code(decl_node) == GS_VAR_DECL) {
    if (gs_decl_register(decl_node))
      eclass = EXPORT_LOCAL;
    else if (gs_tree_public(decl_node) ||
	(gs_decl_context (decl_node) != 0 && gs_decl_external(decl_node)))
      eclass = EXPORT_PREEMPTIBLE;
    else 
      eclass = EXPORT_LOCAL;
  }
  eclass = Adjust_Export_Class_Visibility(eclass, sv);
  return eclass;
}

#include <ext/hash_map>

namespace {

  using __gnu_cxx::hash_map;

  struct ptrhash {
    size_t operator()(void* p) const { return reinterpret_cast<size_t>(p); }
  };

  hash_map<gs_t, TY_IDX,     ptrhash>     ty_idx_map;
  hash_map<gs_t, ST*,        ptrhash>     st_map;
  hash_map<gs_t, SYMTAB_IDX, ptrhash>     symtab_idx_map;
  hash_map<gs_t, LABEL_IDX,  ptrhash>     label_idx_map;
  hash_map<gs_t, ST*,        ptrhash>     string_st_map;
  hash_map<gs_t, BOOL,       ptrhash>     bool_map;
  hash_map<gs_t, INT32,      ptrhash>     field_id_map;
  hash_map<gs_t, INT32,	  ptrhash>     type_field_ids_used_map;
  hash_map<gs_t, INT32,      ptrhash>     scope_number_map;
  hash_map<gs_t, gs_t,       ptrhash>     label_scope_map;
  hash_map<gs_t, DST_INFO_IDX,ptrhash>    decl_idx_map; 
  hash_map<gs_t, DST_INFO_IDX,ptrhash>    decl_field_idx_map; 
  hash_map<gs_t, DST_INFO_IDX,ptrhash>    decl_specification_idx_map; 
  hash_map<gs_t, DST_INFO_IDX,ptrhash>    type_idx_map;
  hash_map<gs_t, LABEL_IDX,  ptrhash>     handler_label_map;
  hash_map<gs_t, DST_INFO_IDX,ptrhash>    abstract_root_map;
#ifdef KEY
  // Map PU to the PU-specific st_map.
  hash_map<PU*, hash_map<gs_t, ST*, ptrhash>*, ptrhash>     pu_map;
  // TRUE if ST is a decl that is being/already been expanded.
  hash_map<gs_t, BOOL,        ptrhash>     expanded_decl_map;
  // TRUE if TREE is a DECL_FUNCTION whose PU should have PU_uplevel set.
  hash_map<gs_t, BOOL,        ptrhash>     func_PU_uplevel_map;
  hash_map<gs_t, gs_t,	      ptrhash>	   parent_scope_map;
  // Record whether a symbol referenced in a cleanup should be marked weak as a
  // workaround to the fact that kg++fe may emit cleanups that g++ won't emit
  // because g++ knows that are not needed.  The linker will complain if these
  // symbols are not defined.
  hash_map<ST*, INT32,        ptrhash>     weak_workaround_map;
#endif
  hash_map<gs_t, ST*,	      ptrhash>	   decl_st2_map;
}

TY_IDX& TYPE_TY_IDX(gs_t t)         { return ty_idx_map[t]; }

BOOL& expanded_decl(gs_t t) {
  FmtAssert (t, ("func_expanded: not a decl"));
  return expanded_decl_map[t];
}

// Put ST in a map based on the tree node T and the current PU.
void
set_DECL_ST(gs_t t, ST* st) {

  // Find the tree node to use as index into st_map.
  gs_t t_index;
  if (
#ifndef TARG_ST
      gs_tree_code(t) == GS_VAR_DECL &&
#endif
      (gs_decl_context(t) == 0 || 
       gs_tree_code(gs_decl_context(t)) == GS_NAMESPACE_DECL) &&
     gs_decl_name (t) && gs_decl_assembler_name(t))
    t_index = gs_decl_assembler_name(t);
  else
    t_index = t;

  // If ST is 1, then the caller only wants to pretend that there is a symbol
  // for T.  Later on, the caller will reset the ST to NULL and assign a real
  // symbol to T.
  if (st == (ST *) 1) {
    st_map[t_index] = st;
    return;
  }

  // If ST is a symbol that should be shared across functions, then put ST in
  // the st_map, which maps T directly to ST.  Otherwise, put ST in the
  // PU-specific st_map.
  //
  // It is observed that g++ uses the same tree for different functions, such
  // as inline functions.  As a result, we cannot attach PU-specific ST's 
  // directly to the tree nodes.
  //
  // If Current_scope is 0, then the symbol table has not been initialized, and
  // we are being called by WFE_Add_Weak to handle a weak symbol.  In that
  // case, use the non-PU-specific st_map.
  if (Current_scope != 0 &&
      (gs_tree_code(t) == GS_PARM_DECL ||
       (gs_tree_code(t) == GS_VAR_DECL &&
        (ST_sclass(st) == SCLASS_AUTO ||
         (! pstatic_as_global &&
	  ST_sclass(st) == SCLASS_PSTATIC))))) {
    // ST is PU-specific.  Use pu_map[pu] to get the PU-specific st_map, then
    // use st_map[t] to get the ST for the tree node t.
    //
    // We can access pu_map[pu] only if Scope_tab[Current_scope].st is valid
    // because we need to get the current PU, but Get_Current_PU requires a
    // valid Scope_tab[Current_scope].st.  If Scope_tab[Current_scope].st is
    // not set, then this means the caller is trying to create the ST for the
    // function symbol.
    if (Scope_tab[Current_scope].st != NULL) {
      // ok to call Get_Current_PU.
      PU *pu = &Get_Current_PU();
      hash_map<PU*, hash_map<gs_t, ST*, ptrhash>*, ptrhash>::iterator it =
	pu_map.find(pu);
      if (it == pu_map.end()) {
	// Create new PU-specific map.
	pu_map[pu] = new hash_map<gs_t, ST*, ptrhash>;
      }
      // Put the ST in the PU-specific st_map.
      (*(pu_map[pu]))[t_index] = st;
    }
  } else {
#ifdef Is_True_On
    if (st_map[t_index]) {
      // The st_map is already set.  This is ok only for weak ST.
      FmtAssert (ST_is_weak_symbol(st_map[t_index]),
		 ("set_DECL_ST: st_map already set"));
    }
#endif
    // Put the ST in the non-PU-specific st_map.
    st_map[t_index] = st;
  }
}

// Get ST associated with the tree node T.
ST*&
get_DECL_ST(gs_t t) {
  static ST *null_ST = (ST *) NULL;
  static ST *return_st;

  // Find the tree node to use as index into st_map.
  gs_t t_index;
  if (
#ifndef TARG_ST
      gs_tree_code(t) == GS_VAR_DECL &&
#endif
      (gs_decl_context(t) == 0 || 
       gs_tree_code(gs_decl_context(t)) == GS_NAMESPACE_DECL) &&
     gs_decl_name (t) && gs_decl_assembler_name(t))
    t_index = gs_decl_assembler_name(t);
  else
    t_index = t;

  // If Current_scope is 0, then the symbol table has not been initialized, and
  // we are being called by WFE_Add_Weak to handle a weak symbol.  Use the
  // non-PU-specific st_map.
  if (Current_scope == 0) {
    return_st = st_map[t_index];
    if(return_st)
      return_st = Trans_TLS(t, return_st);
    return return_st;
  }

  // See if the ST is in the non-PU-specific st_map.
  if (st_map[t_index]) {
    return_st = st_map[t_index];
    if(return_st)
      return_st = Trans_TLS(t, return_st);    
    return return_st;
  }

  // The ST is not in the non-PU-specific map.  Look in the PU-specific maps.
  INT scope = Current_scope;
  do {
    // If Scope_tab[scope].st is NULL, then the function ST has not
    // been set yet, and there is no PU-specific map.
    if (Scope_tab[scope].st != NULL) {
      // See if there is a PU-specific map.
      PU *pu = &Get_Scope_PU(scope);
      hash_map<PU*, hash_map<gs_t, ST*, ptrhash>*, ptrhash>::iterator pu_map_it =
	pu_map.find(pu);
      if (pu_map_it != pu_map.end()) {
        // There is a PU-specific map.  Get the ST from the map.
        hash_map<gs_t, ST*, ptrhash> *st_map2 = pu_map[pu];
        if ((*st_map2)[t_index]) {
          return_st = (*st_map2)[t_index];
          //assert(return_st);
          return_st = Trans_TLS(t, return_st);
          return return_st;
        }
      }
    }
    scope--;
  } while (scope > 1);
  return null_ST;
}

BOOL&
func_PU_uplevel(gs_t t) {
  FmtAssert (gs_tree_code(t) == GS_FUNCTION_DECL,
	     ("func_PU_uplevel: not a FUNCTION_DECL tree node"));
  return func_PU_uplevel_map[t];
}

INT32& WEAK_WORKAROUND(ST *st)         { return weak_workaround_map[st]; }

SYMTAB_IDX& DECL_SYMTAB_IDX(gs_t t) { return symtab_idx_map[t]; }
LABEL_IDX& DECL_LABEL_IDX(gs_t t)   { return label_idx_map[t]; }
ST*& TREE_STRING_ST(gs_t t)         { return string_st_map[t]; }
BOOL& DECL_LABEL_DEFINED(gs_t t)    { return bool_map[t]; }
INT32& DECL_FIELD_ID(gs_t t)        { return field_id_map[t]; }
#ifdef TARG_ST
void SET_TYPE_FIELD_IDS_USED(gs_t t, INT32 v) {
  type_field_ids_used_map[t] = v;
}
BOOL TYPE_FIELD_IDS_USED_KNOWN(gs_t t) {
  hash_map<gs_t, INT32, ptrhash>::iterator it = type_field_ids_used_map.find(t);
  return it != type_field_ids_used_map.end();
}
INT32 GET_TYPE_FIELD_IDS_USED(gs_t t) {
  INT32 v = -1;
  hash_map<gs_t, INT32, ptrhash>::iterator it = type_field_ids_used_map.find(t);
  if (it == type_field_ids_used_map.end()) {
    FmtAssert (gs_tree_code(t) != GS_RECORD_TYPE
	       && gs_tree_code(t) != GS_UNION_TYPE,
	       ("TYPE_FIELD_IDS_USED not set"));
    v = 0;
    type_field_ids_used_map[t] = v;
  } else {
    v = it->second;
  }
  return v;
}
#else
INT32 & TYPE_FIELD_IDS_USED(gs_t t) { return type_field_ids_used_map[t]; }
#endif
INT32 & SCOPE_NUMBER(gs_t t)        { return scope_number_map[t]; }
#ifdef KEY
gs_t & PARENT_SCOPE(gs_t t)	    { return parent_scope_map[t]; }
#endif
gs_t & LABEL_SCOPE(gs_t t)	    { return label_scope_map[t]; }
ST* & DECL_ST2(gs_t t)		    { return decl_st2_map[t]; }

// This is for normal declarations.

// We do not know if the DST entry is filled in.
// So check and ensure a real entry exists.

DST_INFO_IDX & DECL_DST_IDX(gs_t t) 
{ 
	hash_map<gs_t, DST_INFO_IDX,ptrhash>::iterator it =
		decl_idx_map.find(t);
	if(it == decl_idx_map.end()) {
		// substitute for lack of default constructor
		DST_INFO_IDX dsti = DST_INVALID_IDX;
		decl_idx_map[t] = dsti;
	}
	return decl_idx_map[t]; 
}
// This is for static class members and member functions.
// We need a distinct DST record for a single ST.
// Note that only the main record actually need be linked
// to ST as only that one gets an address/location.

// We do not know if the DST entry is filled in.
// So check and ensure a real entry exists.
DST_INFO_IDX & DECL_DST_SPECIFICATION_IDX(gs_t t) 
{ 
	hash_map<gs_t, DST_INFO_IDX,ptrhash>::iterator it =
		decl_specification_idx_map.find(t);
	if(it == decl_specification_idx_map.end()) {
		// substitute for lack of default constructor
		DST_INFO_IDX dsti = DST_INVALID_IDX;
		decl_specification_idx_map[t] = dsti;
	}
	return decl_specification_idx_map[t]; 
}

// This is for static class members and member functions.
// We need a distinct DST record for a single ST.
// Note that only the main record actually need be linked
// to ST as only that one gets an address/location.

// We do not know if the DST entry is filled in.
// So check and ensure a real entry exists.
DST_INFO_IDX & DECL_DST_FIELD_IDX(gs_t t) 
{ 
	hash_map<gs_t, DST_INFO_IDX,ptrhash>::iterator it =
		decl_field_idx_map.find(t);
	if(it == decl_idx_map.end()) {
		// substitute for lack of default constructor
		DST_INFO_IDX dsti = DST_INVALID_IDX;
		decl_field_idx_map[t] = dsti;
	}
	return decl_field_idx_map[t]; 
}

// We do not know if the DST entry is filled in.
// So check and ensure a real entry exists.
DST_INFO_IDX & TYPE_DST_IDX(gs_t t) 
{
	hash_map<gs_t, DST_INFO_IDX,ptrhash>::iterator it =
		type_idx_map.find(t);
	if(it == type_idx_map.end()) {
		// substitute for lack of default constructor
		DST_INFO_IDX dsti = DST_INVALID_IDX;
		type_idx_map[t] = dsti;
	}
	return type_idx_map[t]; 
}

// We do not know if the DST entry is filled in.
// So check and ensure a real entry exists.
DST_INFO_IDX & DECL_DST_ABSTRACT_ROOT_IDX(gs_t t) 
{
	hash_map<gs_t, DST_INFO_IDX,ptrhash>::iterator it =
		abstract_root_map.find(t);
	if(it == abstract_root_map.end()) {
		// substitute for lack of default constructor
		DST_INFO_IDX dsti = DST_INVALID_IDX;
		abstract_root_map[t] = dsti;
	}
	return abstract_root_map[t]; 
}


LABEL_IDX& HANDLER_LABEL(gs_t t)    { return handler_label_map[t]; }
