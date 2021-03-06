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
// MANIFEST: functions to impliment the srec_input_file_emon52 class
//

#pragma implementation "srec_input_file_emon52"

#include <srec/input/file/emon52.h>
#include <srec/record.h>


srec_input_file_emon52::srec_input_file_emon52(const char *filename) :
    srec_input_file(filename)
{
}


srec_input_file_emon52::~srec_input_file_emon52()
{
}


void
srec_input_file_emon52::skip_white_space()
{
    for (;;)
    {
	int c = peek_char();
	if (c != ' ')
    	    return;
	get_char();
    }
}


int
srec_input_file_emon52::read(srec_record &record)
{
    //
    // This format has no termination record type, and no magic start
    // character.  So look ahead to see if there is anything more.
    //
    if (peek_char() < 0)
	return 0;

    //
    // Looks like there should be a record.  Read it all in.
    //
    int length = get_byte();
    skip_white_space();
    unsigned long address = get_word();
    if (get_char() != ':')
	fatal_error("colon expected");
    checksum_reset();
    unsigned char buffer[256];
    for (int j = 0; j < length; ++j)
    {
	skip_white_space();
	buffer[j] = get_byte();
    }
    skip_white_space();
    int csumX = checksum_get16();
    int csum = get_word();
    if (csumX != csum)
	fatal_error("checksum mismatch (%04X)", csumX);
    if (get_char() != '\n')
	fatal_error("end-of-line expected");

    srec_record::type_t type = srec_record::type_data;
    if (length == 0)
    {
	// Actually, this format does not have a termination
	// record, but a length of zero is otherwise meaningless.
	type = type = srec_record::type_start_address;
    }
    record = srec_record(type, address, buffer, length);
    return 1;
}


const char *
srec_input_file_emon52::get_file_format_name()
    const
{
    return "Elektor Monitor (EMON52)";
}
