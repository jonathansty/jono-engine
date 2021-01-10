#include "stdafx.h"
#include "logging.h"

using namespace logging;

thread_local char buffer[msg_size * max_messages];
thread_local uint32_t current_message = 0;

void logging::clear()
{
	current_message = 0;
	memset(buffer, msg_size * max_messages, 0);
}

void logging::logf(char const* fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	char* offset = buffer + current_message * msg_size;
	vsnprintf(offset, msg_size, fmt, args);
	va_end(args);

	::OutputDebugStringA(offset);
	::printf(offset);

	current_message = (++current_message) % max_messages;
}

char const* logging::retrieve_message(uint32_t id)
{
	return buffer + id * msg_size;
}
