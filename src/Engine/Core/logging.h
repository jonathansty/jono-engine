#pragma once
namespace logging
{
	constexpr uint32_t max_messages = 100;
	constexpr uint32_t msg_size = 512;

	void clear();

	void logf(char const* fmt, ...);

	char const* retrieve_message(uint32_t id);

}
