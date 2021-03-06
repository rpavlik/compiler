/*
 * Copyright (C) 2008 Advanced Micro Devices, Inc.  All Rights Reserved.
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


#ifndef glob_INCLUDED
#define glob_INCLUDED
#ifdef __cplusplus
extern "C" {
#endif

/* ====================================================================
 * ====================================================================
 *
 * Module: glob.h
 * $Revision: 1.7 $
 * $Date: 04/12/21 14:57:22-08:00 $
 * $Author: bos@eng-25.internal.keyresearch.com $
 * $Source: /home/bos/bk/kpro64-pending/common/com/SCCS/s.glob.h $
 *
 * Revision history:
 *  15-Sep-89 - Original Version
 *  14-Mar-91 - Copied for TP/Muse
 *
 * Description:
 *
 * External interface to routines in the main Muse/TP compiler driver
 * module, basically very general purpose control and trace routines
 * which don't belong anywhere else.
 *
 * Also, external interface to global compiler data which doesn't
 * belong anywhere else.  See com/config.h for a general discussion
 * of where global configuration and option data declarations are
 * placed.
 *
 * All of this data is currently located in the process driver,
 * bedriver.c for the back end.
 *
 * NOTE:  This file is intended ultimately to be included in multiple
 * compiler processes.  We therefore attempt to distinguish between
 * items which are not process-specific and those which are, placing
 * the latter under the appropriate #ifdefs.
 *
 * ====================================================================
 * ====================================================================
 */

/* Dummy struct definitions to keep prototypes happy: */
struct bb;
struct symtab;

#ifdef __cplusplus
struct ST;
#else
struct st;
#endif

/* ====================================================================
 *
 * Non-process-specific data (intended for both front ends and back
 * end).
 *
 * ====================================================================
 */

extern char *Cur_PU_Name;	/* The current program unit name */
extern char *Orig_PU_Name;	/* The original program unit name in source */

/* Kind of the current program unit -- set similarly to Cur_PU_Name */
typedef enum {
  PU_UNKNOWN,
  PU_FUNCTION,
  PU_SUBROUTINE,
  PU_PROGRAM,
  PU_BLOCKDATA,
  PU_MAX_KIND		/* used for bounds checking */
} PU_KIND;
extern PU_KIND Cur_PU_Kind;

/* the var used to create output.h */
extern int pu_number;
extern int bb_number;
extern const char *Output_h_File_Name;      /* Cycle_Counting Output.h file */
extern FILE *Output_h_File;           /* Cycle counting output.h file */
extern FILE *Call_graph_file;	      /* Call graph file */
extern char * pu_string[1000]; 
extern char * bb_string[1000]; 

/* Current file names: */
extern char *Src_File_Name;	/* Source file */
extern char *Orig_Src_File_Name; /* Original source file passed to driver */
extern char *Cpp_File_Name;	/* cpp-preprocessed file */
extern char *Lst_File_Name;	/* Listing file */
extern char *Err_File_Name;	/* Error file */
extern char *Trc_File_Name;	/* Trace file */
extern char *Tlog_File_Name;	/* Transformation log file */
extern char *Irb_File_Name;	/* ACIR intermediate file */
extern char *IR_File_Name;	/* SGIR intermediate file */
extern char *Ipa_File_Name;	/* IPA file */
extern char *Asm_File_Name;	/* Assembly file */
extern char *Ipl_File_Name;     /* ipl output file */
extern char * Lai_File_Name;     /* LAI file */
extern char *call_graph_file_name; /* Function call graph file */
extern char *cord_output_file_name; /* Output file name after function layout */
extern char *cord_obj_file_name;  /* Object file name which will be reorder function layout */
extern char *Instrumentation_File_Name; /* instrumentation file */
extern char *Feedback_File_Name;/* feedback file produced from prof */
#ifndef MONGOOSE_BE
extern char *Lib_File_Name;	/* Program library file */
#endif
extern char *Lib_Lock_Name;	/* Program library lock file */
extern char *DSTdump_File_Name; /* Dwarf intermediate (i.e. DST) dump file */
extern char *Global_File_Name;	/* Global symbol table file */
extern void *Global_PU_Tree; /* global tree of all the pu's */

extern char *License_File_Name ; /* name of license file */

/* Current file handles if open, NULL otherwise: */
extern FILE *Src_File;		/* Source file */
extern FILE *Cpp_File;		/* cpp-preprocessed file */
extern FILE *Lst_File;		/* Listing file */
extern FILE *Err_File;		/* Error file */
extern FILE *Trc_File;		/* Trace file */
extern FILE *Tlog_File;		/* Transformation log file */
extern FILE *Irb_File;		/* ACIR intermediate file */
extern FILE *IR_File;		/* SGIR intermediate file */
extern FILE *Ipa_File;		/* IPA file */
extern FILE *Asm_File;		/* Assembly file */
extern FILE * Lai_File;          /* LAI file */
extern FILE *Lib_File;		/* Program library file */
extern FILE *Tim_File;		/* Timing info file (usually trace) */


#ifdef SPECMT_LT
  extern FILE *ExchangeFile;              /* exchange file for pseudo-specmt partition */
  extern FILE *LoopMappingFile;           /* loop mapping file for two pass compilation */
  #define SPECMT_FIRST_PASS   1
  #define SPECMT_SECOND_PASS  2
#endif

#ifdef MONGOOSE_BE
#define MMAPPED_FILE (1)	/* some components use mmap instead of
				   stream i/o, in those cases the *_File
				   will be set to MMAPPED_FILE to show that
				   the file is active  */
#endif /* MONGOOSE_BE */

extern INT32 Num_Source_Files;

/* Output requested: */
extern BOOL Assembly;		/* Assembly code */
extern BOOL Object_Code;	/* Object code */
extern BOOL Lai_Code;           /* Lai code */
extern BOOL Symbol_Table_Out;	/* Symbol table output (list or trace) */
extern BOOL Create_Cycle_Output; /* added by cbq */
extern BOOL Show_Progress;	/* Report progress to stdout */

/* Clean up and terminate program with given exit status: */
extern void Terminate ( INT status );

/* ====================================================================
 *
 * Back end process-specific data.
 *
 * ====================================================================
 */

#ifdef BACK_END

/* Front End process info: for consistency checking */
/* 
 * The first two are set in the front end, and examined in the back end.
 * They are also put out to the ipa file.  The third is only used in the 
 * back end to store the Version number of the front end.  (In the front
 * end, this information is stored in "Version".  In the back end, 
 * "Version" contains the version number of the *back* end.  Those
 * variables are initialized in <machine>/<process>/version.c
 */
extern INT32 Fe_Process_Id;
extern INT32 Fe_Process_Time;
extern char *Fe_Version;
extern BOOL Fe_Josie;	/* Did .B come from a Josie front end? */

/* Set FE process info -- defined in flags.c: */
extern void Set_Fe_Info ( INT32 pid, INT32 ptime, char *fe_version );

/* Set source file name: */
extern void Set_File_Name ( char *name );

/* Have the OP_REGCOPY operations been translated? */
extern BOOL Regcopies_Translated;

#else /* ! BACK_END */

/* Our front ends are never Josie front ends: */
# define Fe_Josie	FALSE

#endif /* BACK_END */

#ifdef __cplusplus
}
#endif
#endif /* glob_INCLUDED */
