#!/bin/sh
#
#	srecord - manipulate eprom load files
#	Copyright (C) 2000 Peter Miller;
#	All rights reserved.
#
#	This program is free software; you can redistribute it and/or modify
#	it under the terms of the GNU General Public License as published by
#	the Free Software Foundation; either version 2 of the License, or
#	(at your option) any later version.
#
#	This program is distributed in the hope that it will be useful,
#	but WITHOUT ANY WARRANTY; without even the implied warranty of
#	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#	GNU General Public License for more details.
#
#	You should have received a copy of the GNU General Public License
#	along with this program; if not, write to the Free Software
#	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
#
# MANIFEST: Test the format guessing functionality
#
here=`pwd`
if test $? -ne 0 ; then exit 2; fi
work=${TMP_DIR-/tmp}/$$

pass()
{
	cd $here
	rm -rf $work
	echo PASSED
	exit 0
}

fail()
{
	cd $here
	rm -rf $work
	echo 'FAILED test of the format guessing functionality'
	exit 1
}

no_result()
{
	cd $here
	rm -rf $work
	echo 'NO RESULT for test of the format guessing functionality'
	exit 2
}

trap "no_result" 1 2 3 15

bin=$here/${1-.}/bin
mkdir $work
if test $? -ne 0; then no_result; fi
cd $work
if test $? -ne 0; then no_result; fi

# --------------------------------------------------------------------------
#
cat > test.in << 'fubar'
S00600004844521B
S111000048656C6C6F2C20576F726C64210A7B
S5030001FB
S9030000FC
fubar
if test $? -ne 0; then no_result; fi

cat > test.ok << 'fubar'
Format:	Motorola S-Record
Header:	"HDR"
Start:	00000000
Data:	0000 - 000D
fubar
if test $? -ne 0; then no_result; fi

$bin/srec_info test.in -guess >  test.out
if test $? -ne 0; then fail; fi

diff test.ok test.out
if test $? -ne 0; then fail; fi

# --------------------------------------------------------------------------
#
cat > test.in << 'fubar'
:0E00000048656C6C6F2C20576F726C64210A7F
:0400000500000000F7
:00000001FF
fubar
if test $? -ne 0; then no_result; fi

cat > test.ok << 'fubar'
Format:	Intel Hexadecimal (MCS-86)
Start:	00000000
Data:	0000 - 000D
fubar
if test $? -ne 0; then no_result; fi

$bin/srec_info test.in -guess >  test.out
if test $? -ne 0; then fail; fi

diff test.ok test.out
if test $? -ne 0; then fail; fi

# --------------------------------------------------------------------------
#
cat > test.in << 'fubar'
K0005B4865B6C6CB6F2CB2057B6F72B6C64B210A7F6C7F
:
fubar
if test $? -ne 0; then no_result; fi

cat > test.ok << 'fubar'
Format:	Texas Instruments Tagged (SDSMAC)
Data:	0000 - 000D
fubar
if test $? -ne 0; then no_result; fi

$bin/srec_info test.in -guess >  test.out
if test $? -ne 0; then fail; fi

diff test.ok test.out
if test $? -ne 0; then fail; fi

#
# The things tested here, worked.
# No other guarantees are made.
#
pass
