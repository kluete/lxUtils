// LX utilities

#include <cassert>
#include <sstream>
#include <chrono>
#include <utility>
#include <locale>
#include <iostream>
#include <cmath>
#include <iomanip>
#include <stdexcept>
#include <ctime>

#include <cstring>

#include <csignal>

#ifdef WIN32
	#include "Winsock.h"
	// #include <sys/timeb.h>
	
	#ifdef __MINGW64__
	#endif
#else
	// Nix
	#define	LOG_SIGNAL_TRAP
#endif

#include "lx/xutils.h"

using namespace std;
using namespace std::chrono;
using namespace LX;

#if 0	// def WIN32

// WAY TOO SLOW!!!
typedef ULONGLONG (WINAPI GetTickCount64Proc)(void);
static GetTickCount64Proc *pt2GetTickCount64;
static GetTickCount64Proc *pt2RealGetTickCount;

using uint64 = uint64_t;
using int64 = int64_t;

static uint64	startPerformanceCounter;
static uint64	startGetTickCount = 0;
// MSVC 6 standard doesn't like division with uint64s
static double	counterPerMicrosecond;
static int64_t	s_start_us;

static uint64 UTGetTickCount64()
{
	if (pt2GetTickCount64) {
		return pt2GetTickCount64();
	}
	if (pt2RealGetTickCount) {
		uint64 v = pt2RealGetTickCount();
		// fix return value from GetTickCount
		return (DWORD)v | ((v >> 0x18) & 0xFFFFFFFF00000000);
	}
	return (uint64)GetTickCount();
}

static
void	Time_Initialize(void)
{
	HMODULE kernel32 = GetModuleHandleA("kernel32.dll");
	pt2GetTickCount64 = (GetTickCount64Proc*)GetProcAddress(kernel32, "GetTickCount64");
	// not a typo. GetTickCount actually returns 64 bits
	pt2RealGetTickCount = (GetTickCount64Proc*)GetProcAddress(kernel32, "GetTickCount");

	uint64 frequency;
	QueryPerformanceCounter((LARGE_INTEGER*)&startPerformanceCounter);
	QueryPerformanceFrequency((LARGE_INTEGER*)&frequency);
	counterPerMicrosecond = (double)frequency / 1000000.0f;
	startGetTickCount = UTGetTickCount64();

	const auto	tp = std::chrono::system_clock::now().time_since_epoch();
	const auto	elap_ms_no_typ = duration_cast<milliseconds>(tp);
	const int64_t	elap_ms = elap_ms_no_typ.count();
	
	s_start_us = elap_ms * 1000;
}

static int64 abs64(int64 x) { return x < 0 ? -x : x; }

static
uint64 __GetMicroseconds()
{
	static bool time_init = false;
	if (!time_init) {
		time_init = true;
		Time_Initialize();
	}

	uint64 counter;
	uint64 tick;

	QueryPerformanceCounter((LARGE_INTEGER*) &counter);
	tick = UTGetTickCount64();

	// unfortunately, QueryPerformanceCounter is not guaranteed to be monotonic. Make it so.
	int64 ret = (int64)(((int64)counter - (int64)startPerformanceCounter) / counterPerMicrosecond);
	// if the QPC clock leaps more than one second off GetTickCount64() something is seriously fishy. Adjust QPC to stay monotonic
	int64 tick_diff = tick - startGetTickCount;
	if (abs64(ret / 100000 - tick_diff / 100) > 10)
	{
		startPerformanceCounter -= (uint64)((int64)(tick_diff * 1000 - ret) * counterPerMicrosecond);
		ret = (int64)((counter - startPerformanceCounter) / counterPerMicrosecond);
	}
	return s_start_us + ret;
}

static inline uint64 UTP_GetMilliseconds()
{
	return GetTickCount();
}

#endif // WIN32

//---- Debugger Trap Signal ---------------------------------------------------

void	LX::xtrap(const char *s)
{
    (void)s;
    
	#ifdef LOG_SIGNAL_TRAP
		::kill(0, SIGTRAP);	// (copied from Juce)
	#endif
	
	#ifdef WIN32
		DebugBreak();
	#endif
}

//---- Soft (non-spurious) String to Double conversion ------------------------

double	LX::Soft_stod(const string &s, const double def)
{
	try
	{	
		if (s.empty())		return def;
		
		const double	v = stod(s);
		return v;
	
	}
	catch (std::runtime_error &e)
	{
		const char	*what_s = e.what();	// (don't allocate)
		(void)what_s;
	}
	
	return def;
}

//---- Soft (non-spurious) String to Int conversion ---------------------------

