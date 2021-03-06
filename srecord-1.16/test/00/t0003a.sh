#!/bin/sh
#
#	srecord - manipulate eprom load files
#	Copyright (C) 1998, 1999, 2002, 2003 Peter Miller;
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
# MANIFEST: Test the intel functionality
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
	echo 'FAILED test of the intel functionality'
	exit 1
}

no_result()
{
	cd $here
	rm -rf $work
	echo 'NO RESULT for test of the intel functionality'
	exit 2
}

trap "no_result" 1 2 3 15

bin=$here/${1-.}/bin
mkdir $work
if test $? -ne 0; then no_result; fi
cd $work
if test $? -ne 0; then no_result; fi

cat > test.in << 'fubar'
:10000000DB00E60F5F1600211100197ED300C3004C
:1000100000000101030307070F0F1F1F3F3F7F7FF2
:01002000FFE0
:00000001FF
fubar
if test $? -ne 0; then no_result; fi

cat > test.ok << 'fubar'
S00600004844521B
S1230000DB00E60F5F1600211100197ED300C30000000101030307070F0F1F1F3F3F7F7F4A
S1040020FFDC
S5030002FA
S9030000FC
fubar
if test $? -ne 0; then no_result; fi

$bin/srec_cat test.in -intel -o test.out -motorola -header HDR > LOG 2>&1
if test $? -ne 0; then cat LOG; fail; fi

diff test.ok test.out
if test $? -ne 0; then fail; fi

cat > test.in << 'fubar'
S00600004844521B
S1230000DB00E60F5F1600211100197ED300C30000000101030307070F0F1F1F3F3F7F7F4A
S1040020FFDC
S5030002FA
S9030000FC
fubar
if test $? -ne 0; then no_result; fi

cat > test.ok << 'fubar'
:020000040000FA
:20000000DB00E60F5F1600211100197ED300C30000000101030307070F0F1F1F3F3F7F7F4E
:01002000FFE0
:0400000500000000F7
:00000001FF
fubar
if test $? -ne 0; then no_result; fi

$bin/srec_cat test.in -motorola -o test.out -intel > LOG 2>&1
if test $? -ne 0; then cat LOG; fail; fi

diff test.ok test.out
if test $? -ne 0; then fail; fi

cat > test.in1 << 'fubar'
:10000000DB00E60F5F1600211100197ED300C3004C
:1000100000000101030307070F0F1F1F3F3F7F7FF2
:01002000FFE0
:00000001FF
fubar
if test $? -ne 0; then no_result; fi

cat > test.in2 << 'fubar'
:20000000DB00E60F5F1600211100197ED300C30000000101030307070F0F1F1F3F3F7F7F4E
:01002000FFE0
:0400000500000000F7
:00000001FF
fubar
if test $? -ne 0; then no_result; fi

$bin/srec_cmp test.in1 -intel test.in2 -intel > LOG 2>&1
if test $? -ne 0; then cat LOG; fail; fi

#
# The things tested here, worked.
# No other guarantees are made.
#
pass
