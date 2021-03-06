//
//	srecord - manipulate eprom load files
//	Copyright (C) 2000, 2002, 2003 Peter Miller;
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
// MANIFEST: functions to impliment the srec_input_file_ascii_hex class
//

#pragma implementation "srec_input_file_ascii_hex"

#include <cctype>

#include <srec/input/file/ascii_hex.h>
#include <srec/record.h>


srec_input_file_ascii_hex::srec_input_file_ascii_hex(const char *filename) :
    srec_input_file(filename),
    garbage_warning(false),
    seen_some_input(false),
    address(0),
    state(state_initial)
{
}


srec_input_file_ascii_hex::~srec_input_file_ascii_hex()
{
    // check termination?
}


int
srec_input_file_ascii_hex::read_inner(srec_record &record)
{
    if (state == state_ignore)
	return 0;

    if (state == state_initial)
    {
	for (;;)
	{
	    int c = get_char();
	    if (c < 0)
	    {
		state = state_ignore;
		return 0;
	    }
	    if (c == 2)
	    {
		state = state_body;
		break;
	    }
	    if (!garbage_warning)
	    {
		warning("ignoring garbage lines");
		garbage_warning = true;
	    }
	}
    }

    for (;;)
    {
	int c = peek_char();
	if (c < 0)
	{
	    return 0;
	}
	if (isxdigit(c))
	{
	    unsigned char c = get_byte();
	    record = srec_record(srec_record::type_data, address, &c, 1);
	    int sep = get_char();
	    if (sep >= 0 && !isspace((unsigned char)sep))
		fatal_error("not execution character");
	    ++address;
	    switch (peek_char())
	    {
	    case '\'':
	    case ',':
	    case '%':
	    case ' ':
		// The documentation calls these an "execution" character.
		// Strictly speaking, the space isn't optional.
		get_char();
		break;
	    }
	    return 1;
	}
	c = get_char();
	switch (c)
	{
	case 3:
	    state = state_ignore;
	    return 0;

	case ' ':
	case '\t':
	case '\r':
	case '\n':
	case '\f':
	    break;

	default:
	    fatal_error("illegal character");

	case '$':
	    int command = get_char();
	    unsigned long value = 0;
	    for (;;)
	    {
		value = (value << 4) + get_nibble();
		int c = get_char();
		if (c == ',' || c == '.')
		    break;
		get_char_undo(c);
	    }
	    switch (command)
	    {
	    default:
		fatal_error("unknown command");

	    case 'A':
		address = value;
		break;

	    case 'S':
		unsigned short chk1 = checksum_get16();
		unsigned short chk2 = value & 0xFFFF;
		if (chk1 != chk2)
		{
		    fatal_error
		    (
			"checksum mismatch (%4.4X != %4.4X)",
			chk1,
			chk2
		    );
		}
		break;
	    }
	    break;
	}
    }
}


int
srec_input_file_ascii_hex::read(srec_record &record)
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
srec_input_file_ascii_hex::get_file_format_name()
    const
{
    return "Ascii Hex";
}
