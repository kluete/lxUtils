// lx utils freeformlog

#include <cassert>
#include <algorithm>
#include <fstream>
#include <mutex>
#include <thread>

#include "lx/ulog.h"

using namespace std;
using namespace LX;

// static
rootLog	*rootLog::s_rootLog = nil;

//==== Log Slot (may have multiple) ===========================================

	LogSlot::LogSlot()
		: m_OrgSignal{nil}
{
}

	LogSlot::~LogSlot()
{	
	DisconnectSelf();
}	

void	LogSlot::DisconnectSelf(void)
{
	if (!m_OrgSignal)	return;
	
	m_OrgSignal->Disconnect(this);
	
	// (was set to nil by signal)
	assert(nil == m_OrgSignal);
}

void	LogSlot::SetSignal(LogSignal *sig)
{
	m_OrgSignal = sig;
}

void	LogSlot::RemoveSignal(void)
{
	m_OrgSignal = nil;
}

void	LogSlot::LogAtLevel_LL(const timestamp_t stamp, const LogLevel level, const string &msg) 
{
	if (!m_OrgSignal)	return;		// was already disconnected
	
	LogAtLevel(stamp, level, msg);
}

//==== Log Signal (currently singleton) =======================================

	LogSignal::LogSignal()
		: m_SlotList{}
{
}
	
	LogSignal::~LogSignal()
{
	DisconnectAll();
}

void	LogSignal::Connect(LogSlot *slot)
{
	assert(slot);
	
	// check no duplicates
	assert(find(m_SlotList.begin(), m_SlotList.end(), slot) == m_SlotList.end());
	
	m_SlotList.push_back(slot);
	
	slot->SetSignal(this);
}

void	LogSignal::Disconnect(LogSlot *slot)
{
	assert(slot);
	
	auto	it = find(m_SlotList.begin(), m_SlotList.end(), slot);
	assert(m_SlotList.end() != it);
	
	// remove from list first so can log during disconnection (?)
	m_SlotList.erase(it);
	
	slot->RemoveSignal();
}

//---- Disconnect All slots ---------------------------------------------------

void	LogSignal::DisconnectAll(void)
{
	while (m_SlotList.size() > 0)
	{
		Disconnect(m_SlotList.back());
	}
}

//---- Emit All ---------------------------------------------------------------

	// triggers all connected slots

void	LogSignal::EmitAll(const timestamp_t stamp, const LogLevel level, const string &msg) const
{
	// need MUTEX ?
	//   NO: if re-logs from a signal would lock up (?)
	
	for (LogSlot *slot : m_SlotList)
	{
		slot->LogAtLevel_LL(stamp, level, msg);
	}
}

//---- Clear Log All ----------------------------------------------------------

void	LogSignal::ClearLogAll(void)
{
	for (LogSlot *slot : m_SlotList)
	{
		slot->ClearLog();
	}
}

//==== rootLog (unique) ========================================================

//---- CTOR -------------------------------------------------------------------

	rootLog::rootLog()
		: m_EnabledLevelSet{}
{
	// (singleton)
	assert(!s_rootLog);
	s_rootLog = this;
	
	EnableLevels({FATAL, EXCEPTION, ERROR, WARNING, MSG});
}
	
//---- DTOR -------------------------------------------------------------------

	rootLog::~rootLog()
{
	assert(s_rootLog);
	s_rootLog = nil;
}

//----- Get Singleton instance ------------------------------------------------

// static
rootLog*rootLog::GetSingleton(void)
{
	return s_rootLog;
}

//----- Get Singleton reference -----------------------------------------------

// static
rootLog&	rootLog::Get(void)
{
	assert(s_rootLog);
	
	return *s_rootLog;
}

unordered_set<LogLevel>	rootLog::GetEnabledLevels(void) const
{
	return m_EnabledLevelSet;
}


bool	rootLog::IsLevelEnabled(const LogLevel lvl) const
{
	return m_EnabledLevelSet.count(lvl);
}


//---- Has Log Level LOW-LEVEL ------------------------------------------------

