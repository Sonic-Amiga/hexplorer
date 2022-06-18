//
//	srecord - manipulate eprom load files
//	Copyright (C) 1998, 1999, 2002 Peter Miller;
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
// MANIFEST: functions to impliment the progname class
//

#include <cstring>

#include <progname.h>


static char *progname;


void
progname_set(char *s)
{
	for (;;)
	{
		char *cp = strrchr(s, '/');
		if (!cp)
		{
			progname = s;
			break;
		}
		if (cp[1])
		{
			progname = cp + 1;
			break;
		}
		*cp = 0;
	}
}


const char *
progname_get()
{
	return (progname ? progname : "???");
}