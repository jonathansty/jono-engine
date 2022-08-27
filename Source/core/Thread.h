#pragma once

namespace Threading
{

// Wrapper to manage threads with a run loop
class Thread
{
public:
	Thread()
	{
		_running = true;
		_thread = std::thread([this]()
				{ this->run(); });
	}

	Thread(Thread const&) = delete;
	Thread operator=(Thread const&) = delete;

	virtual ~Thread() {}

	void terminate()
	{
		_running = false;
	}

	void join()
	{
		assert(_thread.joinable());
		_thread.join();
	}

	virtual void run() = 0;

	bool is_running() const { return _running; }

private:
	std::thread _thread;

	std::atomic<bool> _running;
};

} // namespace Threading
