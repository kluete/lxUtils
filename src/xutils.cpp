// LX utilities

#include <cassert>
#include <sstream>
#include <chrono>
#include <utility>
#include <locale>
#include <iostream>
#include <cmath>
#include <iomanip>
#include <ctime>

#include <cstring>

#include <csignal>

#ifdef WIN32
	#include "Winsock.h"
	
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

//---- Debugger Trap Signal ---------------------------------------------------

void	LX::xtrap(const char *s)
{
	#ifdef LOG_SIGNAL_TRAP
		::kill(0, SIGTRAP);	// (copied from Juce)
	#endif
	
	#ifdef __MINGW64__
		assert(0);
	#endif
}

//---- Get Human Time array ---------------------------------------------------

array<int, 4>	LX::ToHumanTimeArray(const double secs)
{
	if (secs < 0)		return {{-1, -1 , -1, -1}};		// no time
	
	const uint64_t	tot_ms = secs * 1000;
	const uint64_t	tot_secs = secs;
	
	const int	h = tot_secs / 3600;
	const int	m = (tot_secs - (h * 3600)) / 60;
	const int	s = (tot_secs % 60);
	const int	ms = tot_ms % 1000;
	
	return {{h, m, s, ms}};
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
		
		return def;
	}
	
	return def;
}

//---- Soft (non-spurious) String to Int conversion ---------------------------

int	LX::Soft_stoi(const string &s, const int def)
{
	try
	{	
		if (s.empty())		return def;
		
		const int	v = stoi(s);
		return v;
	
	}
	catch (std::runtime_error &e)
	{
		const char	*what_s = e.what();	// (don't allocate)
		(void)what_s;
		
		return def;
	}
	
	return def;
}

//---- StopWatch CTOR ---------------------------------------------------------

	StopWatch::StopWatch()
{
	restart();
}
	
StopWatch::tpt	StopWatch::getnow(void) const
{
	// high-res timer @ small Stroustrup, p.124
	return high_resolution_clock::now();
}
	
void	StopWatch::restart(void)
{
	m_LastTimePoint = getnow();
}

uint64_t	StopWatch::elap_ms(const StopWatch::tpt &tp) const
{
	return duration_cast<milliseconds>(tp - m_LastTimePoint).count();
}

uint64_t	StopWatch::elap_micro(const StopWatch::tpt &tp) const
{
	return duration_cast<microseconds>(tp - m_LastTimePoint).count();
}

uint64_t	StopWatch::elap_nano(const StopWatch::tpt &tp) const
{
	return duration_cast<nanoseconds>(tp - m_LastTimePoint).count();
}

string	StopWatch::elap_str(const bool &restart_f)
{
	const auto	tp = getnow();
	
	const uint64_t	ms = elap_ms(tp);
	const uint64_t	mis = elap_micro(tp);
	const uint64_t	nas = elap_nano(tp);
	
	const uint64_t	secs = ms / 1000;
	
	const int	n_digits = 4;
	
	stringstream	ss;
	
	ss << setw(n_digits) << setfill(' ');
	
	if (std::log(nas) < n_digits)		ss << nas  << " nanosecs";
	else if (std::log10(mis) < n_digits)	ss << mis  << " microsecs";
	else if (std::log10(ms) < n_digits)	ss << ms   << " millisecs";
	else					ss << secs << " secs";
	
	if (restart_f)	restart();
	
	return ss.str();
}

double	StopWatch::elap_secs(void) const
{
	const auto	tp = getnow();
	
	const uint64_t	ms = elap_ms(tp);
	
	const double	secs = ms / 1000.0;
	return secs;
}

//==== timestamp ==============================================================

inline
int64_t	NowMicroSecs(void)
{
	const auto	tp = stampclock_t::now().time_since_epoch();
	const auto	elap_us_no_typ = duration_cast<microseconds>(tp);
	const int64_t	elap_us = elap_us_no_typ.count();
	
	return elap_us;
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
timestamp_t	timestamp_t::FromSecs(const double &secs)
{
	const int64_t	usecs = secs * 1'000'000;
	
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

// static
timestamp_t	timestamp_t::FromPlusInfinity(void)
{
	return timestamp_t().OffsetHours(24.0);						// HACK !
}

// reset
timestamp_t&	timestamp_t::reset(void)
{
	m_usecs = NowMicroSecs();
	
	return *this;
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

// elap
int64_t	timestamp_t::elap_us(void) const	{return timestamp_t{}.delta_us(*this);}
int64_t	timestamp_t::elap_ms(void) const	{return timestamp_t{}.delta_ms(*this);}
double	timestamp_t::elap_secs(void) const	{return timestamp_t{}.delta_secs(*this);}

int64_t	timestamp_t::GetIntSecs(void) const	{return GetUSecs() / 1'000'000;}
double	timestamp_t::GetSecs(void) const	{return GetUSecs() / 1'000'000.0;}

stamppoint_t	timestamp_t::GetTimePoint(void) const
{	
	return stamppoint_t(milliseconds(m_usecs / 1'000ul));
}

//---- build timestamp STRING -------------------------------------------------

string	LX::xtimestamp_str(const timestamp_t &t, const string &fmt, const STAMP_FORMAT &stamp_fmt)

	// could theoretically use format flag "%q" or "%Q" for millisecs but may barf depending on platform/country ?
{
	const size_t	MAX_TIME_STAMP_CHARS = 128;
	
	try
	{
		char	buff[MAX_TIME_STAMP_CHARS];
		
		const int64_t		t_us = t.GetUSecs();
		
		const std::time_t	secs = t_us / 1'000'000ul;
		
		const std::tm		*tm_p = std::localtime(&secs);				// thread-safe?
		assert(tm_p);
		
		const size_t	len = strftime(buff, sizeof(buff), fmt.c_str(), tm_p); 
		assert(len > 0);
		assert(len <= sizeof(buff));
		
		string	s {buff, len};
		
		if (stamp_fmt == STAMP_FORMAT::MILLISEC)
		{
			const unsigned int	remain_ms = (t_us - ((int64_t) secs * 1'000'000ul)) / 1'000ul;		// is CRAP unless typecast up

			char	ms_buff[8];				// writes 8 bytes on x86 ???
			
			::memset(&ms_buff[0], 0, sizeof(ms_buff));

			const size_t	ms_len = snprintf(ms_buff, sizeof(ms_buff), "%03ud", remain_ms);
			assert(ms_len > 0);
			assert(ms_len <= sizeof(ms_buff));
			
			s.append(ms_buff, 3);
		}
		
		return s;
	}
	catch (...)
	{
		// apparently fickly - can't log from lx utils!
		// uFatal("FATAL exception error in LX::xtimestamp_str()");
		assert(0);
	}
}

//---- timestamp string (wrapper) ---------------------------------------------

string	LX::xtimestamp_str(const string &fmt, const STAMP_FORMAT &stamp_fmt)
{
	return xtimestamp_str(timestamp_t{}, fmt, stamp_fmt);
}

//---- datestamp string -------------------------------------------------------

string	LX::xdatestamp_str(const string &fmt)
{
	return xtimestamp_str(timestamp_t{}, fmt, STAMP_FORMAT::NO_MILLISEC);
}

// nada mas
