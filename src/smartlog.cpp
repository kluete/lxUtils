
#include <cassert>
#include <string>
#include <vector>
#include <list>
#include <regex>
#include <fstream>

#include "lx/xutils.h"
#include "lx/xstring.h"

#include "logImp.h"

#include "Controller.h"
#include "VisibleLogs.h"
#include "LogDecoder.h"

#include "lx/smartlog.h"

using namespace	std;
using namespace LX;
using namespace juce;

// inline log level name definitions
static const
regex	INLINE_LOG_DEF_RE
{
	"^LOG_NAMES\\(\"([^\"]+)\"\\)$", regex::ECMAScript | regex::optimize
};

//---- Declare All Log Levels -----------------------------------------

static
vector<string>	SplitLogNames(const string &ln_s)
{
	// try decode log levels
	smatch	collect_match;
	
	if (!regex_match(ln_s, collect_match, INLINE_LOG_DEF_RE))
	{
		uErr("error : this log format is deprecated");
		return {};					// no log level found
	}
	
	assert(collect_match.size() == 2);
		
	const string	levels_string = collect_match[1];
		
	vector<string>	names_list;
	
	const regex	re{R"((\w+)\s+)", regex::ECMAScript | regex::optimize};
	
	for (sregex_iterator p(levels_string.begin(), levels_string.end(), re); p != sregex_iterator{}; ++p)
	{
		const string	level_s = (*p)[1];
		names_list.push_back(level_s);
	}
	
	return names_list;
}

//---- Dispatch Log Names -----------------------------------------------------

unordered_map<LogLevel, string>	DispatchLogNames(string ln)
{	
	// msg has all log names concatenated in string
	const vector<string>	names = SplitLogNames(ln);
	if (names.empty())	return {};
			
	// unordered_map<int, LogLevel>	log_index_to_level_map;
	unordered_map<LogLevel, string>	lvl_name_map;	//  = m_Controller.GetAllLevelNames();

	for (int k = 0; k < names.size(); k++)
	{
		const string	level_s = names[k];
		const LogLevel	lvl = log_hash(level_s.c_str());
		
		uLog(DECODER, "level[%03d] = %S", k, level_s);
		
		// assert(!log_index_to_level_map.count(k));
		// log_index_to_level_map.emplace(k, lvl);
		
		// assert(!lvl_name_map.count(lvl));
		lvl_name_map.emplace(lvl, level_s);
		if (lvl_name_map.count(lvl))	continue;		// ignore dupes silently (should check collisiion?)
	}
	
	// m_LogIndexToLevelMap = log_index_to_level_map;
	return lvl_name_map;
}
		
class SmartLog : public ISmartLog
{
public:
	SmartLog(Controller &controller)
		: m_Controller(controller), m_Logs(*controller.GetLogs())
	{
			
	}
	
	virtual ~SmartLog() = default;
	
	bool	IsOp(const timestamp_t &stamp, const LogLevel &level, const string &msg) override
	{
		if (!LogSlot::IsLogOp(level))	return false;
		
		if (level == LOG_DEF)
		{	const unordered_map<LogLevel, string>	lvl_name_map = DispatchLogNames(msg);
			
			m_Controller.SetAllLevelNames(lvl_name_map);
			return true;
		}
		
		return false;
	}
	
private:
	
	Controller	&m_Controller;
	IVisLogs	&m_Logs;
};

//static
ISmartLog*	ISmartLog::Create(Controller &controller)
{
	return new SmartLog(controller);
}

// nada mas
