#include "core.pch.h"
#include "Logging.h"

 Logger::Logger()
	 : _initialized(false)
	 , _time()
{
}

 Logger::~Logger() {
	assert(!_initialized);
}

void Logger::init() {
	
	_time = get_system_time();

	std::string time = get_timestamp(_time);
	std::string log_output_file = fmt::format("Logs/log_{}.txt", time);

	if(!IO::get()->exists("Logs")) {
		IO::get()->create_directory("Logs");
	}

	_file = IO::get()->open(log_output_file.c_str(), IO::Mode::Write);
	if(!_file){
		fmt::print("Failed to open log output file \"{}\"!", log_output_file);
	}

	_initialized = true;
}

void Logger::deinit() {
	if(_file) {
		IO::get()->close(_file);
		_file.reset();
	}
}

void Logger::log(LogEntry const& entry) {
	std::lock_guard lck{ _lock };
	_hasNewMessages = true;
	_buffer.push(entry);

	assert(_initialized);

	// This is currently ok to do straight on any thread because we are locking the entire buffer when pushing data into it. So this 
	// printing and writing code is piggy backing from that. Ideally this should be moved to a logger thread that waits for data through a semaphore and flushes every now and then.
	std::string msg = entry.to_message();
	fmt::print("{}\n", msg);

	if (_file){
		std::string txt = fmt::format("{}\n", msg);
		_file->write((void*)txt.c_str(), u32(txt.size()) * sizeof(char));
	}
}
