
#include <cassert>
#include <string>
#include <vector>
#include <list>
#include <regex>
#include <fstream>

#include "lx/xutils.h"
#include "lx/xstring.h"

#include "Controller.h"
#include "VisibleLogs.h"
#include "LogDecoder.h"

#include "lx/smartlog.h"

using namespace	std;
using namespace LX;
using namespace juce;

class SmartLog : public ISmartLog
{
public:
	SmartLog(Controller &controller)
		: m_Controller(controller), m_Logs(*controller.GetLogs())
	{
			
	}
	
	bool	IsSmartLevel(const timestamp_t stamp, const LogLevel level, const string &msg, const size_t thread_id) override
	{
		
		
	}
	
	
	Controller	&m_Controller;
	IVisibleLogs	&m_Logs;
};

//static
ISmartLog*	ISmartLog::Create(Controller &controller)
{
	return new SmartLog(controller);
}

// nada mas
