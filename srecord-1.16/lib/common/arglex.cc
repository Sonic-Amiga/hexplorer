//
//	srecord - manipulate eprom load files
//	Copyright (C) 1998, 1999, 2002, 2003 Peter Miller;
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

#pragma implementation "arglex"

#include <cctype>
#include <cstring>
#include <errno.h>
#include <iostream>
using namespace std;
#include <unistd.h>

#include <arglex.h>
#include <progname.h>
#include <versn_stamp.h>

// Cygwin's mingw has the execvp prototype in the wrong place.
#ifdef __MSVCRT__
#include <process.h>
#endif

static const arglex::table_ty default_table[] =
{
	{ "-",			arglex::token_stdio,		},
	{ "-Help",		arglex::token_help,		},
	{ "-LICense",		arglex::token_license,		},
	{ "-Page_Length",	arglex::token_page_length,	},
	{ "-Page_Width",	arglex::token_page_width,	},
	{ "-TRACIng",		arglex::token_tracing,		},
	{ "-Verbose",		arglex::token_verbose,		},
	{ "-VERSion",		arglex::token_version,		},
	ARGLEX_END_MARKER
};


arglex::arglex() :
	argc(0),
	argv(0),
	tables(),
	pushback_depth(0),
	usage_tail_(0)
{
	table_set(default_table);
}


arglex::arglex(int ac, char **av) :
	argc(ac - 1),
	argv(av + 1),
	tables(),
	pushback_depth(0),
	usage_tail_(0)
{
	progname_set(av[0]);
	table_set(default_table);
}


arglex::~arglex()
{
}


void
arglex::table_set(const table_ty *tp)
{
	tables.push_back(tp);
}


//
// NAME
//	arglex_compare
//
// SYNOPSIS
//	int arglex_compare(const char *formal, char *actual);
//
// DESCRIPTION
//	The arglex_compare function is used to compare
//	a command line string with a formal spec of the option,
//	to see if they compare equal.
//
//	The actual is case-insensitive.  Uppercase in the formal
//	means a mandatory character, while lower case means optional.
//	Any number of consecutive optional characters may be supplied
//	by actual, but none may be skipped, unless all are skipped to
//	the next non-lower-case letter.
//
//	The underscore (_) is like a lower-case minus,
//	it matches "", "-" and "_".
//
//	The "*" in a pattern matches everything to the end of the line,
//	anything after the "*" is ignored.  The rest of the line is pointed
//	to by the "partial" variable as a side-effect (else it will be 0).
//	This rather ugly feature is to support "-I./dir" type options.
//
//	A backslash in a pattern nominates an exact match required,
//	case must matche excatly here.
//	This rather ugly feature is to support "-I./dir" type options.
//
//	For example: "-project" and "-P' both match "-Project",
//	as does "-proJ", but "-prj" does not.
//
//	For example: "-devDir" and "-d_d' both match "-Development_Directory",
//	but "-dvlpmnt_drctry" does not.
//
//	For example: to match include path specifications, use a pattern
//	such as "-\\I*", and the partial global variable will have the
//	path in it on return.
//
// ARGUMENTS
//	formal	- the "pattern" for the option
//	actual	- what the user supplied
//
// RETURNS
//	int;	zero if no match,
//		non-zero if they do match.
//

static char *partial;

bool
arglex_compare(const char *formal, char *actual)
{
	for (;;)
	{
		unsigned char ac = *actual++;
		if (isupper(ac))
			ac = tolower(ac);
		unsigned char fc = *formal++;
		switch (fc)
		{
		case 0:
			return !ac;
			
		case '_':
			if (ac == '-')
				break;
			// fall through...

		case 'a': case 'b': case 'c': case 'd': case 'e':
		case 'f': case 'g': case 'h': case 'i': case 'j':
		case 'k': case 'l': case 'm': case 'n': case 'o':
		case 'p': case 'q': case 'r': case 's': case 't':
		case 'u': case 'v': case 'w': case 'x': case 'y':
		case 'z': 
			//
			// optional characters
			//
			if (ac == fc && arglex_compare(formal, actual))
				return true;
			//
			// skip forward to next
			// mandatory character, or after '_'
			//
			while (islower(*formal))
				++formal;
			if (*formal == '_')
			{
				++formal;
				if (ac == '_' || ac == '-')
					++actual;
			}
			--actual;
			break;

		case '*':
			//
			// This is a hack, it should really 
			// check for a match match the stuff after
			// the '*', too, a la glob.
			//
			if (!ac)
				return false;
			partial = actual - 1;
			return true;

		case '\\':
			if (actual[-1] != *formal++)
				return false;
			break;

		case 'A': case 'B': case 'C': case 'D': case 'E':
		case 'F': case 'G': case 'H': case 'I': case 'J':
		case 'K': case 'L': case 'M': case 'N': case 'O':
		case 'P': case 'Q': case 'R': case 'S': case 'T':
		case 'U': case 'V': case 'W': case 'X': case 'Y':
		case 'Z': 
			fc = tolower(fc);
			// fall through...

		default:
			//
			// mandatory characters
			//
			if (fc != ac)
				return false;
			break;
		}
	}
}


