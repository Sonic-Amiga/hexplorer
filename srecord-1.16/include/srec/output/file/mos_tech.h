//
//	srecord - manipulate eprom load files
//	Copyright (C) 2000-2002 Peter Miller;
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
// MANIFEST: interface definition for lib/srec/output/file/mos_tech.cc
//

#ifndef INCLUDE_SREC_OUTPUT_FILE_MOS_TECH_H
#define INCLUDE_SREC_OUTPUT_FILE_MOS_TECH_H

#pragma interface "srec_output_file_mos_tech"

#include <srec/output/file.h>

/**
  * The srec_output_file_mos_tech class is used to write a Mos Technologies
  * formatted file.
  */
class srec_output_file_mos_tech:
	public srec_output_file
{
public:
	/**
	  * The destructor.
	  */
	virtual ~srec_output_file_mos_tech();

	/**
	  * The default constructor.  The input will be read from the
	  * standard input.
	  */
	srec_output_file_mos_tech();

	/**
	  * A constructor.  The input will be read from the named file
	  * (or the standard input if the file name is "-").
	  */
	srec_output_file_mos_tech(const char *filename);

	// See base class for documentation.
	void write(const srec_record &);

	// See base class for documentation.
	virtual void line_length_set(int);

	// See base class for documentation.
	virtual void address_length_set(int);

	// See base class for documentation.
	virtual int preferred_block_size_get() const;

private:
	/**
	  * The pref_block_size is used to remember the preferred
	  * block size.  Set by the line_length_set() method.  Read by
	  * the preferred_block_size_get() method.
	  */
	int pref_block_size;

	/**
	  * The write_inner method is used to write a single line (record)
	  * to the file.  Use by the write() method.
	  */
	void write_inner(int type, unsigned long addr, int addr_len,
		const void *data, int data_len);

	/**
	  * The copy constructor.  Do not use.
	  */
	srec_output_file_mos_tech(const srec_output_file_mos_tech &);

	/**
	  * The assignment operator.  Do not use.
	  */
	srec_output_file_mos_tech &operator=(const srec_output_file_mos_tech &);
};

#endif // INCLUDE_SREC_OUTPUT_FILE_MOS_TECH_H
