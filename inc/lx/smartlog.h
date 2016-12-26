
#pragma once

#include <string>

namespace LX
{
// import into namespace
using std::string;

class Controller;
class timestamp_t;


class ISmartLog
{
public:
	virtual ~ISmartLog() = default;
	
	virtual bool	IsOp(const timestamp_t &stamp, const LogLevel &level, const string &msg) = 0;
	
	static
	ISmartLog*	Create(Controller &controller);

	
protected:

	ISmartLog()	{}
};

} // namespace LX
