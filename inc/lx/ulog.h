// lx utils freeform logger

#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <stdexcept>
#include <thread>

#include "lx/xutils.h"
#include "lx/xstring.h"

// forward declarations
namespace LX
{
class LogSignal;

using std::string;
using std::thread;
using std::vector;
using std::unordered_set;

using LOG_HASH_T = uint32_t;			// 32-bit is enough?

constexpr
auto	log_hash(const char* text)		// char[] would be safer?
{
	return djb2_hash_impl<LOG_HASH_T>(text, 5381);
}

constexpr
LOG_HASH_T operator "" _log(const char *s, size_t n)
{
	return log_hash(s);
}

using LogLevel = LOG_HASH_T;

enum class LOG_TYPE_T : int
{
	STD_FILE = 1,
};

//---- Log Slot ---------------------------------------------------------------

class LogSlot
{
	friend class LogSignal;
	
public:
	LogSlot();
	virtual ~LogSlot();

	void	DisconnectSelf(void);

	virtual void	LogAtLevel(const timestamp_t stamp_ms, const LogLevel level, const string &msg) = 0;
	virtual void	ClearLog(void)		{}
	
	bool		IsMainThread(void) const;
	
	static LogSlot*	Create(const LOG_TYPE_T log_t, const string &fn);

private:

	// accessed by signal
	void	LogAtLevel_LL(const timestamp_t stamp_ms, const LogLevel level, const string &msg);
	
	void	SetSignal(LogSignal *sig);
	void	RemoveSignal(void);
	
	LogSignal		*m_OrgSignal;
	
	const thread::id	m_ThreadID;
	
	// no class copy
	LogSlot(const LogSlot &) = delete;
	LogSlot &operator=(const LogSlot &) = delete;
};

//---- Log Signal -------------------------------------------------------------

class LogSignal
{
public:
	LogSignal();
	virtual ~LogSignal();

	void	Connect(LogSlot *slot);
	void	Disconnect(LogSlot *slot);

	void	EmitAll(const timestamp_t stamp_us, const LogLevel level, const string &msg) const;
	void	ClearLogAll(void);
	
private:

	void	DisconnectAll(void);

	vector<LogSlot*>	m_SlotList;

	// no class copy
	LogSignal(const LogSignal &) = delete;
	LogSignal &operator=(const LogSignal &) = delete;
};

//---- Root (top) Log ---------------------------------------------------------

class rootLog: public LogSignal
{
public:
	rootLog();
	virtual ~rootLog();

	void	DoULog(const LogLevel lvl, const string &msg);
	
	// functions
	rootLog&	ClearAllLevels(void);
	rootLog&	EnableLevels(const unordered_set<LogLevel> &enable_set);
	rootLog&	DisableLevels(const unordered_set<LogLevel> &disable_set);
	rootLog&	ToggleLevel(const LogLevel lvl, const bool f);
	rootLog&	EnableLevel(const char *level_s);
	
	bool	IsLevelEnabled(const LogLevel lvl) const;
	unordered_set<LogLevel>	GetEnabledLevels(void) const;
	
	static rootLog*	GetSingleton(void);
	static rootLog&	Get(void);
	static bool	HasLogLevel_LL(const LogLevel lvl);
	static void	DoULog_LL(const LogLevel lvl, const string &msg);
	
private:

	unordered_set<LogLevel>		m_EnabledLevelSet;
	mutable timestamp_t		m_LastTimeStamp;
	
	static rootLog			*s_rootLog;		// no garantee on initialization order?
	
	// no class copy
	rootLog(const rootLog &) = delete;
	rootLog& operator=(const rootLog&) = delete;
};

// these are just convenience aliases, done at compile-time hence free
// aliasing multiple symbol names to the same hash is always ok
// redefining the same symbol (with same hash) is ok in the same "context"
#define BASE_LOG_MACRO(t)	constexpr auto	t = #t##_log;

BASE_LOG_MACRO(	FATAL)
BASE_LOG_MACRO(	ERROR)
BASE_LOG_MACRO(	EXCEPTION)
BASE_LOG_MACRO(	WARNING)
BASE_LOG_MACRO(	MSG)
BASE_LOG_MACRO(	DTOR)
BASE_LOG_MACRO(	UNIT)
BASE_LOG_MACRO(	DELAYER)
BASE_LOG_MACRO(	APP_INIT)
BASE_LOG_MACRO(	SIG)
BASE_LOG_MACRO(	CROSS_THREAD)

#undef BASE_LOG_MACRO

} // namespace LX

template<typename ... Args>
void	uLog(const LX::LogLevel lvl, const char *fmt, Args&& ... args)
{
	try
	{
		if (!LX::rootLog::HasLogLevel_LL(lvl))	return;		// (won't preempt log string unfolding)
			
		const std::string	msg = LX::xsprintf(fmt, std::forward<Args>(args) ...);
		LX::rootLog::DoULog_LL(lvl, msg);
	}
	catch (std::runtime_error &e)
	{
		const char	*what_s = e.what();	// (don't allocate)
		LX::xtrap(what_s);
		
		throw e;	// re-throw
	}
}

// overloads

template<typename ... Args>
void	uLog(const LX::LogLevel lvl, const std::string &fmt, Args&& ... args)
{
	uLog(lvl, fmt.c_str(), std::forward<Args>(args) ...);
}

template<typename ... Args>
void	uLog(const char lvl_s[], const char *fmt, Args&& ... args)
{
	uLog(LX::log_hash(lvl_s), fmt, std::forward<Args>(args) ...);
}

// base shortcuts/wrappers

template<typename ... Args>
void	uMsg(const std::string &fmt, Args&& ... args)
{
	uLog(LX::MSG, fmt, std::forward<Args>(args) ...);
}

template<typename ... Args>
void	uWarn(const std::string &fmt, Args&& ... args)
{
	uLog(LX::WARNING, fmt, std::forward<Args>(args) ...);
}

template<typename ... Args>
void	uErr(const std::string &fmt, Args&& ... args)
{
	uLog(LX::ERROR, fmt, std::forward<Args>(args) ...);
}

template<typename ... Args>
void	uExcept(const std::string &fmt, Args&& ... args)
{
	uLog(LX::EXCEPTION, fmt, std::forward<Args>(args) ...);
}

template<typename ... Args>
void	uFatal(const std::string &fmt, Args&& ... args)
{
	uLog(LX::FATAL, fmt, std::forward<Args>(args) ...);
}

// nada mas
