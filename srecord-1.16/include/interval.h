//
//	srecord - manipulate eprom load files
//	Copyright (C) 1998-2000, 2002, 2003 Peter Miller;
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
// MANIFEST: interface definition for lib/common/interval.cc
//

#ifndef INCLUDE_INTERVAL_H
#define INCLUDE_INTERVAL_H

#pragma interface "interval"

#include <stddef.h>

#include <iostream>
using namespace std;

/**
  * The interval class is used to represent a set of integer values,
  * usually composed of runs of adjacent value.  Set arithmetic is
  * implemented on these intervals.
  */
class interval
{
public:
    /**
      * The destructor.  It isn't virtual, so don't derive from this class.
      */
    ~interval();

    /**
      * The data_t type is used to parameterize the integr type used in
      * the interval.
      */
    typedef unsigned long data_t;

    /**
      * The default constructor.  The interval is empty.
      */
    interval();

    /**
      * The constructor.  The interval contains the single integer supplied.
      *
      * \param val
      *    The interval is constructed to contain the single interger
      *    value supplied.
      */
    interval(data_t val);

    /**
      * The constructor.  The interval contains all values >= lo and < hi.
      *
      * \param lo
      *     The lower bound of the integers in the initial interval.
      * \param hi
      *     The upper bound of the integers in the initial interval;
      *     this value is not included.
      */
    interval(data_t lo, data_t hi);

    /**
      * The copy constructor.
      */
    interval(const interval &);

    /**
      * The assignment operator.
      */
    interval &operator=(const interval &);

    /**
      * The union_ class method is used to calculate the set union of
      * two intervals.
      */
    static interval union_(const interval &, const interval &);

    /**
      * The intersection class method is used to calculate the set
      * intersection of two intervals.
      */
    static interval intersection(const interval &, const interval &);

    /**
      * The difference class method is used to calculate the set
      * difference of two intervals.
      */
    static interval difference(const interval &, const interval &);

    /**
      * The equal class method is used to test the equality of two
      * intervals.
      */
    static bool equal(const interval &, const interval &);

    /**
      * The member method is used to test whether a given value is a
      * member of the interval.
      *
      * \param val
      *     The value to test for membership
      * \returns
      *     True if the given value is a member of the interval,
      *     false if it is not.
      */
    bool member(data_t val) const;

    /**
      * The empty method is used to test whether the interval is empty.
      *
      * \returns
      *     True if the interval is empty,
      *     false if the interval is not empty.
      */
    bool empty() const;

    /**
      * The first_interval_only method is used to crop the interval to the
      * first (numerically least) run of consecutive integers in the set.
      */
    void first_interval_only();

    /**
      * The interval_scan_begin method is used to start traversing every
      * integer value in the interval.
      */
    void scan_begin();

    /**
      * The interval_scan_next method is used to traverse every integer
      * value in the interval.
      */
    bool scan_next(data_t &);

    /**
      * The interval_scan_end method is used to finish traversing every
      * integer value in the interval.
      */
    void scan_end();

    /**
      * The get_lowest method is used to obtain the lower bound of
      * the interval.  It is inclusive.
      */
    data_t get_lowest() const;

    /**
      * The get_highest method is used to obtain the upper bound of
      * the interval.  It is exclusive (i.e. one beyond the highest
      * integer in the set).
      */
    data_t get_highest() const;

    /**
      * The print method is used to print an interval on an output stream.
      */
    void print(ostream &) const;

private:
    /**
      * The length instance variable is used to remember the length of
      * the data instance variable.
      */
    size_t length;

    /**
      * The size instance variable is used to remember the maximum size
      * of the data instance variable.  The length can go up and down
      * depending on the calculation, but the size only ever rises.
      */
    size_t size;

    /**
      * The scan_index instance variable is used to remember where the
      * scan us up to.	Used by the scan_next method, et al.
      */
    size_t scan_index;

    /**
      * The scan_next_datum instance variable is used to remember where
      * the scan us up to.  Used by the scan_next method, et al.
      */
    data_t scan_next_datum;

    /**
      * The data instance variable is used to remember a pointer to
      * the base of an array of interger values.  They come in [lo, hi)
      * pairs.  As a sanity check, there is an extra item, wich contains
      * the same value as the length instance variable.
      */
    data_t *data;

    /**
      * The valid method is used to test whether the interval is
      * internally self-consistent.  Principally of use when debugging.
      */
    bool valid() const;

    /**
      * The append method is used to append another valoue to the end
      * of an interval under construction.
      */
    void append(data_t);
};


inline bool
operator == (const interval &lhs, const interval &rhs)
{
    return interval::equal(lhs, rhs);
}


inline bool
operator != (const interval &lhs, const interval &rhs)
{
    return !interval::equal(lhs, rhs);
}


inline interval
operator * (const interval &lhs, const interval &rhs)
{
    return interval::intersection(lhs, rhs);
}


inline interval &
operator *= (interval &lhs, const interval &rhs)
{
    lhs = interval::intersection(lhs, rhs);
    return lhs;
}


inline interval
operator + (const interval &lhs, const interval &rhs)
{
    return interval::union_(lhs, rhs);
}


inline interval &
operator += (interval &lhs, const interval &rhs)
{
    lhs = interval::union_(lhs, rhs);
    return lhs;
}


inline interval
operator - (const interval &lhs, const interval &rhs)
{
    return interval::difference(lhs, rhs);
}


inline interval &
operator -= (interval &lhs, const interval &rhs)
{
    lhs = interval::difference(lhs, rhs);
    return lhs;
}

inline interval
operator - (const interval &arg)
{
    return (interval(0, 0) - arg);
}

inline ostream &
operator << (ostream &os, const interval &val)
{
    val.print(os);
    return os;
}

#endif // INCLUDE_INTERVAL_H
