#include "core.pch.h"

#include "Logging.h"

Logger::Logger()
		: _initialized(false)
		, _time()
		, _running(false)
{
}

Logger::~Logger()
{
	assert(!_initialized);
}

void Logger::init()
{
	_running = true;
	_time = get_system_time();

	_worker = std::thread(std::bind(&Logger::thread_flush, this));

	_initialized = true;
}

void Logger::deinit()
{
	_running = false;
	_worker.join();
}

void Logger::log(LogEntry const& entry)
{
	std::lock_guard lck{ _lock };
	_hasNewMessages = true;
	_buffer.push(entry);


	assert(_initialized);

	// This is currently ok to do straight on any thread because we are locking the entire buffer when pushing data into it. So this
	// printing and writing code is piggy backing from that. Ideally this should be moved to a logger thread that waits for data through a semaphore and flushes every now and then.
	std::string msg = entry.to_message();
	fmt::print("{}\n", msg);

	{
		std::string txt = fmt::format("{}\n", msg);
		_to_flush.push_back(txt);
		OutputDebugStringA(txt.c_str());

		_cv.notify_one();
	}
}

void Logger::thread_flush()
{
	::SetThreadDescription(GetCurrentThread(), L"LoggingThread");
	if (!IO::get()->exists("Logs"))
	{
		IO::get()->create_directory("Logs");
	}

	std::string time = get_timestamp(_time);
	std::string log_output_file = fmt::format("Logs/log_{}.txt", time);

	auto file = IO::get()->open(log_output_file.c_str(), IO::Mode::Write);
	if (!file)
	{
		fmt::print("Failed to open log output file \"{}\"!", log_output_file);
	}

	while(_running)
	{
		std::unique_lock lk{ _lock };
		_cv.wait(lk);
		std::vector<std::string> data = std::move(_to_flush);
		lk.unlock();

		if(file)
		{
			for (std::string const& line : data)
			{
				file->write((void*)line.c_str(), u32(line.size()) * sizeof(char));
			}
		}
	}

	if (file)
	{
		IO::get()->close(file);
		file.reset();
	}
}
