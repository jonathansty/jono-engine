#pragma once

#include "singleton.h"
#include "RingBuffer.h"
#include <fmt/printf.h>
#include <fstream>

#include "PlatformIO.h"

// Severity levels used for logging
enum class LogSeverity {
	Verbose,
	Info,
	Warning,
	Error
};

enum class LogCategory {
	Unknown,
	Graphics,
	IO,
	System,
	Game,
	UI,
	Input
};

namespace logging {

inline const char* to_string(LogCategory category) {
	switch (category) {
		case LogCategory::Graphics:
			return "Graphics";
		case LogCategory::IO:
			return "IO";
		case LogCategory::System:
			return "System";
		case LogCategory::Game:
			return "Game";
		case LogCategory::UI:
			return "UI";
		case LogCategory::Input:
			return "Input";
		case LogCategory::Unknown:
		default:
			return "Unknown";
	}
}

inline const char* to_string(LogSeverity severity) {
	switch (severity) {
		case LogSeverity::Verbose:
			return "Verbose";
		case LogSeverity::Info:
			return "Info";
		case LogSeverity::Warning:
			return "Warning";
		case LogSeverity::Error:
			return "Error";
	}
}

}

// Namespace that contains helper functions related to logging
namespace Logging {
template <typename S, typename... Args>
void log(const char* file, int line, LogCategory category, LogSeverity severity, const S& format, Args&&... args) {
	std::string msg = fmt::vformat(format, fmt::make_args_checked<Args...>(format, args...));
	LogEntry entry{};
	entry._severity = severity;
	entry._message = msg;
	entry._file = file;
	entry._line = line;
	entry._category = category;
	Logger::instance()->log(entry);
}

} // namespace Logging




// Entry used to store log information
struct LogEntry {
	LogSeverity _severity;
	LogCategory _category;
	std::string _message;
	const char* _file;
	int _line;

	std::string to_message() const {
		return fmt::sprintf("[%s(%d)][%s][%s] %s", _file, _line, logging::to_string(_category), logging::to_string(_severity), _message.c_str());
	}
};

inline SYSTEMTIME get_system_time() {
	SYSTEMTIME time{};
	::GetSystemTime(&time);
	return time;
}

inline std::string get_timestamp(SYSTEMTIME const& time) {
	return fmt::format("{}{}{}{}{}{}", time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond);
}

// Log manager that allows storing log entries in memory
class Logger : public TSingleton<Logger> 
{
public:
	Logger();
	~Logger();

	static constexpr int c_buffer_size = 2048;

	using Severity = LogSeverity;

	void init();
	void deinit();
	void log(LogEntry const& entry);

	RingBuffer<LogEntry, c_buffer_size> const& GetBuffer() const { return _buffer; }

	// Hack to scroll down to bottom if new data was received
	bool _hasNewMessages = false;

private:
	bool _initialized;

	// Time we started the game and logging data 
	SYSTEMTIME _time;

	std::mutex _lock;
	RingBuffer< LogEntry, c_buffer_size> _buffer;

	IO::IFileRef _file;
};

// Expose some logging macros 
#define LOG(category, severity, message, ...) Logging::log(__FILE__, __LINE__, category, severity, message, __VA_ARGS__)
#define LOG_ERROR(category, message, ...)   LOG(LogCategory::category, LogSeverity::Error, message, __VA_ARGS__)
#define LOG_INFO(category, message, ...)    LOG(LogCategory::category, LogSeverity::Info, message, __VA_ARGS__)
#define LOG_VERBOSE(category, message, ...) LOG(LogCategory::category, LogSeverity::Verbose, message, __VA_ARGS__)
#define LOG_WARNING(category, message, ...) LOG(LogCategory::category, LogSeverity::Warning, message, __VA_ARGS__)
