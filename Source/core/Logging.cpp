#include "core.pch.h"
#include "Logging.h"

void Logger::Log(LogEntry const& entry) {
	std::lock_guard lck{ _lock };
	_hasNewMessages = true;
	_buffer.push(entry);

	// For now print to console aswell even though this isn't thread safe!
	printf("%s\n", entry.to_message().c_str());
}
