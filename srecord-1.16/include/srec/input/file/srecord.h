//
//	srecord - manipulate eprom load files
//	Copyright (C) 1998-2000, 2002, 2003 Peter Miller;
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
// MANIFEST: interface definition for lib/input/file/srecord.cc
//

#ifndef INCLUDE_INPUT_FILE_SRECORD_H
#define INCLUDE_INPUT_FILE_SRECORD_H

#pragma interface "srec_input_file_srecord"

#include <srec/input/file.h>

/**
  * The srec_input_file_srecord class is used to represent the parse
  * state of a Motorola S-Record formatted input file.
  */
class srec_input_file_srecord:
    public srec_input_file
{
public:
    /**
      * The destructor.
      */
    virtual ~srec_input_file_srecord();

    /**
      * The constructor.
      */
    srec_input_file_srecord(const char *filename);

    // See base class for documentation.
    int read(srec_record &);

    // See base class for documentation.
    const char *get_file_format_name() const;

private:
    /**
      * The data_count instance variable is used to remember the number
      * of data lines has occurred fo far in the input file.
      */
    unsigned long data_count;

    /**
      * The read_inner method is used to read a record of input.
      * The read method is a wrapper around this method.
      */
    int read_inner(srec_record &);

    /**
      * The garbage_warning instance variable is used to remember whether
      * or not a warning about garbage input lines has been issued yet.
      */
    bool garbage_warning;

    /**
      * The seen_some_input instance variable is used to remember where
      * any data has been seen in this file yet.
      */
    bool seen_some_input;

    /**
      * The header_seen instance variable is used to remember whether
      * or not the header record has been seen yet.
      */
    bool header_seen;

    /**
      * The termination_seen instance variable is used to remember
      * whether or not the termination record has been seen yet.
      */
    bool termination_seen;

    /**
      * The default constructor.  Do not use.
      */
    srec_input_file_srecord();

    /**
      * The copy constructor.  Do not use.
      */
    srec_input_file_srecord(const srec_input_file_srecord &);

    /**
      * The assignment operator.  Do not use.
      */
    srec_input_file_srecord &operator=(const srec_input_file_srecord &);
};

#endif // INCLUDE_INPUT_FILE_SRECORD_H
