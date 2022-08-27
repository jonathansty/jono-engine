#include "Assert.h"
#include "core.pch.h"

using std::string;

#ifdef WIN64
void mb_error(const char* file, int line, std::string const& msg)
{
	string s;
	if (!msg.empty())
	{
		s = fmt::format("{} ({}): {}", file, line, msg);
	}
	else
	{
		s = fmt::format("An error was encountered in : {} ({})", file, line);
	}

	int output = ::MessageBoxA(NULL, s.c_str(), "Critical error encountered!", MB_RETRYCANCEL);
	if (output == IDCANCEL)
	{
		// crash
		*(int*)0 = 0;
	}
	if (output == IDRETRY)
	{
		return;
	}
}
#else
void mb_error(const char* file, int line, std::string const& msg)
{
	static_assert("unimplemented");
}
#endif
