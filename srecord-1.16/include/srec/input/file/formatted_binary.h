//
//	srecord - manipulate eprom load files
//	Copyright (C) 2003 Peter Miller;
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
// MANIFEST: interface defn for include/srec/input/file/formatted_binary.cc
//

#ifndef INCLUDE_SREC_INPUT_FILE_FORMATTED_BINARY_H
#define INCLUDE_SREC_INPUT_FILE_FORMATTED_BINARY_H

#pragma interface "srec_input_file_formatted_binary"

#include <srec/input/file.h>

/**
  * The srec_input_file_formatted_binary class is used to represent the
  * parse state when reading a file in the Formatted Binary format.
  */
class srec_input_file_formatted_binary:
    public srec_input_file
{
public:
    /**
      * The destructor.
      */
    virtual ~srec_input_file_formatted_binary();

    /**
      * The constructor.
      */
    srec_input_file_formatted_binary(const char *filename);

    // See base class for documentation.
    int read (srec_record &);

    // See base class for documentation.
    const char *get_file_format_name() const;

    // See base class for documentation.
    const char *mode() const;

private:
    /**
      * The header_seen instance variable is used to remember whether
      * the file header has been seen yet.
      */
    bool header_seen;

    /**
      * The upper_bound instance variable is used to remember how long
      * the header said the file was going to be.
      */
    unsigned long upper_bound;

    /**
      * The address instance variable is used to remember where we are
      * up to in extracting the data from the file.
      */
    unsigned long address;

    /**
      * The trailer_seen instance variable is used to remember whether
      * the file trailer has been seen yet.
      */
    bool trailer_seen;

    /**
      * The check_sum instance variable is used to remember the simple
      * sum of the data bytes in the file.
      */
    unsigned short check_sum;

    /**
      * The default constructor.
      */
    srec_input_file_formatted_binary();

    /**
      * The copy constructor.
      */
    srec_input_file_formatted_binary(const srec_input_file_formatted_binary &);

    /**
      * The assignment operator.
      */
    srec_input_file_formatted_binary &operator=(
	const srec_input_file_formatted_binary &);
};

#endif // INCLUDE_SREC_INPUT_FILE_FORMATTED_BINARY_H
