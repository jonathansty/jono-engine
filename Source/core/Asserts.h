#pragma once

CORE_API void mb_error(const char* file, int line, std::string const& msg);

#define ASSERT(condition) \
	if (!(condition))     \
	mb_error(__FILE__, __LINE__, "")

#define ASSERTMSG(condition, msg, ...)                               \
	if (!(condition))                                                \
	{                                                                \
		mb_error(__FILE__, __LINE__, fmt::format(msg, __VA_ARGS__)); \
	}

#define FAILMSG(msg, ...) mb_error(__FILE__, __LINE__, fmt::format(msg, __VA_ARGS__))