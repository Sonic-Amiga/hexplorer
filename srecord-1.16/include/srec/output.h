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
// MANIFEST: interface definition for lib/srec/output.cc
//

#ifndef INCLUDE_SREC_OUTPUT_H
#define INCLUDE_SREC_OUTPUT_H

#pragma interface "srec_output"

#include <stdarg.h>
#include <string>
using namespace std;
#include <format_printf.h>

class srec_record; // forward

/**
  * The srec_output class is used to represent an abstract output vector.
  * It could be a file, it could be a filter first, before it reaches
  * a file.
  */
class srec_output
{
public:
    /**
      * The destructor.
      */
    virtual ~srec_output();

    /**
      * The write method is used to write a recordonto an output.
      * Derived classes must implement this method.
      */
    virtual void write(const srec_record &) = 0;

    /**
      * The write_header method is used to write a header record
      * to the output.  If no record is specified, a default
      * record will be supplied.  The write method will be called.
      */
    virtual void write_header(const srec_record * = 0);

    /**
      * The write_data method is used to write data to the output.
      * A suitable data record wil be produced.  The write method
      * will be called.
      */
    virtual void write_data(unsigned long, const void *, size_t);

    /**
      * The write_start_address method is used to write a start
      * address record to the output.  If no record is specified,
      * a default record will be supplied.	The write method will
      * be called.
      */
    virtual void write_start_address(const srec_record * = 0);

    /**
      * The set_line_length method is used to set the maximum
      * length of an output line, for those formats for which
      * this is a meaningful concept, and the line length is at
      * all controllable.  Derived classes must implement this method.
      */
    virtual void line_length_set(int) = 0;

    /**
      * The address_length_set method is used to set the minimum
      * number of bytes to be written for addresses in the output,
      * for those formats for which this is a meaningful concept, and
      * the address length is at all controllable.	Derived classes
      * must implement this method.
      */
    virtual void address_length_set(int) = 0;

    /**
      * The preferred_block_size_get method is used to get the
      * proferred block size of the output fformat.  Often, but not
      * always, influenced by the line_lebgth_set method.  Derived
      * classes must implement this method.
      */
    virtual int preferred_block_size_get() const = 0;

    /**
      * The fatal_error method is used to report fatal errors.
      * The `fmt' string is in the same style a standard C printf
      * function.  It calls the fatal_error_v method.  This method
      * does not return.
      */
    virtual void fatal_error(const char *fmt, ...) const
							FORMAT_PRINTF(2, 3);
    /**
      * The fatal_error_v method is used to report fatal errors.
      * The `fmt' string is in the same style a standard C vprintf
      * function.  It calls ::exit.  This method does not return.
      */
    virtual void fatal_error_v(const char *fmt, va_list ap) const;

    /**
      * The fatal_error_errno method is used to report fatal errors,
      * and append the string equivalent of errno.  The `fmt' string
      * is in the same style a standard C printf function.  It calls
      * ::exit().  This method does not return.
      */
    virtual void fatal_error_errno(const char *fmt, ...) const
							FORMAT_PRINTF(2, 3);
    /**
      * The fatal_error_errno_v method is used to report fatal
      * errors.  The `fmt' string is in the same style a standard C
      * vprintf function.  It calls the ::exit function.
      * This method does not return.
      */
    virtual void fatal_error_errno_v(const char *fmt, va_list ap) const;

    /**
      * The warning method is used to likely but non-fatal errors.
      * The `fmt' string is in the same style a standard C printf
      * function.  It calls the warning_v method.
      */
    virtual void warning(const char *fmt, ...) const
							FORMAT_PRINTF(2, 3);
    /**
      * The warning_v method is used to report likely but non-fatal
      * errors.  The `fmt' string is in the same style a standard
      * C vprintf function.
      */
    virtual void warning_v(const char *fmt, va_list ap) const;

    /**
      * The filename method is used to determine the name of the
      * output file.  It is used for the various error messages.
      * Derived classes must implement this method.
      */
    virtual const string filename() const = 0;

    /**
      * The notify_upper_bound method is used to notify the output class
      * of the upper bound (highest address plus one) of the output
      * to come.  Shall be called before the hread or any data records
      * are written.
      */
    virtual void notify_upper_bound(unsigned long addr);

protected:
    /**
      * The default constructor.  Only dervived classes may use.
      */
    srec_output();

private:
    /**
      * The copy constructor.  Do not use.
      */
    srec_output(const srec_output &);

    /**
      * The assignment operator.  Do not use.
      */
    srec_output &operator=(const srec_output &);
};

#endif // INCLUDE_SREC_OUTPUT_H
