/*
 *  Copyright (C) 2006. QLogic Corporation. All Rights Reserved.
 */

//-*-c++-*-

/*
 * Copyright 2005, 2006 PathScale, Inc.  All Rights Reserved.
 */

// ====================================================================
// ====================================================================
//
// Module: opt_htable_emit.cxx
// $Revision: 1.5 $
// $Date: 05/05/06 08:36:10-07:00 $
// $Author: bos@eng-24.pathscale.com $
// $Source: be/opt/SCCS/s.opt_htable_emit.cxx $
//
// Revision history:
//  03-OCT-96 shin - Original Version
//
// ====================================================================
//
// Copyright (C) 2000, 2001 Silicon Graphics, Inc.  All Rights Reserved.
//
/*
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
//
// ====================================================================
//
// Description:
//
// ====================================================================
// ====================================================================


#ifdef USE_PCH
#include "opt_pch.h"
#endif // USE_PCH
#pragma hdrstop


#ifdef _KEEP_RCS_ID
#define opt_htable_emit_CXX	"opt_htable_emit.cxx"
static char *rcs_id = 	opt_htable_emit_CXX"$Revision: 1.5 $";
#endif /* _KEEP_RCS_ID */

#include "limits.h"
#include "defs.h"
#include "tracing.h"
#include "config_targ.h"
#include "wn.h"
#include "wn_simp.h"
#include "wn_util.h"

#include "opt_alias_mgr.h"
#include "opt_base.h"
#include "opt_bb.h"
#include "opt_cfg.h"
#include "opt_htable.h"
#include "opt_main.h"
#include "opt_mu_chi.h"
#include "opt_region_emit.h"	// PRUNE_BOUND
#include "opt_rvi.h"
#include "opt_sym.h"

#include "opt_emit_template.h" // this comes the last

class RVI;

class ML_WHIRL_EMITTER {
private:
  CFG             *_cfg;
  CODEMAP         *_htable;
  OPT_STAB        *_opt_stab;
  WN              *_opt_func;
  STMT_CONTAINER   _wn_list;
  MEM_POOL        *_loc_pool;
  MEM_POOL        *_mem_pool;
  ALIAS_MANAGER   *_alias_mgr;
  BOOL	           _do_rvi;       // do the bit vector RVI afterward
  RVI             *_rvi;
  BOOL             _trace;

  STACK<E_REGION*> _region_stack; // for MP regions

  ID_MAP<IDTYPE, INT32> _preg_renumbering_map;	// never initialized
						// in current
						// implementation.

  ML_WHIRL_EMITTER(const ML_WHIRL_EMITTER&);
  ML_WHIRL_EMITTER& operator = (const ML_WHIRL_EMITTER&);

  WN              *Emit(void);

  void             Gen_stmt(STMTREP*);

  void             Create_entry(BB_NODE *bb);
  STACK<E_REGION *> *Region_stack(void)       { return &_region_stack; }

  void             Pop_region(void);

public:
  ML_WHIRL_EMITTER(CFG           *cfg,
                   OPT_STAB      *opt_stab,
                   CODEMAP       *htable,
                   ALIAS_MANAGER *alias_mgr,
                   RVI           *rvi,
                   MEM_POOL      *lpool,
                   MEM_POOL      *gpool);
  ~ML_WHIRL_EMITTER(void)                            {}

  OPT_STAB        *Opt_stab(void) const       { return _opt_stab; }
  MEM_POOL        *Loc_pool(void) const       { return _loc_pool; }
  MEM_POOL        *Mem_pool(void) const       { return _mem_pool; }
  CFG             *Cfg(void) const            { return _cfg; }
  CODEMAP         *Htable(void) const         { return _htable; }
  WN              *Opt_func(void) const       { return _opt_func; }
  STMT_CONTAINER  *Wn_list(void)              { return &_wn_list; }
  ALIAS_MANAGER   *Alias_Mgr(void) const      { return _alias_mgr; }
  BOOL             Trace(void) const          { return _trace; }
  BOOL             For_preopt(void) const     { return FALSE; }

  void             Gen_wn(BB_NODE *f,
                          BB_NODE *l);
  void             Insert_wn(WN *wn)          { _wn_list.Append(wn); }
                                              // insert a wn into the list
  BOOL             Verify(WN *wn);
  WN              *Build_loop_info(BB_NODE *);// build loop_info wn

