#pragma once

namespace Threading
{

// Wrapper to manage threads with a run loop
class Thread
{
public:
	JONO_INLINE Thread(const char* name);

	Thread(Thread const&) = delete;
	Thread operator=(Thread const&) = delete;

	virtual ~Thread() 
	{ 
		if(m_Thread.joinable())
		{
			Join(); 
		}
	}

	void Terminate();

	void Join();

	JONO_INLINE void Execute();

	virtual void Run() = 0;

	JONO_INLINE bool IsRunning() const { return m_Running; }

protected:
	std::atomic<bool> m_Running;

	std::thread const& GetThread() const { return m_Thread; }
	std::string_view GetName() const { return m_Name; }

private:
	std::string m_Name;
	std::thread m_Thread;
};

JONO_INLINE void Thread::Execute()
{
	m_Thread = std::thread([this](){ this->Run(); });
}

JONO_INLINE void Thread::Join()
{
	assert(m_Thread.joinable());
	m_Thread.join();
}

JONO_INLINE void Thread::Terminate()
{
	m_Running = false;
}

JONO_INLINE Thread::Thread(const char* name)
		: m_Name(name)
		, m_Running(false)
{
}

} // namespace Threading
