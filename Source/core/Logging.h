#pragma once

#include "singleton.h"
#include "RingBuffer.h"

// Severity levels used for logging
enum class LogSeverity {
	Verbose,
	Info,
	Warning,
	Error
};

// Namespace that contains helper functions related to logging
namespace Logging {
template <typename S, typename... Args>
void log(const char* file, int line, LogSeverity severity, const S& format, Args&&... args) {
	std::string msg = fmt::vformat(format, fmt::make_args_checked<Args...>(format, args...));
	LogEntry entry{};
	entry._severity = severity;
	entry._message = msg;
	entry._file = file;
	entry._line = line;
	Logger::instance()->Log(entry);
}

} // namespace Logging




// Entry used to store log information
struct LogEntry {
	LogSeverity _severity;
	std::string _message;
	const char* _file;
	int _line;
};

// Log manager that allows storing log entries in memory
class Logger : public TSingleton<Logger> 
{
public:
	static constexpr int c_buffer_size = 2048;

	using Severity = LogSeverity;


	void Log(Severity severity, std::string const& msg);
	void Log(Severity severity, std::string const& category, std::string const& msg);
	void Log(LogEntry const& entry);

	RingBuffer<LogEntry, c_buffer_size> const& GetBuffer() const { return _buffer; }

	// Hack to scroll down to bottom if new data was received
	bool _hasNewMessages = false;

private:
	std::mutex _lock;
	RingBuffer< LogEntry, c_buffer_size> _buffer;
};

// Expose some logging macros 
#define LOG(severity, message, ...) Logging::log(__FILE__, __LINE__,severity, message, __VA_ARGS__)
#define LOG_ERROR(message, ...) LOG(LogSeverity::Error, message, __VA_ARGS__)
#define LOG_INFO(message, ...) LOG(LogSeverity::Info, message, __VA_ARGS__)
#define LOG_VERBOSE(message, ...) LOG(LogSeverity::Verbose, message, __VA_ARGS__)
#define LOG_WARNING(message, ...) LOG(LogSeverity::Warning, message, __VA_ARGS__)