  // a few routines used by the template in opt_emit_template.h
  ID_MAP<IDTYPE, INT32> &Preg_renumbering_map(void)
    { return _preg_renumbering_map; }	// not used, but must be defined.

  BOOL             Gen_lno_info(void)         { return TRUE; }
  BOOL	           Do_rvi(void) const 
			{ return WOPT_Enable_RVI1 && _do_rvi; }
  RVI             *Rvi(void) const            { return _rvi; };

  void             Connect_sr_wn(STMTREP*,
                                 WN *)        {}
  void             Set_region_entry_stmt(STMTREP*) {}
  STMTREP         *Region_entry_stmt(void)    { return NULL; }

  WN_MAP          *Wn_to_cr_map(void)         { return NULL; }
};


// fix up the function entry, handles region entry also
void
ML_WHIRL_EMITTER::Create_entry(BB_NODE *bb)
{
  // this is similar to Create_block in preopt emitter
  WN *stmt = WN_CreateBlock();
  WN_first(stmt) = Wn_list()->Head();
  WN_last(stmt) = Wn_list()->Tail();

  WN *wn = bb->Entrywn();
  Is_True(wn != NULL,("ML_WHIRL_EMITTER::Create_entry, wn is NULL"));

  WN_Set_Linenum(stmt, WN_Get_Linenum(wn));
  _opt_func = wn;
  bb->Init_stmt(_opt_func);

  WN_func_body(_opt_func) = stmt;
  REGION_emit(Cfg()->Rid(), _opt_func, RL_MAINOPT, 1, 0);
}

void
ML_WHIRL_EMITTER::Gen_stmt(STMTREP *stmt)
{
  WN *wn = Gen_stmt_wn(stmt, Wn_list(), this);

  // OPR_EVAL, need loop info
}

BOOL
ML_WHIRL_EMITTER::Verify(WN *wn)
{
  return TRUE;
}

void
ML_WHIRL_EMITTER::Pop_region( void )
{
  E_REGION *e_region = _region_stack.Pop();
  WN *prev_wn = e_region->Prev_wn();
  WN *last_region_wn = _wn_list.Tail();
  WN *first_region_wn = prev_wn ? WN_next(prev_wn) : _wn_list.Head();
  BB_REGION *bb_region = e_region->Region_start()->Regioninfo();

  // Sometimes the region emitted is empty (EH Guard regions have
  // to be present even if they are empty). Fix up the pointers.
  if (!first_region_wn) {
    Is_True(prev_wn == _wn_list.Tail(),
	    ("ML_WHIRL_EMITTER::Pop_region, prev_wn mistake"));
    last_region_wn = NULL;
  }

  // create the region and the body
  WN *region_body = WN_CreateBlock();
  WN_first(region_body) = first_region_wn;
  WN_last(region_body)  = last_region_wn;

  WN *region_wn = WN_CreateRegion(REGION_type_to_kind(bb_region->Rid()),
				  region_body,
				  bb_region->Region_pragma_list(),
				  bb_region->Region_exit_list(), 
				  RID_id(bb_region->Rid()),
				  bb_region->Ereg_supp());

  // go through region pragmas and convert aux_ids back to STs and offsets
  if (REGION_is_EH(region_wn))
    Opt_stab()->Convert_EH_pragmas(region_wn);

  // update the wn list so this region node replaces the statements
  // it put into its body.
  if ( first_region_wn != NULL )
    WN_prev(first_region_wn) = NULL;
  Is_True( last_region_wn == NULL || WN_next(last_region_wn) == NULL,
    ("MAIN_EMITTER::Pop_region: last_region_wn has non-null next") );

  if ( prev_wn != NULL )
    WN_next(prev_wn) = region_wn;
  WN_prev(region_wn) = prev_wn;
  _wn_list.Set_tail(region_wn);
  // do we become the first statement?
  if ( first_region_wn == _wn_list.Head() )
    _wn_list.Set_head(region_wn);

  // update the RID and level
  REGION_emit(bb_region->Rid(), region_wn, RL_MAINOPT, 
	      bb_region->Region_num_exits(), bb_region->Region_line_num());
}

//=====================================================================
// Try to build a LOOP_INFO wn for emitting.  If unsuccessful, returns
// a NULL wn.
//=====================================================================

