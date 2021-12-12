#include "core.pch.h"
#include "Logging.h"

void Logger::Log(Severity severity, std::string const& msg) {
	Log({ severity, msg });
}
void Logger::Log(Severity severity, std::string const& category, std::string const& msg) {
	Log({ severity, fmt::format("[{}] {}", category, msg) });
}

void Logger::Log(LogEntry const& entry) {
	std::lock_guard lck{ _lock };
	_hasNewMessages = true;
	_buffer.push(entry);
}
