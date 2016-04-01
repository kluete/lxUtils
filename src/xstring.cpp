// LX custom sprintf implementation

/*

- full implementation of Stroustrup "type-safe printf()", p. 809

# sortof like:
- boost::format()
- https://github.com/cppformat/cppformat

# misc ideas for specialization
- derive stringstream
  - custom u/int8_t handlers
  - ios_base::register_callback()
- custom stringstream manip(ulator)
- std::enable_if
- std::conditional
- explicit conversion operators
- NOTE: template must be instantiable for ALL types
- C++11 printf() flags @ Stroustrup p. 1257

*/

#include <utility>
#include <locale>
#include <iostream>
#include <cstdint>
#include <string>
#include <algorithm>
#include <stdlib.h>			// atoi() for 'droid
#include <type_traits>
#include <cmath>
#include <iomanip>
#include <iterator>			// for back_inserter on Windows

#include "lx/xutils.h"

// compile-time selection of int type with size of pointer
typedef	std::conditional<(sizeof(void*) == 4), std::uint32_t, std::uint64_t>::type PTR_INT_EQUIV;
const int	PTR_NUM_NYBBLES = sizeof(void*) * 2;

#include "lx/xstring.h"

using namespace std;
using namespace LX;

//---- To Lower-Case ----------------------------------------------------------

string	LX::ToLower(const string &s)
{
	string	res;
	
	std::transform(s.begin(), s.end(), back_inserter(res), ::tolower);
	
	return res;
}

//---- To Human Bytes ---------------------------------------------------------

string	LX::ToHumanBytes(const size_t sz)
{
	struct g123 : std::numpunct<char>
	{
		// std::string	do_grouping() const { return "\1\2\3";}
		std::string	do_grouping() const { return "\3";}
	};
	
	ostringstream	ss;
	
	// copy std locale, tack-on punctuation grouping
	ss.imbue(locale(cout.getloc(), new g123));
	
	ss << sz;
	
	return ss.str();
}

//---- Get Time String --------------------------------------------------------

string	LX::ToHumanTime(const double secs, const bool ms_f)
{
	if (secs < 0)		return "";		// no time
	
	const uint64_t	tot_ms = secs * 1000;
	const uint64_t	tot_secs = secs;
	
	const uint64_t	h = tot_secs / 3600;
	const uint64_t	m = (tot_secs - (h * 3600)) / 60;
	const uint64_t	s = (tot_secs % 60);
	const uint64_t	ms = tot_ms % 1000;
	
	if (ms_f)
		return xsprintf("%02d:%02d:%02d.%03d", h, m, s, ms);
	else	return xsprintf("%02d:%02d:%02d", h, m, s);
}

//---- xsprintf() lowest specialization ---------------------------------------

string	LX::xsprintf(const char *s)
{
	assert(s);
	
	outstream	ss;
	
	while (s && *s)
	{
		if ((*s == '%') && (*++s != '%'))	throw std::runtime_error("invalid format: missing argument in vanilla xsprintf()");
		
		ss << *s++;
	}
	
	return ss.str();	
}

//---- handle xsprintf() prefix -----------------------------------------------

	// (can't move() std::ostringstream cause isn't noexcept)

void	LX::xhandleprefix(const char *&s, outstream &ss)
{
	assert(s);
	
	// vanilla chars
	while (*s && (*s != '%'))	ss.put(*s++);
	
	if (0 == *s)
	{	
		throw runtime_error("arg overflow in xprintf()");
	}
	
	assert('%' == *s);
	
	// skip percent char
	s++;
	if (0 == *s)			throw runtime_error("truncated format in xprintf()");
	
	if ('%' == *s)
	{	// double "%%", doesn't consume argument so don't follow normal code path
		ss.put(*s++);
		// recurse & step out
		return xhandleprefix(s, ss);
	}
	
	// check SIGN prefix
	if ('+' == *s)
	{
		s++;
		ss << showpos;
	}
	
	// check FILL prefix and WIDTH
	const char	pad_char = isdigit(*s) ? (('0' == *s) ? '0' : ' ') : 0;
	int		n_pad_left = 0;
	
	while (isdigit(*s))
		n_pad_left += (n_pad_left * 10) + (*s++ - '0');
	
	
	int	n_pad_right = -1;		// [no-right-pad]
	
	if ('.' == *s)
	{	
		s++;
		n_pad_right = 0;
		
		while (isdigit(*s))
			n_pad_right += (n_pad_right * 10) + (*s++ - '0');
	}
	
	if (n_pad_right >= 0)
	{	// right-pad
		ss << fixed << setprecision(n_pad_right);
	}
	
	if (n_pad_left > 0)
	{
		// front-pad with global width pad (left & right)
		const int	total_pad = (n_pad_right > 0) ? (n_pad_left + 1 + n_pad_right) : n_pad_left;		// include '.' for width-pad
		
		ss << noskipws << setfill(pad_char) << setw(total_pad);
	}
	
	// skip any size specifier
	switch (*s)
	{	case 'z':		// size_t
		case 'h':		// short / unsigned short
		case 'l':		// long / unsigned long
		case 'L':		// long double
	
			s++;
			if (0 == *s)	throw runtime_error("incomplete size format specifier in xprintf()");
			break;
			
		default:
		
			break;
	}
}

//---- int8_t specialization --------------------------------------------------

void	LX::xdump(const int8_t& i8, outstream &ss)
{
	ss << (int) i8;
}

//---- uint8_t specialization -------------------------------------------------

void	LX::xdump(const uint8_t& u8, outstream &ss)
{
	ss << (unsigned int) u8;
}

//---- const void* specialization ---------------------------------------------

void	LX::xdump(const void* p, outstream &ss)
{
	static_assert(sizeof(p) == sizeof(PTR_INT_EQUIV), "unhandled pointer size");
	
	// ss << noshowbase << setw(16) << setfill('0') << p;		// Linux C runtime bug: cannot disable "0x" prefix
	
	const PTR_INT_EQUIV	dbytes = static_cast<const char*>(p) - static_cast<const char*>(nullptr);

	ss << "0x" << hex << setw(PTR_NUM_NYBBLES - 4) << setfill('0') << dbytes;	
}

//---- thread id specialization -----------------------------------------------

void	LX::xdump(const thread::id &thread_id, outstream &ss)
{
	ss << hex << thread_id;
}

// nada mas