int	LX::Soft_stoi(const string &s, const int def)
{
	try
	{	
		if (s.empty())	return def;
		
		const int	v = stoi(s);
		return v;
	
	}
	catch (std::runtime_error &e)
	{
		const char	*what_s = e.what();	// (don't allocate)
		(void)what_s;
	}
	
	return def;
}
//---- Soft (non-spurious) String to 32-bit long conversion -------------------

uint32_t	LX::Soft_stoul(const string &s, const uint32_t def, const int base)
{
	try
	{	
		if (s.empty())		return def;
		
		size_t	dummy = 0;
		
		const uint32_t	v = stoul(s, &dummy, base);
		
		return v;
	}
	catch (std::runtime_error &e)
	{
		const char	*what_s = e.what();	// (don't allocate)
		(void)what_s;
	}
	
	return def;
}

//==== timestamp ==============================================================

// static
int64_t	timestamp_t::NowMicroSecs(void)
{
	#ifndef WIN32
		const auto	tp = stampclock_t::now().time_since_epoch();
		const auto	elap_us_no_typ = duration_cast<microseconds>(tp);
		const int64_t	elap_us = elap_us_no_typ.count();
	
		return elap_us;
	#else
		// return __GetMicroseconds();		// WAY TOO SLOW!
		const auto	tp = stampclock_t::now().time_since_epoch();
		const auto	elap_ms_no_typ = duration_cast<milliseconds>(tp);
		const int64_t	elap_us = elap_ms_no_typ.count() * 1000ll;

		return elap_us;
	#endif
}

	timestamp_t::timestamp_t(const int64_t &u_secs)
		: m_usecs(u_secs)
{
	assert(u_secs >= 0);				// REMOVE eventually?
}

	timestamp_t::timestamp_t()
		: timestamp_t(NowMicroSecs())
{
}

// static
timestamp_t	timestamp_t::FromUS(const int64_t &us)
{
	return timestamp_t(us);
}

// static
timestamp_t	timestamp_t::FromSecs(const double &secs)
{
	const int64_t	usecs = secs * 1'000'000;
	
	return timestamp_t(usecs);
}

// static
timestamp_t	timestamp_t::FromMS(const double &ms)
{
	const int64_t	usecs = ms * 1'000;
	
	return timestamp_t(usecs);
}

// static
timestamp_t	timestamp_t::Now(void)
{
	return timestamp_t{};
}

