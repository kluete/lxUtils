
#pragma once

#include <string>

namespace LX
{
// import into namespace
using std::string;

class ISmartLog
{
public:
	virtual ~ISmartLog() = default;
	
	virtual bool	IsSmartLevel(const timestamp_t stamp, const LogLevel level, const string &msg, const size_t thread_id) = 0;
	
	static
	ISmartLog*	*Create(Controller &controller);

	
protected:

	ISmartLog()	{}
};

} // namespace LX