//
// NAME
//	is_a_number
//
// SYNOPSIS
//	int is_a_number(char *s);
//
// DESCRIPTION
//	The is_a_number function is used to determine if the
//	argument is a number.
//
//	The value is placed in arglex_value.alv_number as
//	a side effect.
//
//	Negative and positive signs are accepted.
//	The C conventions for decimal, octal and hexadecimal are understood.
//
//	There may be no white space anywhere in the string,
//	and the string must end after the last digit.
//	Trailing garbage will be interpreted to mean it is not a string.
//
// ARGUMENTS
//	s	- string to be tested and evaluated
//
// RETURNS
//	int;	zero if not a number,
//		non-zero if is a number.
//

static int
is_a_number(const char *s, long &n)
{
	int		sign;

	n = 0;
	switch (*s)
	{
	case '-':
		++s;
		sign = -1;
		break;

	case '+':
		++s;
		sign = 1;
		break;

	default:
		sign = 1;
		break;
	}
	switch (*s)
	{
	case '0':
		if ((s[1] == 'x' || s[1] == 'X') && s[2])
		{
			s += 2;
			for (;;)
			{
				switch (*s)
				{
				case '0': case '1': case '2': case '3':
				case '4': case '5': case '6': case '7':
				case '8': case '9':
					n = n * 16 + *s++ - '0';
					continue;

				case 'A': case 'B': case 'C':
				case 'D': case 'E': case 'F':
					n = n * 16 + *s++ - 'A' + 10;
					continue;

				case 'a': case 'b': case 'c':
				case 'd': case 'e': case 'f':
					n = n * 16 + *s++ - 'a' + 10;
					continue;
				}
				break;
			}
		}
		else
		{
			for (;;)
			{
				switch (*s)
				{
				case '0': case '1': case '2': case '3':
				case '4': case '5': case '6': case '7':
					n = n * 8 + *s++ - '0';
					continue;
				}
				break;
			}
		}
		break;

	case '1': case '2': case '3': case '4':
	case '5': case '6': case '7': case '8': case '9':
		for (;;)
		{
			switch (*s)
			{
			case '0': case '1': case '2': case '3':
			case '4': case '5': case '6': case '7':
			case '8': case '9':
				n = n * 10 + *s++ - '0';
				continue;
			}
			break;
		}
		break;

	default:
		return 0;
	}
	if (*s)
		return 0;
	n *= sign;
	return 1;
}


//
// NAME
//	arglex
//
// SYNOPSIS
//	int arglex::token_next(void);
//
// DESCRIPTION
//	The arglex function is used to perfom lexical analysis
//	on the command line arguments.
//
//	Unrecognised options are returned as arglex_token_option
//	for anything starting with a '-', or
//	arglex_token_string otherwise.
//
// RETURNS
//	The next token in the token stream.
//	When the end is reached, arglex_token_eoln is returned forever.
//
// CAVEAT
//	Must call arglex_init befor this function is called.
//