WN *
ML_WHIRL_EMITTER::Build_loop_info( BB_NODE *label_bb )
{
  if ( label_bb->Label_loop_info() == NULL )
    return NULL;

  UINT16 est_trips = WN_loop_trip_est(label_bb->Label_loop_info());
  UINT16 depth = WN_loop_depth(label_bb->Label_loop_info());
  INT32 lflags = WN_loop_flag(label_bb->Label_loop_info());

  // this label_bb should be the first body_bb in the loop.  So, it's
  // prev block should be the dohead.
  BB_NODE *dohead_bb = label_bb->Prev();
#ifdef TARG_ST
  // FdF 14/06/2004: Check that the preheader is the static
  // predecessor of loop head
  if (dohead_bb == NULL || label_bb->Loop() == NULL || dohead_bb != label_bb->Loop()->Preheader() ) {
    // this must not be a valid do-loop any more
    return NULL;
  }
#else
  if ( dohead_bb == NULL || dohead_bb->Kind() != BB_DOHEAD ) {
    // this must not be a valid do-loop any more
    return NULL;
  }
#endif

#ifdef TARG_ST
  // FdF 10/05/2004: Check that these nodes are connected through
  // control-flow
  if (dohead_bb != label_bb->Idom()) {
    return NULL;
  }
#endif

  // make sure the cfg's loop information is still valid, and this
  // block is indeed the first body block.
  BB_LOOP *bb_loop = dohead_bb->Loop();
#ifdef TARG_ST
  // FdF 14/06/2004
  bb_loop = label_bb->Loop();
#endif
  if ( bb_loop == NULL || bb_loop->Body() != label_bb ) {
    return NULL;
  }

  BB_NODE *doend_bb = bb_loop->End();
  if ( doend_bb == NULL ||
      (doend_bb->Kind() != BB_DOEND &&
       doend_bb->Kind() != BB_WHILEEND && doend_bb->Kind() != BB_REPEATEND) ) {
    // this must not be a valid do-loop any more
    return NULL;
  }

  // make sure that the dohead dominates the ending condition
  if ( ! dohead_bb->Dominates_strictly( doend_bb ) ) {
    return NULL;
  }

  // if feedback info is avaiable, update the estimated trip count for CG.
  if ( Cfg()->Feedback() ) {
    FB_FREQ
      freq_trips = Cfg()->Feedback()->Get_node_freq_out( doend_bb->Id() );
    freq_trips /= Cfg()->Feedback()->Get_node_freq_out( dohead_bb->Id() );
    if ( freq_trips.Known() ) {
#ifdef TARG_ST
      //[TB] to avoid trip_estimated to be NULL when loop is never
      //executed in the profiling info
      INT32 trips = MAX(INT32( freq_trips.Value() + 0.5 ), 1);
#else
      INT32 trips = INT32( freq_trips.Value() + 0.5 );
#endif
      // est_trip is UINT16: check for overflow.
      est_trips = (trips <= USHRT_MAX) ? trips : USHRT_MAX;
    }
  }

  // Set the innermost flag
  if (bb_loop->Child() == NULL)
    lflags |= WN_LOOP_INNERMOST;
  else
    lflags &= ~WN_LOOP_INNERMOST;

  // this is the common point at which we can build the node, given
  // all available information
  WN *trip_count = NULL;
  WN *induction = NULL;

  // did LFTR replace the induction variable?
  if ( bb_loop->Iv_replacement() != NULL ) {
    induction = Gen_exp_wn(bb_loop->Iv_replacement(), this);
  }
  else {
    CODEREP *iv = bb_loop->Iv();
    if (iv != NULL && iv->Kind() == CK_VAR && 
	(Do_rvi() && iv->Bitpos() != ILLEGAL_BP ||
         !Do_rvi()))
    {
#ifdef TARG_ST
      // [CG] bug 1-6-0-B/19
      // Use object_ty instead of Lod_ty to handle structure fields.
      // and create a LDID with correct type and field id.
      MTYPE ivtype = TY_mtype(iv->object_ty());
      induction = WN_CreateLdid(Ldid_from_mtype(ivtype),
				iv->Offset(),
				Opt_stab()->St(iv->Aux_id()),
				iv->Lod_ty(), iv->Field_id());
#else
      MTYPE ivtype = TY_mtype(iv->Lod_ty());
#ifdef KEY // bug 5645
      if (ivtype == MTYPE_M)
	ivtype = iv->Dtyp();
#endif

      induction = WN_CreateLdid(Ldid_from_mtype(ivtype),
				iv->Offset(),
				Opt_stab()->St(iv->Aux_id()),
				iv->Lod_ty(),
				iv->Field_id());
#endif
      if (Do_rvi() && ST_class(WN_st(induction)) != CLASS_PREG) {
	Warn_todo("ML_WHIRL_EMITTER::Build_loop_info: do not adjust bitpos by 1" );
	Rvi()->Map_bitpos(induction, iv->Bitpos() + 1);
      }
      Alias_Mgr()->Gen_alias_id(induction, iv->Points_to(Opt_stab()));
    }
  }

  // must have an induction variable
  if ( induction == NULL ) {
    return NULL;
  }

  if (bb_loop->Trip_count_stmt() != NULL) {
    if (bb_loop->Wn_trip_count() != NULL) 
      trip_count = bb_loop->Wn_trip_count();
  } else {
    if (bb_loop->Trip_count_expr()) 
      trip_count = Gen_exp_wn(bb_loop->Trip_count_expr(), this);
  }

  WN *loop_info = WN_CreateLoopInfo( induction, trip_count, 
				     est_trips, depth, lflags );
  return loop_info;
}