// static
timestamp_t	timestamp_t::FromDMS(const int64_t &dms)
{
	return timestamp_t(NowMicroSecs() + (dms * 1'000));
}

// static
timestamp_t	timestamp_t::FromDUS(const int64_t &dus)
{
	return timestamp_t(NowMicroSecs() + dus);
}

// static
timestamp_t	timestamp_t::FromBigBang(void)
{
	return timestamp_t(0ull);
}

void	timestamp_t::reset(void)
{
	m_usecs = NowMicroSecs();	
}

bool	timestamp_t::operator<(const timestamp_t &old_stamp) const
{
	return (GetUSecs() < old_stamp.GetUSecs());
}

bool	timestamp_t::operator==(const timestamp_t &old_stamp) const
{
	return (GetUSecs() == old_stamp.GetUSecs());
}

timestamp_t	timestamp_t::operator-(const timestamp_t &old_stamp) const
{
	const int64_t	d_usecs = GetUSecs() - old_stamp.GetUSecs();
	
	return timestamp_t(d_usecs);
}

timestamp_t	timestamp_t::OffsetMilliSecs(const int64_t &d_ms) const
{
	return timestamp_t(m_usecs + (d_ms * 1'000.0));
}

timestamp_t	timestamp_t::OffsetSecs(const double &d_secs) const
{
	return timestamp_t(m_usecs + (d_secs * 1'000'000.0));
}

timestamp_t	timestamp_t::OffsetHours(const double &d_hours) const
{
	const double	d_secs = d_hours * 60.0 * 60.0;
	
	return timestamp_t(m_usecs + (d_secs * 1'000'000.0));
}

int64_t	timestamp_t::delta_us(const timestamp_t &old_stamp) const
{
	const int64_t	d_usecs = GetUSecs() - old_stamp.GetUSecs();
	
	return d_usecs;
}
int64_t	timestamp_t::delta_ms(const timestamp_t &old_stamp) const	{return delta_us(old_stamp) / 1'000.0;}
double	timestamp_t::delta_secs(const timestamp_t &old_stamp) const	{return delta_us(old_stamp) / 1'000'000.0;}


int64_t	timestamp_t::GetUSecs(void) const noexcept	{return m_usecs;}

int64_t	timestamp_t::GetIntSecs(void) const 		{return GetUSecs() / 1'000'000;}
double	timestamp_t::GetSecs(void) const		{return GetUSecs() / 1'000'000.0;}

timestamp_t::stamppoint_t	timestamp_t::GetTimePoint(void) const
{	
	return stamppoint_t(milliseconds(m_usecs / 1'000ul));
}

// elap
int64_t	timestamp_t::elap_us(void) const	{return timestamp_t{}.delta_us(*this);}
int64_t	timestamp_t::elap_ms(void) const	{return timestamp_t{}.delta_ms(*this);}
double	timestamp_t::elap_secs(void) const	{return timestamp_t{}.delta_secs(*this);}

string	timestamp_t::elap_str(void) const
{
	const uint64_t	ms = elap_ms();
	const uint64_t	us = elap_us();
	// const uint64_t	ns = elap_ns(tp);
	const uint64_t	secs = elap_secs();
	
	const int	n_digits = 4;
	
	stringstream	ss;
	
	ss << setw(n_digits) << setfill(' ');
	
	/*if (std::log(ns) < n_digits)		ss << ns  << " nanosecs";
	else*/
	if (std::log10(us) < n_digits)		ss << us  << " microsecs";
	else if (std::log10(ms) < n_digits)	ss << ms   << " millisecs";
	else					ss << secs << " secs";
	
	return ss.str();
}

namespace LX
{

uint32_t raw(STAMP_FORMAT o)
{
	return static_cast<uint32_t>(o);
}

STAMP_FORMAT operator ~ (STAMP_FORMAT o)
{
	return STAMP_FORMAT(~raw(o));
}

STAMP_FORMAT operator | (STAMP_FORMAT a, STAMP_FORMAT b)
{
	return STAMP_FORMAT(raw(a) | raw(b));
}

STAMP_FORMAT operator & (STAMP_FORMAT a, STAMP_FORMAT b)
{
	return STAMP_FORMAT(raw(a) & raw(b));
}

bool	operator!(STAMP_FORMAT o)
{
	return (raw(o) == 0);
}

bool	any(STAMP_FORMAT o)
{
	return !!o;
}

} // namespace LX

//---- build timestamp STRING -------------------------------------------------

	// could theoretically use format flag "%q" or "%Q" for millisecs but may barf depending on platform/country ?

string	timestamp_t::str(const STAMP_FORMAT fmt0) const
{
	const size_t	MAX_TIME_STAMP_CHARS = 128;
	
	try
	{
		/*
		static
		int	s_dTZ = 100;
		
		if (100 == s_dTZ)	s_dTZ = 0;						// (could compute TZ delta ONCE?)
		*/
		
		char	buff[MAX_TIME_STAMP_CHARS];						// thread-safe but SLOWER
		size_t	index = 0;
		
		const int64_t		t_us = GetUSecs();
		const std::time_t	secs = t_us / 1'000'000ul;				// TERRIBLE resolution on windows?
		
		const bool		utc_f = any(fmt0 & STAMP_FORMAT::UTC);
		const STAMP_FORMAT	fmt = fmt0 & ~STAMP_FORMAT::UTC;
		
		#ifdef WIN32
			std::tm	tm_struct;
			
			if (utc_f)
				_gmtime64_s(&tm_struct, &secs/*__time64_t*/);
			else	_localtime64_s(&tm_struct, &secs);

			const auto	tm_p = &tm_struct;
		#else
			std::tm	tm_struct;
			
			if (utc_f)	gmtime_r(&secs, &tm_struct);
			else		localtime_r(&secs, &tm_struct);
			
			const auto	tm_p = &tm_struct;
		#endif		
		
		if (any(fmt & STAMP_FORMAT::YMD))
		{
			index += strftime(buff, sizeof(buff), "%Y-%m-%d", tm_p);
			assert(index < MAX_TIME_STAMP_CHARS);
		}
		
		if (any(fmt & STAMP_FORMAT::HMS))
		{
			if (any(fmt & STAMP_FORMAT::YMD))
			{
				buff[index++] = ' ';
			}
			
			index += strftime(buff + index, sizeof(buff) - index, "%H:%M:%S", tm_p);
			assert(index < MAX_TIME_STAMP_CHARS);
		}
		
		const unsigned int	remain_ms = (t_us - ((int64_t) secs * 1'000'000ul)) / 1'000ul;		// must typecast UP or goes to shit on x32
		
		if (any(fmt & STAMP_FORMAT::MS))
		{
			index += snprintf(buff + index, sizeof(buff) - index, ":%03u", remain_ms);
			assert(index < MAX_TIME_STAMP_CHARS);
		}
		
		if (any(fmt & STAMP_FORMAT::US))
		{
			const unsigned int	remain_us = t_us % 1'000ul;
		
			index += snprintf(buff + index, sizeof(buff) - index, ":%03u", remain_us);
			assert(index < MAX_TIME_STAMP_CHARS);
		}
		
		return string(buff, buff + index);
	}
	catch (...)
	{
		// apparently fickly
		assert(0);
	}
	
	return string("<failed>");
}

// nada mas
