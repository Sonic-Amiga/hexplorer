//
//	srecord - manipulate eprom load files
//	Copyright (C) 2002 Peter Miller;
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
// MANIFEST: interface definition for include/srec/output/file/os65v.cc
//

#ifndef INCLUDE_SREC_OUTPUT_FILE_OS65V_H
#define INCLUDE_SREC_OUTPUT_FILE_OS65V_H

#pragma interface "srec_output_file_os65v"

#include <srec/output/file.h>

/**
  * The srec_output_file_os65v class is used to represent the file state
  * of an OS65V output file, in OS65V format.
  */
class srec_output_file_os65v:
    public srec_output_file
{
public:
    /**
      * The destructor.
      */
    virtual ~srec_output_file_os65v();

    /**
      * The constructor.
      */
    srec_output_file_os65v(const char *);

    // See base class for documentation
    void write(const srec_record &);

    // See base class for documentation
    void line_length_set(int);

    // See base class for documentation
    void address_length_set(int);

    // See base class for documentation
    int preferred_block_size_get() const;

    // See base class for documentation
    const char *mode(void) const;

private:
    /**
      * The address instance variable is used to remember the current
      * file location.
      */
    unsigned long address;

    /**
      * The output mode is either address mode ('.') or data mode ('/').
      * Initially the file is in an unidentified mode (NUL).
      */
    char state;

    /**
      * The seen_start_address instance variable is used to remember
      * whether or not a start address has been seen.  Normally this
      * is only given at the end of data.  This variable is used to
      * determine whether to emit thre "return to monitor" sequence,
      * if a "GO" command has not been issued.
      */
    bool seen_start_address;

    /**
      * The default constructor.  Do not use.
      */
    srec_output_file_os65v();

    /**
      * Copy constructor.  Do not use.
      */
    srec_output_file_os65v(const srec_output_file_os65v &);

    /**
      * Assignment operator.  Do not use.
      */
    srec_output_file_os65v &operator=(const srec_output_file_os65v &);
};

#endif // INCLUDE_SREC_OUTPUT_FILE_OS65V_H
