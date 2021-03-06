//
//	srecord - manipulate eprom load files
//	Copyright (C) 1998, 1999, 2001-2003 Peter Miller;
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
// MANIFEST: functions to impliment the srec_output class
//

#pragma implementation "srec_output"

#include <errno.h>
#include <string.h>
#include <iostream>
using namespace std;

#include <cstdio>
#include <cstdarg>

#include <srec/output.h>
#include <srec/record.h>


srec_output::srec_output()
{
}


srec_output::srec_output(const srec_output &)
{
	fatal_error("bug (%s, %d)", __FILE__, __LINE__);
}


srec_output &
srec_output::operator=(const srec_output &)
{
	fatal_error("bug (%s, %d)", __FILE__, __LINE__);
	return *this;
}


srec_output::~srec_output()
{
}


void
srec_output::fatal_error(const char *fmt, ...)
	const
{
	va_list ap;
	va_start(ap, fmt);
	fatal_error_v(fmt, ap);
	va_end(ap);
}


void
srec_output::fatal_error_v(const char *fmt, va_list ap)
	const
{
	cout.flush();
	cerr << filename() << ": ";
	char buf[1024];
	vsnprintf(buf, sizeof(buf), fmt, ap);
	cerr << buf << endl;
	cerr.flush();
	exit(1);
}


void
srec_output::fatal_error_errno(const char *fmt, ...)
	const
{
	va_list ap;
	va_start(ap, fmt);
	fatal_error_errno_v(fmt, ap);
	va_end(ap);
}


void
srec_output::fatal_error_errno_v(const char *fmt, va_list ap)
	const
{
	int n = errno;
	cout.flush();
	cerr << filename() << ": ";
	char buf[1024];
	vsnprintf(buf, sizeof(buf), fmt, ap);
	cerr << buf;
	cerr << ": " << strerror(n) << " [" << n << "]" << endl;
	cerr.flush();
	exit(1);
}


void
srec_output::warning(const char *fmt, ...)
	const
{
	va_list ap;
	va_start(ap, fmt);
	warning_v(fmt, ap);
	va_end(ap);
}


void
srec_output::warning_v(const char *fmt, va_list ap)
	const
{
	cout.flush();
	cerr << filename() << ": warning: ";
	char buf[1024];
	vsnprintf(buf, sizeof(buf), fmt, ap);
	cerr << buf << endl;
	cerr.flush();
}


void
srec_output::write_header(const srec_record *rp)
{
    if (rp)
    {
	// Make sure we are writing a header record
	srec_record record(*rp);
	record.set_type(srec_record::type_header);
	write(record);
    }
    else
    {
	//
	// This is the default header record.
	// If you want to change it, this is the place.
	//
	static char hdr[] = "http://srecord.sourceforge.net/";
	srec_record record(srec_record::type_header, (srec_record::address_t)0,
	    (const srec_record::data_t *)hdr, strlen(hdr));
	write(record);
    }
}


void
srec_output::write_data(unsigned long address, const void *data, size_t length)
{
	const srec_record::data_t *data_p = (const srec_record::data_t *)data;
	size_t block_size = preferred_block_size_get();
	while (length > 0)
	{
		int nbytes = (length > block_size ? block_size : length);
		srec_record record(srec_record::type_data, address, data_p,
			nbytes);
		write(record);
		address += nbytes;
		data_p += nbytes;
		length -= nbytes;
	}
}


void
srec_output::write_start_address(const srec_record *rp)
{
    if (rp)
    {
	// Make sure we are writing a start address record
	srec_record record(*rp);
	record.set_type(srec_record::type_start_address);
	write(record);
    }
    else
    {
	//
	// This is the default start address record.
	// If you want to change it, this is the place.
	//
	srec_record record(srec_record::type_start_address, 0, 0, 0);
	write(record);
    }
}


void
srec_output::notify_upper_bound(unsigned long)
{
}
