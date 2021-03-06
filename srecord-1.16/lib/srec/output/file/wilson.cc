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
// MANIFEST: functions to impliment the srec_output_file_wilson class
//

#pragma implementation "srec_output_file_wilson"

#include <srec/output/file/wilson.h>
#include <srec/record.h>


srec_output_file_wilson::srec_output_file_wilson()
	: srec_output_file(), pref_block_size(32)
{
}


srec_output_file_wilson::srec_output_file_wilson(const char *filename)
	: srec_output_file(filename), pref_block_size(32)
{
}


srec_output_file_wilson::srec_output_file_wilson(const srec_output_file_wilson &)
	: srec_output_file(), pref_block_size(32)
{
	fatal_error("bug (%s, %d)", __FILE__, __LINE__);
}


srec_output_file_wilson &
srec_output_file_wilson::operator=(const srec_output_file_wilson &)
{
	fatal_error("bug (%s, %d)", __FILE__, __LINE__);
	return *this;
}


srec_output_file_wilson::~srec_output_file_wilson()
{
	// check for termination record
}


void
srec_output_file_wilson::put_byte(unsigned char n)
{
	static const char *table[256] =
	{
		"@",	"A",	"B",	"C",	"D",	"E",	"F",	"G",
		"H",	"I",	"J",	"K",	"L",	"M",	"N",	"O",
		"P",	"Q",	"R",	"S",	"T",	"U",	"V",	"W",
		"X",	"Y",	"Z",	"[",	"\\",	"]",	"^",	"_",
		"`",	"a",	"b",	"c",	"d",	"e",	"f",	"g",
		"h",	"i",	"j",	"k",	"l",	"m",	"n",	"o",
		"p",	"q",	"r",	"s",	"t",	"u",	"v",	"w",
		"x",	"y",	"z",	"{",	"|",	"}",	"~",	"\177",
		"\200",	"\201",	"\202",	"\203",	"\204",	"\205",	"\206",	"\207",
		"\210",	"\211",	"\212",	"\213",	"\214",	"\215",	"\216",	"\217",
		"\220",	"\221",	"\222",	"\223",	"\224",	"\225",	"\226",	"\227",
		"\230",	"\231",	"\232",	"\233",	"\234",	"\235",	"\236",	"\237",
		"\240",	"\241",	"\242",	"\243",	"\244",	"\245",	"\246",	"\247",
		"\250",	"\251",	"\252",	"\253",	"\254",	"\255",	"\256",	"\257",
		"\260",	"\261",	"\262",	"\263",	"\264",	"\265",	"\266",	"\267",
		"\270",	"\271",	"\272",	"\273",	"\274",	"\275",	"\276",	"\277",
		"\300",	"\301",	"\302",	"\303",	"\304",	"\305",	"\306",	"\307",
		"\310",	"\311",	"\312",	"\313",	"\314",	"\315",	"\316",	"\317",
		"\320",	"\321",	"\322",	"\323",	"\324",	"\325",	"\326",	"\327",
		"\330",	"\331",	"\332",	"\333",	"\334",	"\335",	"\336",	"\337",
		":0",	":1",	":2",	":3",	":4",	":5",	":6",	":7",
		":8",	":9",	"::",	":;",	":<",	":=",	":>",	":?",
		";0",	";1",	";2",	";3",	";4",	";5",	";6",	";7",
		";8",	";9",	";:",	";;",	";<",	";=",	";>",	";?",
		"<0",	"<1",	"<2",	"<3",	"<4",	"<5",	"<6",	"<7",
		"<8",	"<9",	"<:",	"<;",	"<<",	"<=",	"<>",	"<?",
		"=0",	"=1",	"=2",	"=3",	"=4",	"=5",	"=6",	"=7",
		"=8",	"=9",	"=:",	"=;",	"=<",	"==",	"=>",	"=?",
		"\340",	"\341",	"\342",	"\343",	"\344",	"\345",	"\346",	"\347",
		"\350",	"\351",	"\352",	"\353",	"\354",	"\355",	"\356",	"\357",
		"\360",	"\361",	"\362",	"\363",	"\364",	"\365",	"\366",	"\367",
		"\370",	"\371",	"\372",	"\373",	"\374",	"\375",	"\376",	"\377",
	};
	put_string(table[n]);
	checksum_add(n);
}


void
srec_output_file_wilson::write_inner(int tag, unsigned long address,
	const void *data, int data_nbytes)
{
	//
	// Make sure the line is not too long.
	//
	if (data_nbytes > 250)
		fatal_error("data length (%d) too long", data_nbytes);

	//
	// Assemble the data for this line.
	//
	unsigned char buffer[256];
	int line_length = data_nbytes + 5;
	buffer[0] = line_length;
	srec_record::encode_big_endian(buffer + 1, address, 4);
	if (data_nbytes)
		memcpy(buffer + 5, data, data_nbytes);

	//
	// Emit the line as hexadecimal text.
//
	put_char(tag);
	checksum_reset();
	for (int j = 0; j < line_length; ++j)
		put_byte(buffer[j]);
	put_byte(~checksum_get());
	put_char('\n');
}


void
srec_output_file_wilson::write(const srec_record &record)
{
	switch (record.get_type())
	{
	case srec_record::type_header:
		// This format doesn't appear to have header records
		break;

	case srec_record::type_data:
		write_inner
		(
			'#',
			record.get_address(),
			record.get_data(),
			record.get_length()
		);
		break;

	case srec_record::type_data_count:
		// ignore
		break;

	case srec_record::type_start_address:
		if (data_only_flag)
			break;
		write_inner('\'', record.get_address(), 0, 0);
		break;

	case srec_record::type_unknown:
		fatal_error("can't write unknown record type");
		break;
	}
}


void
srec_output_file_wilson::line_length_set(int linlen)
{
	//
	// Given the number of characters, figure the maximum number of
	// data baytes.
	// <tag> <size1:2> <addr4:8> ...data... <cs1:2>
	// 1 +   2 +       8 +       2*n +      2       <= linlen
	//
	int n = (linlen - 15) / 2;

	//
	// Constrain based on the file format.
	//
	// The size field (max 255) includes the size of the data,
	// the size of the address (4 bytes) and the size of the
	// size (1 byte), thus 250 (255 - 4 - 1) bytes of data is
	// the safest maximum.
	//
	if (n < 1)
		n = 1;
	else if (n > 250)
		n = 250;

	//
	// An additional constraint is the size of the srec_record
	// data buffer.
	//
	if (n > srec_record::max_data_length)
		n = srec_record::max_data_length;
	pref_block_size = n;
}


void
srec_output_file_wilson::address_length_set(int)
{
	// ignore  (this may change if I ever get a formal spec for
	// this format)
}


int
srec_output_file_wilson::preferred_block_size_get()
	const
{
	return pref_block_size;
}


const char *
srec_output_file_wilson::mode()
	const
{
	return "wb";
}
