#include "engine.stdafx.h"
#include "logging.h"

using namespace logging;

thread_local char buffer[max_msg_size * max_messages];
thread_local uint32_t current_message = 0;

std::mutex log_buffer_mutex;
std::vector<std::string> log_buffer;

void logging::clear()
{
	current_message = 0;
	memset(buffer, max_msg_size * max_messages, 0);
}

void logging::logf(char const* fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	char* offset = buffer + current_message * max_msg_size;
	vsnprintf(offset, max_msg_size, fmt, args);
	va_end(args);

	::OutputDebugStringA(offset);
	::printf(offset);

	current_message = (++current_message) % max_messages;
}

char const* logging::retrieve_message(uint32_t id)
{
	return buffer + id * max_msg_size;
}



