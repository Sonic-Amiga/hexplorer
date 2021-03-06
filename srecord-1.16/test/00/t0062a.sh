#!/bin/sh
#
#	srecord - manipulate eprom load files
#	Copyright (C) 2001 Peter Miller;
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
# MANIFEST: Test the unfill functionality
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
	echo 'FAILED test of the unfill functionality'
	exit 1
}

no_result()
{
	cd $here
	rm -rf $work
	echo 'NO RESULT for test of the unfill functionality'
	exit 2
}

trap "no_result" 1 2 3 15

bin=$here/${1-.}/bin
mkdir $work
if test $? -ne 0; then no_result; fi
cd $work
if test $? -ne 0; then no_result; fi

cat > test.in << 'fubar'
S00600004844521B
S12300002BB62152DF32840E10AE628F87804D0D90C8F4719185F4AC539F8FF4BD33EABD56
S123002046B5634BBE65A1205FE4F3E67F8247C1EB8D180BE2636EB2498F09DBE31E44D336
S1230040C89EDF7157401A5CB159FCBF2E061D736D0AF2BEDA14B58D4482CE5E80C8658BCF
S9030000FC
fubar
if test $? -ne 0; then no_result; fi

cat > test.ok << 'fubar'
S00600004844521B
S12300002BB62152DF32840E10AE628F87804D0D90C8F4719185F4AC539F8FF4BD33EABD56
S10E002046B5634BBE65A1205FE4F30E
S123002C7F8247C1EB8D180BE2636EB2498F09DBE31E44D3C89EDF7157401A5CB159FCBF4B
S117004C2E061D736D0AF2BEDA14B58D4482CE5E80C8658B57
S5030004F8
S9030000FC
fubar
if test $? -ne 0; then no_result; fi

$bin/srec_cat test.in -unfill 0xE6 -o test.out
if test $? -ne 0; then fail; fi

diff test.ok test.out
if test $? -ne 0; then fail; fi

#
# The things tested here, worked.
# No other guarantees are made.
#
pass
