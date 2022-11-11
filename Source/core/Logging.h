#pragma once

#include "RingBuffer.h"
#include "singleton.h"
#include <fmt/printf.h>
#include <fstream>

#include "PlatformIO.h"

// Severity levels used for logging
enum class LogSeverity
{
	Verbose,
	Info,
	Warning,
	Error,
	Fatal
};

enum class LogCategory
{
	Unknown,
	Graphics,
	IO,
	System,
	Game,
	UI,
	Input
};

namespace logging
{

CORE_API inline const char* to_string(LogCategory category)
{
	switch (category)
	{
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

CORE_API inline const char* to_string(LogSeverity severity)
{
	switch (severity)
	{
		case LogSeverity::Verbose:
			return "Verbose";
		case LogSeverity::Info:
			return "Info";
		case LogSeverity::Warning:
			return "Warning";
		case LogSeverity::Error:
			return "Error";
		case LogSeverity::Fatal:
			return "Fatal";
	}
	return "";
}

} // namespace logging

// Entry used to store log information
#pragma warning(push)
#pragma warning(disable : 4251)
struct CORE_API LogEntry
{
	LogSeverity _severity;
	LogCategory _category;
	const char* _file;
	int _line;

	std::string to_message() const
	{
		return fmt::sprintf("[%s(%d)][%s][%s] %s", _file, _line, logging::to_string(_category), logging::to_string(_severity), GetMessage().c_str());
	}

	std::string const& GetMessage() const { return _message; }

	std::thread::id GetThreadID() const { return _thread_id; }

	std::thread::id _thread_id;
	std::string _message;
};
#pragma warning(pop)

inline SYSTEMTIME get_system_time()
{
	SYSTEMTIME time{};
	::GetSystemTime(&time);
	return time;
}

inline std::string get_timestamp(SYSTEMTIME const& time)
{
	return fmt::format("{}{}{}{}{}{}", time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond);
}

#pragma warning(push)
#pragma warning(disable : 4251)

// Log manager that allows storing log entries in memory
class CORE_API Logger : public TSingleton<Logger>
{
public:
	Logger();
	~Logger();

	static constexpr int c_buffer_size = 512;

	using Severity = LogSeverity;

	void init();
	void deinit();
	void log(LogEntry const& entry);

	void clear() { _buffer.clear(); }
	RingBuffer<LogEntry, c_buffer_size> const& GetBuffer() const { return _buffer; }

	// Hack to scroll down to bottom if new data was received
	bool _hasNewMessages = false;

private:
	void thread_flush();

	std::atomic<bool> _running;
	bool _initialized;

	// Time we started the game and logging data
	SYSTEMTIME _time;

	std::mutex _lock;
	std::condition_variable _cv;
	RingBuffer<LogEntry, c_buffer_size> _buffer;

	std::vector<std::string> _to_flush;
	std::thread _worker;

	IO::IFileRef _file;
};
#pragma warning(pop)

// Namespace that contains helper functions related to logging
namespace Logging
{
template <typename S, typename... Args>
void log(const char* file, int line, LogCategory category, LogSeverity severity, const S& format, Args&&... args)
{
	// For now return manually here. This should be driven by some configuration
	if (severity == LogSeverity::Verbose)
		return;

	std::thread::id current_id = std::this_thread::get_id();

	std::string msg = fmt::vformat(format, fmt::make_format_args(args...));
	LogEntry entry = LogEntry();
	entry._severity = severity;
	entry._thread_id = current_id;
	entry._message = msg;
	entry._file = file;
	entry._line = line;
	entry._category = category;
	Logger::instance()->log(entry);
}

} // namespace Logging

// Expose some logging macros
#define LOG(category, severity, message, ...) Logging::log(__FILE__, __LINE__, category, severity, message, __VA_ARGS__)

#define LOG_FATAL(category, message, ...)                                 \
	LOG(LogCategory::category, LogSeverity::Fatal, message, __VA_ARGS__); \
	assert("Fatal error occurred! See log for more details.")

#define LOG_ERROR(category, message, ...) LOG(LogCategory::category, LogSeverity::Error, message, __VA_ARGS__)
#define LOG_INFO(category, message, ...) LOG(LogCategory::category, LogSeverity::Info, message, __VA_ARGS__)
#define LOG_VERBOSE(category, message, ...) LOG(LogCategory::category, LogSeverity::Verbose, message, __VA_ARGS__)
#define LOG_WARNING(category, message, ...) LOG(LogCategory::category, LogSeverity::Warning, message, __VA_ARGS__)
