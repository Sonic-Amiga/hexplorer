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
# MANIFEST: Test the ti-tagged format functionality
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
	echo 'FAILED test of the ti-tagged format functionality'
	exit 1
}

no_result()
{
	cd $here
	rm -rf $work
	echo 'NO RESULT for test of the ti-tagged format functionality'
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
# Test writing the format
#
cat > test.in << 'fubar'
S00600004844521B
S10A000048656C6C6F2C20B5
S10A0123576F726C64210A9E
S5030002FA
S9030000FC
fubar
if test $? -ne 0; then no_result; fi

cat > test.ok << 'fubar'
K0008HDRB4865B6C6CB6F2C*2090123B576FB726CB6421*0A7F4D5F
:
fubar
if test $? -ne 0; then no_result; fi

$bin/srec_cat test.in -o test.out -ti-tagged
if test $? -ne 0; then fail; fi

diff test.ok test.out
if test $? -ne 0; then fail; fi


# --------------------------------------------------------------------------
#
# Test reading the format
#
cat > test.ok << 'fubar'
S00A00006F756768696E6704
S10A000048656C6C6F2C20B5
S10A0123576F726C64210A9E
S5030002FA
S9030000FC
fubar
if test $? -ne 0; then no_result; fi

cat > test.in << 'fubar'
K000Coughing7FBBAF
B4865B6C6CB6F2C*207FBBDF
90123B576FB726CB6421*0A7FAD2F
:
fubar
if test $? -ne 0; then no_result; fi

$bin/srec_cat test.in -ti-tagged -o test.out
if test $? -ne 0; then fail; fi

diff test.ok test.out
if test $? -ne 0; then fail; fi

#
# The things tested here, worked.
# No other guarantees are made.
#
pass
