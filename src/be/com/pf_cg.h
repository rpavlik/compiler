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


#ifndef pf_cg_INCLUDED
#define pf_cg_INCLUDED

#include "wn.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct pf_pointer {
  WN* wn_pref_1L;
  WN* wn_pref_2L;
  WN_OFFSET distance_1L;
  WN_OFFSET distance_2L;
  mINT16 lrnum_1L;
  mINT16 lrnum_2L;
  mUINT32 flag;
} PF_POINTER;

/* flag structure 
     bits 8-11:  L1 confidence
     bits 12-15: L2 confidence
*/
#define GET_CONF_1L(flag)   (((flag) >> 8) & 0xf)
#define SET_CONF_1L(flag,x) flag=(((flag) & 0xfffff0ff) | (((x) & 0xf) << 8))

#define GET_CONF_2L(flag)   (((flag) >> 12) & 0xf)
#define SET_CONF_2L(flag,x) flag=(((flag) & 0xffff0fff) | (((x) & 0xf) << 12))

#define PF_PTR_wn_pref_1L(pp) 	((pp)->wn_pref_1L)
#define PF_PTR_wn_pref_2L(pp) 	((pp)->wn_pref_2L)
#define PF_PTR_distance_1L(pp) 	((pp)->distance_1L)
#define PF_PTR_distance_2L(pp) 	((pp)->distance_2L)
#define PF_PTR_lrnum_1L(pp) 	((pp)->lrnum_1L)
#define PF_PTR_lrnum_2L(pp) 	((pp)->lrnum_2L)
#define PF_PTR_flag(pp)		((pp)->flag)

#define PF_PTR_conf_1L(pp)	 (GET_CONF_1L(PF_PTR_flag(pp)))
#define PF_PTR_set_conf_1L(pp,x) (SET_CONF_1L(PF_PTR_flag(pp),x))

#define PF_PTR_conf_2L(pp)	 (GET_CONF_2L(PF_PTR_flag(pp)))
#define PF_PTR_set_conf_2L(pp,x) (SET_CONF_2L(PF_PTR_flag(pp),x))

/* Macros to access visited bit in pf_pointer. */
#define VISITED_BIT          0x1
#define VISIT_CM_BIT         0x2
#define VISIT_EM_BIT         0x4
#define AUTO_BIT             0x8
#define PF_PRUNED_BIT        0x10   // the prefetch is pruned by CG

#define VISITED(pp)          (((pp)->flag) & VISITED_BIT)
#define SET_VISITED(pp)      ((pp)->flag |=  VISITED_BIT)
#define RESET_VISITED(pp)    ((pp)->flag &= ~VISITED_BIT)

#define VISITED_CM(pp)       (((pp)->flag) & VISIT_CM_BIT)
#define SET_VISITED_CM(pp)   ((pp)->flag |=  VISIT_CM_BIT)
#define RESET_VISITED_CM(pp) ((pp)->flag &= ~VISIT_CM_BIT)

#define VISITED_EM(pp)       (((pp)->flag) & VISIT_EM_BIT)
#define SET_VISITED_EM(pp)   ((pp)->flag |=  VISIT_EM_BIT)
#define RESET_VISITED_EM(pp) ((pp)->flag &= ~VISIT_EM_BIT)

#define AUTO(pp)             (((pp)->flag) & AUTO_BIT)
#define SET_AUTO(pp)         ((pp)->flag |=  AUTO_BIT)
#define RESET_AUTO(pp)       ((pp)->flag &= ~AUTO_BIT)

#define PF_PRUNED(pp)        (((pp)->flag) & PF_PRUNED_BIT)
#define SET_PF_PRUNED(pp)    ((pp)->flag |=  PF_PRUNED_BIT)
#define RESET_PF_PRUNED(pp)  ((pp)->flag &= ~PF_PRUNED_BIT)

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* pf_cg_INCLUDED */
