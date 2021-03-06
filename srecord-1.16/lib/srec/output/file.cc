//
//	srecord - manipulate eprom load files
//	Copyright (C) 1998-2002 Peter Miller;
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
// MANIFEST: functions to impliment the srec_output_file class
//

#pragma implementation "srec_output_file"

#include <cstdio>
#include <errno.h>
#include <iostream>
using namespace std;
#include <sys/types.h>
#include <sys/stat.h>

#include <srec/output/file.h>


bool srec_output_file::data_only_flag = false;


srec_output_file::srec_output_file() :
	file_name("standard output"),
	line_number(1),
	vfp(0),
	checksum(0),
	position(0),
	is_regular(true)
{
	vfp = stdout;
	set_is_regular();
}


srec_output_file::srec_output_file(const srec_output_file &)
{
	fatal_error("bug (%s, %d)", __FILE__, __LINE__);
}


const char *
srec_output_file::mode()
	const
{
	return "w";
}


srec_output_file::srec_output_file(const char *fn) :
	file_name(fn),
	line_number(1),
	vfp(0),
	checksum(0),
	position(0),
	is_regular(true)
{
	if (file_name == string("-"))
	{
		file_name = "standard output";
		vfp = stdout;
		set_is_regular();
	}
	else
	{
		//
		// The call to fopen is deferred until the constructor has
		// completed.  This is so that the virtual mode() method
		// is available (it isn't in the base class constructor).
		//
	}
}


srec_output_file &
srec_output_file::operator=(const srec_output_file &)
{
	fatal_error("bug (%s, %d)", __FILE__, __LINE__);
	return *this;
}


void *
srec_output_file::get_fp()
{
	if (!vfp)
	{
		//
		// The call to fopen is deferred until the constructor has
		// completed.  This is so that the virtual mode() method
		// is available (it isn't in the base class constructor).
		//
		vfp = fopen(file_name.c_str(), mode());
		if (!vfp)
			fatal_error_errno("open");
		set_is_regular();
	}
	return vfp;
}


srec_output_file::~srec_output_file()
{
	FILE *fp = (FILE *)get_fp();
	if (fflush(fp))
		fatal_error_errno("write");
	if (fp != stdout && fclose(fp))
		fatal_error_errno("close");
}


const string
srec_output_file::filename()
	const
{
	char buffer[20];
	sprintf(buffer, ": %d", line_number);
	return (file_name + buffer);
}


void
srec_output_file::put_char(int c)
{
	FILE *fp = (FILE *)get_fp();
	putc(c, fp);
	if (ferror(fp))
		fatal_error_errno("write");
	if (c == '\n')
		++line_number;
        ++position;
}


void
srec_output_file::put_nibble(int n)
{
	put_char("0123456789ABCDEF"[n & 15]);
}


void
srec_output_file::put_byte(unsigned char n)
{
	put_nibble(n >> 4);
	put_nibble(n);
	checksum_add(n);
}


void
srec_output_file::put_word(int n)
{
	put_byte(n >> 8);
	put_byte(n);
}


void
srec_output_file::put_3bytes(unsigned long n)
{
	put_byte(n >> 16);
	put_byte(n >> 8);
	put_byte(n);
}


int
srec_output_file::checksum_get()
{
	return (checksum & 0xFF);
}


int
srec_output_file::checksum_get16()
{
	return (checksum & 0xFFFF);
}


void
srec_output_file::checksum_reset()
{
	checksum = 0;
}

void
srec_output_file::checksum_add(unsigned char n)
{
	checksum += n;
}


void
srec_output_file::seek_to(unsigned long address)
{
	//
	// Seeking on non-regular files is problematic.  Avoid doing
	// this if possible.  (Usually we can, srec_cat emits records
	// in ascending address order.)
	//
	if (!is_regular)
	{
	    while (position < address)
		put_char(0);
	}
	if (address == position)
		return;

	//
	// We'll have to try a seek.
	//
	FILE *fp = (FILE *)get_fp();
	if (fseek(fp, address, 0) < 0)
		fatal_error_errno("seek %ld", address);
	position = address;
}


void
srec_output_file::put_string(const char *s)
{
	while (*s)
		put_char(*s++);
}


void
srec_output_file::put_stringf(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	char buffer[1000];
	vsprintf(buffer, fmt, ap);
	va_end(ap);
	put_string(buffer);
}


void
srec_output_file::data_only()
{
	data_only_flag = true;
}


void
srec_output_file::set_is_regular()
{
    FILE *fp = (FILE *)vfp;
    struct stat st;
    is_regular = fstat(fileno(fp), &st) == 0 && S_ISREG(st.st_mode);
}