// ====================================================================
//  Emit the whole program in WHIRL form
// ====================================================================
WN *
ML_WHIRL_EMITTER::Emit(void)
{
  if (Trace())
    fprintf(TFile,"%sML_WHIRL_EMITTER\n%s",DBar,DBar);

  // Reduce the region boundary sets to a minimum
  Is_True(Cfg()->Rid() != NULL, ("ML_WHIRL_EMITTER::Emit, NULL RID"));
  if (!RID_TYPE_func_entry(Cfg()->Rid())) {
    // the constructor does everything
    PRUNE_BOUND prune(Cfg(), Opt_stab());
    // the destructor is called here
  }

  // do we have valid loop-body information, which is necessary for
  // some of the checks
  Cfg()->Analyze_loops();

  BOOL saved_wn_simp_enable = WN_Simplifier_Enable(FALSE);

  // Fix 592011:  simplify CG's job.
  Cfg()->Delete_empty_BB();

  // Visit BBs in program order and preprocess then emit to WN
  BB_NODE *bb;
  CFG_ITER cfg_iter;
  FOR_ALL_ELEM(bb, cfg_iter, Init(Cfg())) {
    if (bb->Reached()) { // skip fake entry and exit BBs

      if (Trace())
	fprintf(TFile,"----- BB%" PRIdPTR " -----\n",bb->Id());

      // does this block mark the beginning of a region?
      if ( bb->Kind() == BB_REGIONSTART ) {
        WN *prev_wn = _wn_list.Tail();  // need to save the current tail
	Push_region(Region_stack(), bb, Loc_pool());
        E_REGION *e_region = _region_stack.Top();
        e_region->Set_prev_wn(prev_wn); // in new region's prev_wn
	Is_Trace(Trace(),(TFile,"Push_region(RGN %d), prev_wn = 0x%p\n",
		  RID_id(bb->Regioninfo()->Rid()),prev_wn));
      }

      // generate alternate entry statement if necessary
      if ( bb->Kind() == BB_ENTRY && bb->Entrywn() &&
	   (WN_opcode(bb->Entrywn()) == OPC_ALTENTRY ||
	    (WN_opcode(bb->Entrywn()) == OPC_LABEL &&
	     (WN_Label_Is_Handler_Begin(bb->Entrywn())
#if defined( KEY) && !defined(TARG_ST)
	      || LABEL_target_of_goto_outer_block(WN_label_number(bb->Entrywn()))
#endif
	     ))) )
      {
        Insert_wn( bb->Entrywn() );
	// Update Feedback?
      }

      // mark the beginning of this BB with a comment
      BOOL generate_comments = Get_Trace(TP_GLOBOPT, 0xffffffff);
      WN *comment_wn = NULL;
      if ( generate_comments ) {
	char str[120];
	sprintf(str,"BB%03" PRIdPTR " (%s) %40.40s", bb->Id(), bb->Kind_name(), SBar);
	comment_wn = WN_CreateComment(str);
	WN_Set_Linenum(comment_wn, bb->Linenum());

	// should we put the comment at the top of the block, or
	// immediately after the label?
	if ( bb->Label_stmtrep() == NULL ) {
	  Insert_wn( comment_wn );
	  comment_wn = NULL;
	}
      }

      STMTREP_ITER stmt_iter(bb->Stmtlist());
      STMTREP *tmp;
      FOR_ALL_NODE(tmp, stmt_iter, Init()) {
	if ( tmp->Live_stmt() )
          Gen_stmt(tmp);

	OPERATOR stmt_opr = OPCODE_operator(tmp->Op());

	// insert the comment if necessary after the label
	if ( stmt_opr == OPR_LABEL && comment_wn != NULL ) {
	  Insert_wn( comment_wn );
	  comment_wn = NULL;
	}
      }

      bb->Set_wngend();

      // was this block the end of a region?
      while (_region_stack.Elements() > 0 &&
	     _region_stack.Top()->Region_end() == bb) {
	Is_Trace(Trace(),(TFile,"Pop_region(RGN %d), prev_wn = 0x%p\n",
	    RID_id(_region_stack.Top()->Region_start()->Regioninfo()->Rid()),
		  _region_stack.Top()->Prev_wn()));
        Pop_region();
      }
    }
  }

  // we should have cleared off the stack of regions
  Is_True( _region_stack.Elements() == 0,
    ("ML_WHIRL_EMITTER::Emit: region stack not empty") );

  // find the bb that is the entry point for this PU/region
  BB_NODE *entry_bb = Cfg()->Find_entry_bb();
  if (entry_bb->Kind() == BB_ENTRY) {
    // generate a function entry
    Create_entry(entry_bb);
  } else { // region
    Is_True(entry_bb->Kind() == BB_REGIONSTART,
	    ("ML_WHIRL_EMITTER::Emit, unknown entry kind %s",
	     entry_bb->Kind_name()));
    _opt_func = _wn_list.Head();
    Is_True(REGION_consistency_check(_opt_func),
	    ("ML_WHIRL_EMITTER::Emit, inconsistent region"));
  }

  // update SYMTAB with number of labels
  Is_True(Get_Preg_Num(PREG_Table_Size(CURRENT_SYMTAB)) ==
	  Opt_stab()->Last_preg(),
	  ("ML_WHIRL_EMITTER::Emit, incorrect last preg number"));
  // For now, we assume that labels have been created during wopt
  // using the regular symtab mechanism, so we don't have to do
  // anything to tell the symtab how many labels we used.

  if (Trace())  {
    fprintf(TFile,"%sAfter ML_WHIRL_EMITTER\n%s",DBar,DBar);
    _alias_mgr->Print(_opt_func, TFile);
    Print_dep_graph(TFile);
  }

  /*PPP this shouldn't be necessary - the main emitter is destroying alias
    info for previously processed regions */
  REGION_update_alias_info(_opt_func,_alias_mgr);
  Verify(_opt_func);

  Is_True(REGION_consistency_check(_opt_func),(""));

  WN_Simplifier_Enable(saved_wn_simp_enable);

  return _opt_func;
}

ML_WHIRL_EMITTER::ML_WHIRL_EMITTER(CFG           *cfg,
                                   OPT_STAB      *opt_stab,
                                   CODEMAP       *htable,
                                   ALIAS_MANAGER *alias_mgr,
                                   RVI           *rvi,
                                   MEM_POOL      *lpool,
                                   MEM_POOL      *gpool)
     :_cfg(cfg), _opt_stab(opt_stab), _htable(htable), _alias_mgr(alias_mgr),
     _rvi(rvi), _do_rvi(rvi->Do_rvi()), _mem_pool(gpool), _loc_pool(lpool),
     _region_stack(lpool), _preg_renumbering_map(128, 0, lpool, FALSE)
{
  _trace = Get_Trace(TP_GLOBOPT, MAIN_EMIT_DUMP_FLAG);
  Emit();
}

WN*
COMP_UNIT::Emit_ML_WHIRL(RVI *rvi)
{
  // TODO: there are some question about Pop_region, not complete
  // TODO: LOOP_INFO is not done
  ML_WHIRL_EMITTER emitter(Cfg(),
                           Opt_stab(),
                           Htable(),
                           Alias_mgr(),
                           rvi,
                           Loc_pool(),
                           Mem_pool());
  return emitter.Opt_func();
}
