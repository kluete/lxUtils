// lx utils template-based string formatting
// based on Stroustrup's idea
//   http://www.stroustrup.com/C++11FAQ.html#variadic-templates 

#pragma once

#include <cassert>
#include <cstdint>
#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <thread>
#include <stdexcept>
#include <type_traits>

#if LX_WX
	#include "wx/string.h"
#endif

#if LX_JUCE
	#include "JuceHeader.h"
#endif
	
namespace LX
{

std::string	ToLower(const std::string &s);
std::string	ToHumanBytes(const size_t sz);
std::string	ToHumanTime(const double secs, const bool ms_f);

typedef	std::ostringstream	outstream;
void	xhandleprefix(const char *&s, outstream &ss);
// no-arg specialization
std::string	xsprintf(const char *s);

template<typename T>
struct Has_ToStdStringMethod
{
	template<typename U, std::string (U::*)() const> struct SFINAE {};
	template<typename U> static char Test(SFINAE<U, &U::ToStdString>*);
	template<typename U> static int Test(...);
	static const bool Has = sizeof(Test<T>(0)) == sizeof(char);
};

template<typename _T>
typename std::enable_if<!Has_ToStdStringMethod<_T>::Has,void>::type			// if _T is NOT wxString
	xdump(const _T &val, outstream &ss)
{
	ss << val;
}

template<typename _T>
typename std::enable_if<Has_ToStdStringMethod<_T>::Has,void>::type			// if _T is wxString
	xdump(const _T &val, outstream &ss)
{
	ss << val.ToStdString();
}

// workaround specializations
void	xdump(const std::int8_t &i8, outstream &ss);
void	xdump(const std::uint8_t &ui8, outstream &ss);
void	xdump(const void* p, outstream &ss);
void	xdump(const std::thread::id &thread_id, outstream &ss);

#if LX_JUCE
	inline
	void	xdump(const juce::String &val, outstream &ss)
	{
		ss << val.toStdString();
	}
#endif

// full vararg sprintf() re-implementation
template<typename _T, typename ... Args>
std::string	xsprintf(const char *s, const _T &val, Args&& ... args)
{
	using namespace std;
	
	outstream	ss;
	
	xhandleprefix(s/*&*/, ss/*&*/);
	
	const char	fmt_c = *s++;
	const bool	integral_f = is_integral<_T>();
	const bool	float_f = is_floating_point<_T>();
	const bool	enum_f = is_enum<_T>();
	const bool	int_conv_f = is_convertible<_T, int>();
	const bool	thread_f = is_same<_T, std::thread::id>();
	const bool	int_f = integral_f || (enum_f && int_conv_f) || thread_f;		// tortuous accomodation for enums
	const bool	number_f = int_f || float_f;
	const bool	ptr_f = true;	// (is_pointer<_T>()) || (is_member_object_pointer<_T>()) || (is_member_function_pointer<_T>()) || (is_pointer<void>());
	
	#if LX_WX
		const bool	wxstring_f = is_same<_T, wxString>();
	#else
		const bool	wxstring_f = false;
	#endif
	
	#if LX_JUCE
		const bool	juce_string_f = is_same<_T, juce::String>();
	#else
		const bool	juce_string_f = false;
	#endif
	
	switch (fmt_c)
	{
		case 'c':
		
			if (!int_f)			throw runtime_error("bad xsprintf() char format");
			if (sizeof(_T) != 1)		throw runtime_error("bad xsprintf() char arg size");		// '.' may be passed as int vs char?
			if (is_same<_T, bool>())	ss << boolalpha;
				
			ss << val;
			break;
		
		case 'S':	// QUOTED string
		case 's':
		{
			bool	ok = false;
			
			ok |= is_same<_T, std::string>();
			ok |= is_convertible<_T, const char*>();
			ok |= wxstring_f;
			ok |= juce_string_f;
			if (!ok)
			{
				assert(0);
				throw runtime_error("bad xsprintf() string format");
			}
			if (fmt_c == 'S')	ss << "\"";
			
			if (wxstring_f)		xdump(val, ss);		// wxString
			else if (juce_string_f)	xdump(val, ss);		// juce::String
			else			ss << val;
			
			if (fmt_c == 'S')	ss << "\"";
		}	break;
			
		case 'd':
		case 'i':
		case 'u':					// used to be handled separately
			
			if (!number_f)			throw runtime_error("bad xsprintf() integer format");
			xdump(val, ss);
			break;
		
		case 'X':
			
			if (!int_f)			throw runtime_error("bad xsprintf() integer format for upper-case hex");
			ss << hex << uppercase;
			xdump(val, ss);
			break;
			
		case 'x':
			
			if (!int_f)			throw runtime_error("bad xsprintf() integer format for (lower-case) hex");
			ss << hex;
			xdump(val, ss);
			break;
		
		case 'p':
		{	// ptr
			if (!ptr_f)			throw runtime_error("bad xsprintf() pointer format");
			xdump(val, ss);
		}	break;
		
		case 'g':
		
			// ss.precision(10);
		
		case 'f':
		case 'e':
		case 'E':
		case 'G':
			
			if (!number_f)			throw runtime_error("bad xsprintf() float or double format");
			ss << val;
			break;
			
		default:
		{	// ERROR - unknown format flag
			throw runtime_error("unhandled xsprintf() format flag");
		}	break;
	}
	
	return ss.str() + xsprintf(s, std::forward<Args>(args) ...);	// recurse with tail of arg list
}

} // namespace LX

// nada mas
