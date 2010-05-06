/*
 * Copyright 2004, 2005, 2006 PathScale, Inc.  All Rights Reserved.
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


//  isa_properties_gen.cxx
/////////////////////////////////////
//
//  Generate an interface for specifying properties (attributes) for 
//  various instructions in the ISA.
//
/////////////////////////////////////
//
//  $Revision: 1.5 $
//  $Date: 04/12/21 14:57:26-08:00 $
//  $Author: bos@eng-25.internal.keyresearch.com $
//  $Source: /home/bos/bk/kpro64-pending/common/targ_info/generate/SCCS/s.isa_properties_gen.cxx $


#include <stddef.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <assert.h>
#include <list>
#include <vector>
#include "topcode.h"
#include "gen_util.h"
#include "isa_properties_gen.h"


struct isa_property {
  const char* name;         // Name given for documentation and debugging
  int bit_position;         // bit postion in flag word
  std::vector <bool> members;    // set of opcodes that have this property
};

// special values for bit_position above:
enum {
  BIT_POS_ALL = -1,         // all members have this property
  BIT_POS_NONE = -2         // no members have this property
};

static std::list<ISA_PROPERTY> properties; // All the properties

static const char * const interface[] = {
  "/* ====================================================================",
  " * ====================================================================",
  " *",
  " * Description:",
  " *",
  " *   A description of the properties (attributes) for the instructions",
  " *   in the ISA. The description exports the following:",
  " *",
  " *   BOOL TOP_is_xxx(TOP topcode)",
  " *       Return true/false if 'topcode' has/does-not-have the property",
  " *       'xxx'.",
  " *",
  " * ====================================================================",
  " * ====================================================================",
  " */",
  NULL
};


/////////////////////////////////////
void ISA_Properties_Begin( const char* /* name */ )
/////////////////////////////////////
//  See interface description.
/////////////////////////////////////
{
}

/////////////////////////////////////
ISA_PROPERTY ISA_Property_Create( const char* name )
/////////////////////////////////////
//  See interface description.
/////////////////////////////////////
{
  ISA_PROPERTY result = new isa_property;

  result->name = name;
  result->members = std::vector <bool> (TOP_count, false);

  properties.push_back(result);

  return result;
}

/////////////////////////////////////
void Instruction_Group( ISA_PROPERTY property, ... )
/////////////////////////////////////
//  See interface description.
/////////////////////////////////////
{
  va_list ap;
  TOP opcode;

  va_start(ap,property);
  while ( (opcode = static_cast<TOP>(va_arg(ap,int))) != TOP_UNDEFINED ) {
    property->members[(int)opcode] = true;
  }
  va_end(ap);
}

/////////////////////////////////////
void ISA_Properties_End(void)
/////////////////////////////////////
//  See interface description.
/////////////////////////////////////
{
  std::list<ISA_PROPERTY>::iterator isi;
  int isa_property_count;	// How many non-constant properties?
  int code;

#define FNAME "targ_isa_properties"
  char filename[1000];
  sprintf (filename, "%s.h", FNAME);
  FILE* hfile = fopen(filename, "w");
  sprintf (filename, "%s.c", FNAME);
  FILE* cfile = fopen(filename, "w");
  sprintf (filename, "%s.Exported", FNAME);
  FILE* efile = fopen(filename, "w");

  fprintf(cfile,"#include \"%s.h\"\n\n", FNAME);

  Emit_Header (hfile, "targ_isa_properties", interface);

  isa_property_count = 0;  
  for ( isi = properties.begin(); isi != properties.end(); ++isi ) {
    ISA_PROPERTY property = *isi;
    bool member;
    bool prev_member = property->members[0];
    for (code = 1; code < TOP_count; code++) {
      member = property->members[code];
      if (member != prev_member) break;
    }
    if (member != prev_member) {
      property->bit_position = isa_property_count++;
    } else {
      property->bit_position = member ? BIT_POS_ALL : BIT_POS_NONE;
    }
  }

  const char *int_type;
  const char *int_suffix;
  int int_size;
  if (isa_property_count <= 8) {
    int_type = "mUINT8";
    int_suffix = "";
    int_size = 8;
  } else if (isa_property_count <= 16) {
    int_type = "mUINT16";
    int_suffix = "";
    int_size = 16;
  } else if (isa_property_count <= 32) {
    int_type = "mUINT32";
    int_suffix = "U";
    int_size = 32;
  } else {
    assert (isa_property_count <= 64);
    int_type = "mUINT64";
    int_suffix = "ULL";
    int_size = 64;
  }
  fprintf (hfile, "extern const %s ISA_PROPERTIES_flags[];\n\n", int_type);
  fprintf (efile, "ISA_PROPERTIES_flags\n");
  fprintf (cfile,"const %s ISA_PROPERTIES_flags[] = {\n", int_type);

  for (code = 0; code < TOP_count; code++) {
    unsigned long long flag_value = 0;

    for ( isi = properties.begin(); isi != properties.end(); ++isi ) {
      ISA_PROPERTY property = *isi;
      if (property->bit_position >= 0 && property->members[code]) {
	flag_value |= (1ULL << property->bit_position);
      }
    }
    fprintf (cfile, "  0x%0*llx%s, /* %s:", int_size / 4,
					    flag_value, 
					    int_suffix,
					    TOP_Name((TOP)code));
    for ( isi = properties.begin(); isi != properties.end(); ++isi ) {
      ISA_PROPERTY property = *isi;
      if (property->members[code]) fprintf (cfile, " %s", property->name);
    }
    fprintf (cfile, " */\n");
  }
  fprintf (cfile, "};\n");

  for ( isi = properties.begin(); isi != properties.end(); ++isi ) {
    ISA_PROPERTY property = *isi;
    int bit_position = property->bit_position;
    if (bit_position >= 0) {
      fprintf (hfile, "#define PROP_%-16s 0x%llx%s\n", 
		      property->name, 
		      (1ULL << bit_position),
		      int_suffix);
    }
  }

  fprintf (hfile, "\n\n");
  for ( isi = properties.begin(); isi != properties.end(); ++isi ) {
    ISA_PROPERTY property = *isi;
    int bit_position = property->bit_position;
    if (bit_position < 0) {
      fprintf (hfile, "#define TOP_is_%s(t)\t (%s)\n",
		      property->name, 
		      bit_position == BIT_POS_ALL ? "TRUE" : "FALSE");
    } else {
      fprintf (hfile, "#define TOP_is_%s(t)\t (ISA_PROPERTIES_flags[(INT)t] & PROP_%s)\n",
		      property->name, 
		      property->name);
    }
  }

  Emit_Footer (hfile);
}