// static
bool	rootLog::HasLogLevel_LL(const LogLevel lvl)
{
	if (!s_rootLog)		return false;		// not yet initialized or already exited
	
	const bool	f = s_rootLog->IsLevelEnabled(lvl);
	return f;
}

//---- Do ULog LOW-LEVEL ------------------------------------------------------

// static
void	rootLog::DoULog_LL(const LogLevel lvl, const string &msg)
{
	assert(s_rootLog);
	
	s_rootLog->DoULog(lvl, msg);
}

//---- Do ULog ----------------------------------------------------------------

void	rootLog::DoULog(const LogLevel lvl, const string &msg)
{
	if (!IsLevelEnabled(lvl))		return;		// level not enabled
	
	// need MUTEX ?
	
	const LX::timestamp_t	now{};
	
	EmitAll(now, lvl, msg);
}

//---- Clear All Log Levels ---------------------------------------------------

rootLog&	rootLog::ClearAllLevels(void)
{
	m_EnabledLevelSet.clear();
	
	return *this;
}

//---- Enable Log Levels ------------------------------------------------------

rootLog&	rootLog::EnableLevels(const unordered_set<LogLevel> &levels)
{
	m_EnabledLevelSet.insert(levels.begin(), levels.end());
	
	return *this;
}

rootLog&	rootLog::EnableLevel(const char *level_s)
{
	return EnableLevels({log_hash(level_s)});
}

//---- Disable Log Levels -----------------------------------------------------

rootLog&	rootLog::DisableLevels(const unordered_set<LogLevel> &levels)
{
	// m_EnabledLevelSet.erase(levels.begin(), levels.end());		// doesn't work?
	for (const auto lvl : levels)
	{
		m_EnabledLevelSet.erase(lvl);		
	}
	
	return *this;
}

//---- Toggle (one) Level -----------------------------------------------------

rootLog&	rootLog::ToggleLevel(const LogLevel lvl, const bool f)
{
	if (f)
		m_EnabledLevelSet.insert(lvl);
	else	m_EnabledLevelSet.erase(lvl);
	
	return *this;
}

//---- File Log ---------------------------------------------------------------

class FileLog : public LogSlot
{
public:
	// ctor
	FileLog(const string &fname, const STAMP_FORMAT fmt, const double min_elap_secs)
		: LogSlot{},
		m_Fmt(fmt),
		m_MinSepElapSecs(min_elap_secs),
		m_ThreadID(this_thread::get_id()),
		m_OFS {fname, ios_base::trunc}
	{
		assert(m_OFS && m_OFS.is_open());
	}
	// dtor
	virtual ~FileLog()	{}
	
	// IMP
	void	LogAtLevel(const timestamp_t stamp, const LogLevel level, const string &msg) override
	{
		unique_lock<mutex>	locker(m_Mutex);
		
		const double	delta_secs = std::min(stamp.delta_secs(m_LastStamp), 80.0);
		m_LastStamp = stamp;
		
		if (delta_secs > m_MinSepElapSecs)
		{
			const string	sep_s = string((size_t)delta_secs, '-');
			
			m_OFS << sep_s << endl;
		}
		
		if (this_thread::get_id() != m_ThreadID)
		{
			// OFF-THREAD
			m_OFS << stamp.str(m_Fmt) << " _THREAD " << hex << this_thread::get_id() << " : " << msg << endl;
		}
		else
		{	m_OFS << stamp.str(m_Fmt) << " " << msg << endl;
		}
	}
	
private:
	
	const STAMP_FORMAT	m_Fmt;
	const double		m_MinSepElapSecs;
	const thread::id	m_ThreadID;
	mutable mutex		m_Mutex;
	ofstream		m_OFS;
	timestamp_t		m_LastStamp;
};

//---- instantiate ------------------------------------------------------------

LogSlot*	LogSlot::Create(const LOG_TYPE_T log_t, const string &fn, const STAMP_FORMAT fmt, const double min_elap_secs)
{
	switch (log_t)
	{
		case LOG_TYPE_T::STD_FILE:
		
			return new FileLog(fn, fmt, min_elap_secs);
			break;
		
		default:
		
			return nil;
			break;
	}
}


// nada mas
