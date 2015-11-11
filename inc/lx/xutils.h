// LX utilities

#pragma once

#include <cstdint>
#include <string>
#include <chrono>
#include <tuple>
#include <array>

#ifndef nil
	#define	nil	nullptr
#endif

namespace LX
{
// miracle enum CLASS hasher for unordered_set/map			// actually DUMBS DOWN enum to a size_t - is too brutal?
struct EnumClassHash
{
	template<typename _T>
	std::size_t operator()(_T t) const
	{
		return static_cast<std::size_t>(t);
	}
};

// RAII auto-lock (SINGLE-THREAD)
class AutoLock
{
public:
	AutoLock(bool &f)
		: m_Flag(f)
	{
		f = true;
	}			// should use C++11 instead
	~AutoLock()
	{
		m_Flag = false;
	}
private:
	bool	&m_Flag;
};

std::array<int, 4>	ToHumanTimeArray(const double secs);
int			Soft_stoi(const std::string &s, const int def);
double			Soft_stod(const std::string &s, const double def);

// stopwatch
class StopWatch
{
public:
	StopWatch();

	void	restart(void);
	
	std::string	elap_str(const bool &restart_f = false);
	double		elap_secs(void) const;
	
private:

	// timepoint type
	using tpt = std::chrono::high_resolution_clock::time_point;
	
	tpt	getnow(void) const;
	
	uint64_t	elap_ms(const tpt &tp) const;
	uint64_t	elap_micro(const tpt &tp) const;
	uint64_t	elap_nano(const tpt &tp) const;

	tpt	m_LastTimePoint;
};

// timestamp class
// using stampclock_t = std::chrono::steady_clock;		// won't get correct YEAR/MONTH/DAY/TZ
using stampclock_t = std::chrono::system_clock;			// may move BACK IN TIME
using stamppoint_t = stampclock_t::time_point;

class timestamp_t
{
public:
	// ctors
	timestamp_t();
	timestamp_t(const std::int64_t &t);
	
	static timestamp_t	Now(void);
	static timestamp_t	FromSecs(const double &secs);
	static timestamp_t	FromDMS(const int64_t &dms);
	static timestamp_t	FromDUS(const int64_t &dus);
	static timestamp_t	FromBigBang(void);
	static timestamp_t	FromPlusInfinity(void);
	
	timestamp_t&	reset(void);
	
	bool		operator<(const timestamp_t &old_stamp) const;
	bool		operator==(const timestamp_t &old_stamp) const;
	timestamp_t	operator-(const timestamp_t &old_stamp) const;
	
	timestamp_t	OffsetSecs(const double &d_secs) const;
	timestamp_t	OffsetMilliSecs(const int64_t &d_ms) const;
	timestamp_t	OffsetHours(const double &d_hours) const;
	
	std::int64_t	delta_us(const timestamp_t &old_stamp) const;
	std::int64_t	delta_ms(const timestamp_t &old_stamp) const;
	double		delta_secs(const timestamp_t &old_stamp) const;
	
	std::int64_t	elap_us(void) const;
	std::int64_t	elap_ms(void) const;
	double		elap_secs(void) const;
	
	std::int64_t	GetUSecs(void) const;
	std::int64_t	GetIntSecs(void) const;
	double		GetSecs(void) const;
	stamppoint_t	GetTimePoint(void) const;
	
private:

	std::int64_t	m_usecs;
};

// timestamp string
enum class STAMP_FORMAT : uint8_t
{
	NO_MILLISEC = 0,
	MILLISEC
};

std::string	xtimestamp_str(const timestamp_t &t_ms, const std::string &fmt = "%H:%M:%S:", const STAMP_FORMAT &stamp_fmt = STAMP_FORMAT::MILLISEC);		// from millisec timestamp
std::string	xtimestamp_str(const std::string &fmt = "%H:%M:%S:", const STAMP_FORMAT &stamp_fmt = STAMP_FORMAT::MILLISEC);
std::string	xdatestamp_str(const std::string &fmt = "%Y-%m-%d_%H:%M:%S");

void	xtrap(const char *s = nil);

} // namespace LX

// nada mas