int
arglex::token_next()
{
	const table_ty	*tp;
	const table_ty	*hit[20];
	int		nhit;
	char		*arg;

	if (pushback_depth > 0)
	{
		//
		// the second half of a "-foo=bar" style argument.
		//
		arg = pushback[--pushback_depth];
	}
	else
	{
		if (argc <= 0)
		{
			value_string_ = "";
			token = token_eoln;
			return token;
		}
		arg = argv[0];
		argc--;
		argv++;

		//
		// See if it looks like a GNU "-foo=bar" option.
		// Split it at the '=' to make it something the
		// rest of the code understands.
		//
		if (arg[0] == '-' && arg[1] != '=')
		{
			char *eqp;

			eqp = strchr(arg, '=');
			if (eqp)
			{
				pushback[pushback_depth++] = eqp + 1;
				*eqp = 0;
			}
		}

		//
		// Turn the GNU-style leading "--"
		// into "-" if necessary.
		//
		if
		(
			arg[0] == '-'
		&&
			arg[1] == '-'
		&&
			arg[2]
		&&
			!is_a_number(arg + 1, value_number_)
		)
			++arg;
	}
	value_string_ = arg;

	//
	// see if it is a number
	//
	if (is_a_number(arg, value_number_))
	{
		token = arglex::token_number;
		return token;
	}

	//
	// scan the tables to see what it matches
	//
	nhit = 0;
	partial = 0;
	for
	(
		table_ptr_vec_t::iterator it = tables.begin();
		it != tables.end();
		++it
	)
	{
		for (tp = *it; tp->name; tp++)
		{
			if (arglex_compare(tp->name, arg))
				hit[nhit++] = tp;
		}
	}

	//
	// deal with unknown or ambiguous options
	//
	switch (nhit)
	{
	case 0:
		//
		// not found in the tables
		//
		if (*value_string_ == '-')
			token = arglex::token_option;
		else
			token = arglex::token_string;
		break;

	case 1:
		if (partial)
			pushback[pushback_depth++] = partial;
		value_string_ = hit[0]->name;
		token = hit[0]->token;
		break;

	default:
		cerr << "option ``" << value_string_ <<
			"'' is ambiguous" << endl;
		exit(1);
	}
	return token;
}


const char *
arglex::token_name(int n)
{
	switch (n)
	{
	case token_eoln:
		return "end of command line";

	case token_number:
		return "number";

	case token_option:
		return "option";

	case token_stdio:
		return "standard input or output";

	case token_string:
		return "string";

	default:
		break;
	}
	for
	(
		table_ptr_vec_t::iterator it = tables.begin();
		it != tables.end();
		++it
	)
	{
		for (const table_ty *tp = *it; tp->name; ++tp)
		{
			if (tp->token == n)
				return tp->name;
		}
	}
	return "unknown command line token";
}


void
arglex::help(const char *name)
	const
{
	if (!name)
		name = progname_get();
	const char *cmd[3] = { "man", name, 0 };
	execvp(cmd[0], (char *const *)cmd);
	cerr << cmd[0] << ": " << strerror(errno) << endl;
	exit(1);
}


void
arglex::version()
	const
{
	cout << progname_get() << " version " << version_stamp() << endl;
	cout << "Copyright (C) " << copyright_years() << " Peter Miller;"
		<< endl;
	cout << "All rights reserved." << endl;
	cout << endl;
	cout << "The " << progname_get()
		<< " program comes with ABSOLUTELY NO WARRANTY;" << endl;
	cout << "for details use the '" << progname_get()
		<< " -LICense' command." << endl;
	cout << "The " << progname_get()
		<< " program is free software, and you are welcome" << endl;
	cout << "to redistribute it under certain conditions; for" << endl;
	cout << "details use the '" << progname_get() << " -LICense' command."
		<< endl;
	exit(0);
}


void
arglex::license()
	const
{
	help("srec_license");
}


void
arglex::bad_argument()
	const
{
	switch (token_cur())
	{
	case token_string:
		cerr << "misplaced file name (``" << value_string()
			<< "'') on command line" << endl;
		break;

	case token_number:
		cerr << "misplaced number (" << value_string()
			<< ") on command line" << endl;
		break;

	case token_option:
		cerr << "unknown ``" << value_string() << "'' option" << endl;
		break;

	case token_eoln:
		cerr << "command line too short" << endl;
		break;

	default:
		cerr << "misplaced ``" << value_string() << "'' option" << endl;
		break;
	}
	usage();
	exit(1);
}


int
arglex::token_first()
{
	switch (token_next())
	{
	default:
		return token_cur();

	case token_help:
		if (token_next() != token_eoln)
			bad_argument();
		help();
		break;

	case token_version:
		if (token_next() != token_eoln)
			bad_argument();
		version();
		break;

	case token_license:
		if (token_next() != token_eoln)
			bad_argument();
		license();
		break;
	}
	exit(0);
}


void
arglex::usage_tail_set(const char *s)
{
	usage_tail_ = s;
}


const char *
arglex::usage_tail_get()
	const
{
	if (!usage_tail_)
		usage_tail_ = "<filename>...";
	return usage_tail_;
}


void
arglex::usage()
	const
{
	cerr << "Usage: " << progname_get() << " [ <option>... ] "
		<< usage_tail_get() << endl;
	cerr << "       " << progname_get() << " -Help" << endl;
	cerr << "       " << progname_get() << " -VERSion" << endl;
	cerr << "       " << progname_get() << " -LICense" << endl;
	exit(1);
}
