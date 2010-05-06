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
   dwarf_addr_finder.h
   $Source$
   $Date$

   Defines user interface.

*/

/* return codes for functions
*/
#define DW_DLV_NO_ENTRY -1
#define DW_DLV_OK        0
#define DW_DLV_ERROR     1


/* the following are the 'section' number passed to the called-back
   function.
   The called-back application must translate this to the 
   appropriate elf section number/pointer.

   Putting this burden on the application avoids having to store
   the numbers in the Dwarf_Debug structure (thereby saving space
   for most consumers).
*/
#define DW_SECTION_INFO      0
#define DW_SECTION_FRAME     1
#define DW_SECTION_ARANGES   2
#define DW_SECTION_LINE      3
#define DW_SECTION_LOC       4  /* .debug_loc */

/* section is one of the above codes: it specifies a section.
   secoff is the offset in the dwarf section.
   existingAddr is the value at the specified offset (so the
	called back routine can sanity check the proceedings).
   It's up to the caller to know the size of an address (4 or 8)
   and update the right number of bytes.
*/
typedef int (*Dwarf_addr_callback_func)   (int /*section*/, 
        Dwarf_Off /*secoff*/, Dwarf_Addr /*existingAddr*/);

/* call this to do the work: it calls back thru cb_func
   once per each address to be modified.
   Once this returns you are done.
   Returns DW_DLV_OK if finished ok.
   Returns DW_DLV_ERROR if there was some kind of error, in which
	the dwarf error number was passed back thu the dwerr ptr.
   Returns DW_DLV_NO_ENTRY if there are no relevant dwarf sections,
	so there were no addresses to be modified (and none
	called back).
*/
int _dwarf_addr_finder(struct Elf * elf_file_ptr,
                Dwarf_addr_callback_func cb_func,
                int *dwerr);

