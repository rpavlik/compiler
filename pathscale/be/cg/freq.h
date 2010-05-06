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


/* ====================================================================
 * ====================================================================
 *
 * Module: freq.h
 * $Revision$
 * $Date$
 * $Author$
 * $Source$
 *
 * Description:
 *
 * Utilities for determining execution frequencies of BBs and edges.
 *
 * Utilities:
 *
 *    void FREQ_Compute_BB_Frequencies(void)
 *	Determine the block and edge frequency for all BBs in the control flow 
 *	graph. This is done using several heuristics to predict the direction
 *	of conditional branches and/or feedback if available. This routine
 *	also sets the BB_freq field of each basic block.
 *
 *    void FREQ_Incorporate_Feedback(cosnt WN* entry)
 *	Incorporate feedback (from the WHIRL) into the CG CFG; entry is the 
 *	entry whirl node of the tree
 *
 *    void FREQ_Print_BB_Note(BB *bb, FILE *file)
 *	Print a note to 'file' which indicates the block and successor
 *	edge frequencies for 'bb'.
 *
 *    void FREQ_Region_Initialize(void)
 *	Must be called at the start of processing for each region.
 *
 *    BOOL FREQ_Frequencies_Computed(void)
 *	Returns TRUE if block and edge frequencies have been computed;
 *	i.e. FREQ_Compute_BB_Frequencies was called and -CG:enable_freq
 *	was TRUE.
 *
 *    BOOL WN_Is_Pointer(WN *wn)
 *	Returns TRUE if whirl node 'wn' is a pointer.
 *
 *    BOOL FREQ_Match(float f1, float f2)
 *	Return TRUE if BB_freqs <f1> and <f2> are "close enough" (used in
 *	sanity checks of frequency info).
 *
 *    BOOL FREQ_Check_Consistency(const char *caller)
 *      Perform a consistency check over the BBs in the region.
 *      DevWarn if a problem is found. <caller> is an ascii string
 *      to identify the caller in error messages.  Return TRUE iff
 *      check detects no errors.
 *
 *    BB_SET *FREQ_Find_Never_BBs(MEM_POOL *pool)
 *	Return the set of BBs that can be inferred are never executed
 *	as a result of NEVER frequency hint pragmas. The returned set
 *      is allocated from <pool>. If there are no NEVER BBs, then
 *      NULL is returned instead of an empty set.
 *
 *    void FREQ_View_CFG(const char *status)
 *	Use DaVinci to view a feedback/frequency annotated CFG.
 *	<status> is an informational string used to label the graph.
 *
 *    BOOL FREQ_Verify(const char *caller)
 *      Prints the block frequencies and edge probabilites of the
 *      current region into the trace file.  Also prints a "FAIL"
 *      message to the trace file if a consistency problem is found.
 *      <caller> is an ascii string to identify the caller.
 *      Return TRUE iff no errors are detected.
 *
 * ====================================================================
 * ====================================================================
 */

#ifndef	FREQ_INCLUDED
#define	FREQ_INCLUDED

#include "bb_set.h"

extern void FREQ_Compute_BB_Frequencies(void);

extern void FREQ_Print_BB_Note(
  BB *bb,
  FILE *file
);

extern void FREQ_Region_Initialize(void);

inline BOOL FREQ_Frequencies_Computed(void)
{
  extern BOOL FREQ_freqs_computed;
  return FREQ_freqs_computed;
}

extern BOOL WN_Is_Pointer(WN *wn);

inline BOOL FREQ_Match(float f1, float f2)
{
  float ratio;

  if (f1 == f2) return TRUE;

  /* Tolerance is 0.1% of (f1+f2).  When (f1+f2) is zero, we'll
   * always return FALSE, but that's desirable since negative
   * freqs don't make sense.
   */
  ratio = (f1-f2)/(f1+f2);
  return ratio <= 0.001 && ratio >= -0.001;
}

extern BOOL FREQ_Check_Consistency(const char *caller);

extern BB_SET *FREQ_Find_Never_BBs(MEM_POOL *pool);

extern void FREQ_Incorporate_Feedback(const WN* entry);

extern void FREQ_View_CFG(const char *status);

extern BOOL FREQ_Verify(const char *caller);

#endif /* FREQ_INCLUDED */
