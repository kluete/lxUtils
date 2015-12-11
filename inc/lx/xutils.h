// LX utilities

#pragma once

#include <cstdint>
#include <string>
#include <chrono>

namespace LX
{

// based on Daniel "djb" Bernstein's hasher (http://www.cse.yorku.ca/~oz/hash.html)
template<typename _intype>
constexpr
_intype	djb2_hash_impl(const char* text, _intype prev_hash)
{
	return text[0] == '\0' ? prev_hash : djb2_hash_impl(&text[1], prev_hash * 33 ^ static_cast<_intype>(text[0]));
}
	
#ifndef nil
	#define	nil	nullptr
#endif // nil
	
// hasher for unordered_set/map	(DUMBS DOWN enum to a size_t - is too brutal?)
struct EnumClassHash
{
	template<typename _T>
	std::size_t operator()(_T t) const
	{
		return static_cast<std::size_t>(t);
	}
};

int	Soft_stoi(const std::string &s, const int def);
double	Soft_stod(const std::string &s, const double def);

// timestamp string format
enum class STAMP_FORMAT : uint8_t
{
	SECOND = 0,
	MILLISEC
};

//---- Timestamp --------------------------------------------------------------

class timestamp_t
{
	// using stampclock_t = std::chrono::steady_clock;			// won't get correct YEAR/MONTH/DAY/TZ
	using stampclock_t = std::chrono::system_clock;				// may move back in time on system-time-adjust
public:
	using stamppoint_t = typename stampclock_t::time_point;
	
	// ctor
	timestamp_t();
	
	static timestamp_t	Now(void);
	static timestamp_t	FromSecs(const double &secs);
	static timestamp_t	FromDMS(const int64_t &dms);
	static timestamp_t	FromDUS(const int64_t &dus);
	static timestamp_t	FromBigBang(void);
	
	bool		operator<(const timestamp_t &old_stamp) const;
	bool		operator==(const timestamp_t &old_stamp) const;
	timestamp_t	operator-(const timestamp_t &old_stamp) const;
	
	timestamp_t	OffsetSecs(const double &d_secs) const;
	timestamp_t	OffsetMilliSecs(const int64_t &d_ms) const;
	timestamp_t	OffsetHours(const double &d_hours) const;
	
	std::int64_t	GetUSecs(void) const noexcept;
	std::int64_t	GetIntSecs(void) const;
	double		GetSecs(void) const;
	stamppoint_t	GetTimePoint(void) const;
	
	std::int64_t	delta_us(const timestamp_t &old_stamp) const;
	std::int64_t	delta_ms(const timestamp_t &old_stamp) const;
	double		delta_secs(const timestamp_t &old_stamp) const;
	
	std::int64_t	elap_us(void) const;
	std::int64_t	elap_ms(void) const;
	double		elap_secs(void) const;
	std::string	elap_str(void) const;

	std::string	stamp_str(const std::string &fmt = "%H:%M:%S:", const STAMP_FORMAT &stamp_fmt = STAMP_FORMAT::MILLISEC) const;

	void		reset(void);		// (only non-const function)

protected:

	timestamp_t(const std::int64_t &t);
	
	static std::int64_t	NowMicroSecs(void);

private:

	std::int64_t	m_usecs;		// would be faster w/ const ?
};

std::string	xtimestamp_str(const timestamp_t &stamp, const std::string &fmt = "%H:%M:%S:", const STAMP_FORMAT &stamp_fmt = STAMP_FORMAT::MILLISEC);
std::string	xtimestamp_str(const std::string &fmt = "%H:%M:%S:", const STAMP_FORMAT &stamp_fmt = STAMP_FORMAT::MILLISEC);
std::string	xdatestamp_str(const std::string &fmt = "%Y-%m-%d_%H:%M:%S");

void	xtrap(const char *s = nullptr);

} // namespace LX

// nada mas
