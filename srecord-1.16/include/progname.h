//
//	srecord - manipulate eprom load files
//	Copyright (C) 1998, 1999, 2002, 2003 Peter Miller;
//	All rights reserved.
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
//
// MANIFEST: interface definition for lib/common/progname.cc
//

#ifndef INCLUDE_PROGNAME_H
#define INCLUDE_PROGNAME_H

/**
  * The progname_set function is used by main() to set the name of the
  * currently executing programme.
  */
void progname_set(char *);

/**
  * The progname_get function is used to retrieve the name of the
  * currently executing programme.  Used by error messages.
  */
const char *progname_get();

#endif // INCLUDE_PROGNAME_H
