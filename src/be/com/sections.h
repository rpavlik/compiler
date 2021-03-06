/*
 *  Copyright (C) 2008 PathScale, LLC.  All Rights Reserved.
 */

/*
 *  Copyright (C) 2007 QLogic Corporation.  All Rights Reserved.
 */

/*
 * Copyright 2005, 2006 PathScale, Inc.  All Rights Reserved.
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


/*
 * target-specific info about elf sections.
 */

#ifndef sections_INCLUDED 
#define sections_INCLUDED

#include "defs.h"
#include "symtab.h"

/*
 * data layout keeps a table of predefined sections,
 * with info about the sections.
 */
typedef UINT16 SECTION_IDX;

enum _sec_kind {
  _SEC_UNKNOWN =0,
  _SEC_TEXT,
  _SEC_DATA,
  _SEC_SDATA,
#ifdef KEY
  _SEC_LDATA_MIPS_LOCAL,	// bug 12619
#endif
  _SEC_LDATA,
  _SEC_RDATA,
  _SEC_SRDATA,
  _SEC_LIT4,
  _SEC_LIT8,
  _SEC_LIT16,
  _SEC_BSS,
  _SEC_SBSS,
  _SEC_LBSS,
  _SEC_GOT,
  _SEC_CPLINIT,
  _SEC_EH_REGION,
  _SEC_EH_REGION_SUPP,
  _SEC_DISTR_ARRAY,
#ifdef KEY
  _SEC_DATA_REL,
  _SEC_DATA_REL_RO,
#endif
  _SEC_THREAD_PRIVATE_FUNCS,
  _SEC_INDEX_MAX,
  };

typedef struct {
  SECTION_IDX	id;		/* The partition ID */
  ST           *block;
  UINT32	flags;		/* elf attributes */
  UINT32	etype;		/* elf type */
  UINT32	entsize;	/* elf entsize */
  INT64		max_sec_size;	/* Maximum size of this section */
  const char	*name;		/* Print name */
  INT32         pad_size;       /* size to pad, neg ==> post pad */
} SECTION;

extern SECTION Sections[_SEC_INDEX_MAX];

#define SEC_id(s)		(Sections[s].id)
#define SEC_block(s)		(Sections[s].block)
#define SEC_flags(s)		(Sections[s].flags)
#define SEC_type(s)		(Sections[s].etype)
#define SEC_entsize(s)		(Sections[s].entsize)
#define SEC_max_sec_size(s)	(Sections[s].max_sec_size)
#define SEC_name(s)		(Sections[s].name)
#define SEC_pad_size(s)         (Sections[s].pad_size)

inline INT Get_Section_Elf_Flags (SECTION_IDX s)
{
        return SEC_flags(s);
}
inline INT Get_Section_Elf_Type (SECTION_IDX s)
{
        return SEC_type(s);
}
inline INT Get_Section_Elf_Entsize (SECTION_IDX s)
{
        return SEC_entsize(s);
}

extern SECTION_IDX Corresponding_Short_Section (SECTION_IDX sec);
extern BOOL SEC_is_gprel (SECTION_IDX sec);
extern BOOL SEC_is_merge (SECTION_IDX sec);
extern BOOL SEC_is_exec (SECTION_IDX sec);
extern BOOL SEC_is_nobits (SECTION_IDX sec);
extern BOOL Is_String_Literal (ST *st);

#endif
