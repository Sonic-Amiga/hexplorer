//
//	srecord - manipulate eprom load files
//	Copyright (C) 2001-2003 Peter Miller;
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
// MANIFEST: functions to impliment the srec_input_file_fastload class
//

#pragma implementation "srec_input_file_fastload"

#include <srec/input/file/fastload.h>
#include <srec/record.h>
#include <cstring>


srec_input_file_fastload::srec_input_file_fastload(const char *filename) :
    srec_input_file(filename),
    seen_some_input(false),
    address(0)
{
}


srec_input_file_fastload::~srec_input_file_fastload()
{
}


int
srec_input_file_fastload::get_digit()
{
    int c = get_char();
    switch (c)
    {
    case 'A': return 0;
    case 'B': return 1;
    case 'C': return 2;
    case 'D': return 3;
    case 'E': return 4;
    case 'F': return 5;
    case 'G': return 6;
    case 'H': return 7;
    case 'I': return 8;
    case 'J': return 9;
    case 'K': return 10;
    case 'L': return 11;
    case 'M': return 12;
    case 'N': return 13;
    case 'O': return 14;
    case 'P': return 15;
    case 'Q': return 16;
    case 'R': return 17;
    case 'S': return 18;
    case 'T': return 19;
    case 'U': return 20;
    case 'V': return 21;
    case 'W': return 22;
    case 'X': return 23;
    case 'Y': return 24;
    case 'Z': return 25;
    case 'a': return 26;
    case 'b': return 27;
    case 'c': return 28;
    case 'd': return 29;
    case 'e': return 30;
    case 'f': return 31;
    case 'g': return 32;
    case 'h': return 33;
    case 'i': return 34;
    case 'j': return 35;
    case 'k': return 36;
    case 'l': return 37;
    case 'm': return 38;
    case 'n': return 39;
    case 'o': return 40;
    case 'p': return 41;
    case 'q': return 42;
    case 'r': return 43;
    case 's': return 44;
    case 't': return 45;
    case 'u': return 46;
    case 'v': return 47;
    case 'w': return 48;
    case 'x': return 49;
    case 'y': return 50;
    case 'z': return 51;
    case '0': return 52;
    case '1': return 53;
    case '2': return 54;
    case '3': return 55;
    case '4': return 56;
    case '5': return 57;
    case '6': return 58;
    case '7': return 59;
    case '8': return 60;
    case '9': return 61;
    case ',': return 62;
    case '.': return 63;
    }
    get_char_undo(c);
    return -1;
}


unsigned long
srec_input_file_fastload::get_number(int min_digits, int max_digits)
{
    unsigned long result = 0;
    for (int ndigits = 0; ndigits < max_digits; ++ndigits)
    {
	int c = get_digit();
	if (c < 0)
	{
	    if (ndigits < min_digits)
	    {
	       	fatal_error("base-64 number expected (%d < %d)",
		    ndigits, min_digits);
	    }
	    break;
	}
	result = (result << 6) | c;
    }
    return result;
}


void
srec_input_file_fastload::expect_white_space()
{
    switch (peek_char())
    {
    case -1:
    case ' ':
    case '\t':
    case '\r':
    case '\n':
    case '/':
	break;

    default:
	fatal_error("white space expected");
    }
}


int
srec_input_file_fastload::read_inner(srec_record &record)
{
    unsigned long n;
    unsigned char data[srec_record::max_data_length];
    unsigned long data_address = address;
    srec_record::type_t type;
    int data_length = 0;
    unsigned char the_byte;
    for (;;)
    {
	switch (peek_char())
	{
	case -1:
	    return 0;

	case ' ':
	case '\t':
	case '\n':
	case '\r':
	    get_char();
	    break;

	case '/':
	    if (data_length > 0)
	    {
		got_a_record:
		record =
		    srec_record
		    (
			srec_record::type_data,
			data_address,
			data,
			data_length
		    );
		return 1;
	    }
	    get_char();
	    switch (get_char())
	    {
	    case 'A':
		address = get_number(1, 6);
		expect_white_space();
		data_address = address;
		break;

	    case 'B':
		the_byte = get_number(1, 6);
		data[data_length++] = the_byte;
		checksum_add(the_byte);
		expect_white_space();
		address++;
		// assert(data_length == 1);
		break;

	    case 'C':
		n = get_number(1, 6);
		if ((int)n != checksum_get16())
		{
		    fatal_error("checksum mismatch (%04X != %04X)",
			(int)n, checksum_get16());
		}
		expect_white_space();
		break;

	    case 'E':
		get_number(1, 6);
		seek_to_end();
		type = srec_record::type_start_address;
		record = srec_record(type, address, 0, 0);
		return 1;

	    case 'K':
		get_number(1, 6);
		expect_white_space();
		checksum_reset();
		break;

	    case 'S':
		// Ignore symbols
		for (;;)
		{
		    int c = get_char();
		    if (c < 0)
		       	fatal_error("end-of-input in symbol");
		    if (c == ',')
		       	break;
		}
		get_number(1, 6);
		expect_white_space();
		break;
											    case 'Z':
		n = get_number(1, 6);
		expect_white_space();
		if (n >= srec_record::max_data_length)
		    fatal_error("clearing too many bytes (%lu)", n);
		memset(data, 0, n);
		type = srec_record::type_data;
		record = srec_record(type, address, data, n);
		address += n;
		return 1;

	    default:
		fatal_error("unknown command");
	    }
	    break;

	default:
	    if (data_length + 3 > srec_record::max_data_length)
		goto got_a_record;
	    n = get_number(4, 4);
	    the_byte = n >> 16;
	    data[data_length++] = the_byte;
	    checksum_add(the_byte);
	    the_byte = n >> 8;
	    data[data_length++] = the_byte;
	    checksum_add(the_byte);
	    the_byte = n;
	    data[data_length++] = the_byte;
	    checksum_add(the_byte);
	    address += 3;
	    break;
	}
    }
}


int
srec_input_file_fastload::read(srec_record &record)
{
    if (!read_inner(record))
    {
	if (!seen_some_input)
    	    fatal_error("file contains no data");
	return 0;
    }
    seen_some_input = true;
    return 1;
}


const char *
srec_input_file_fastload::get_file_format_name()
    const
{
    return "LSI Logic Fast Load";
}
