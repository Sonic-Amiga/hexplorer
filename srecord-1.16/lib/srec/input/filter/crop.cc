//
//	srecord - manipulate eprom load files
//	Copyright (C) 1998, 1999, 2001, 2002 Peter Miller;
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
// MANIFEST: functions to impliment the srec_input_filter_crop class
//

#pragma implementation "srec_input_filter_crop"

#include <interval.h>
#include <srec/input/filter/crop.h>
#include <srec/record.h>


srec_input_filter_crop::srec_input_filter_crop(srec_input *a1,
		const interval &a2) :
	srec_input_filter(a1),
	range(a2),
	data(),
	data_range()
{
}


srec_input_filter_crop::~srec_input_filter_crop()
{
}


int
srec_input_filter_crop::read(srec_record &record)
{
	for (;;)
	{
		//
		// If we are not holding any current data,
		// fetch another record from out input.
		//
		if (data_range.empty())
		{
			//
			// If the input is exhausted, we are done.
			//
			if (!srec_input_filter::read(data))
				return 0;
			switch (data.get_type())
			{
			default:
				//
				// If the input is of a typoe we don't
				// care about, pass it through.
				//
				record = data;
				return 1;

			case srec_record::type_data:
				//
				// Data records are remembered, and
				// doled out peicmeal, as they mask
				// against the clip region.
				//
				data_range =
					interval
					(
						data.get_address(),
						data.get_address() + data.get_length()
					);
				break;

			case srec_record::type_start_address:
				//
				// Discard start address records which do
				// not fall into the clip region.
				//
				if (!range.member(data.get_address()))
					continue;
				record = data;
				return 1;
			}
		}

		//
		// Intersect the data left unprocessed in this record
		// with the clip region.  There could be more than one
		// region, but we only want the first one.  We will make
		// another pass if there is more than one.
		//
		interval fragment = range * data_range;
		if (fragment.empty())
		{
			data_range = interval();
			continue;
		}
		fragment.first_interval_only();

		//
		// Construct the return record from the data held.
		//
		unsigned long lo = fragment.get_lowest();
		unsigned long hi = fragment.get_highest();
		record =
			srec_record
			(
				srec_record::type_data,
				lo,
				data.get_data() + lo - data.get_address(),
				hi - lo
			);

		//
		// Subtract this segment from the data held.  If there
		// is anything left, another pass will construct another
		// data record for it.
		//
		data_range -= fragment;

		//
		// Success
		//
		return 1;
	}
}
