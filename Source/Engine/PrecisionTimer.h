//-----------------------------------------------------------------
// Game Engine Object
// C++ Header - version v2_16 jan 2015 
// Copyright DAE Programming Team
// http://www.digitalartsandentertainment.be/
//-----------------------------------------------------------------

#pragma once

class PrecisionTimer final
{
public:
	PrecisionTimer();
	~PrecisionTimer();

	// Returns the elapsed time since Tick was called
	double get_delta_time(); // in seconds

	void reset(); 
	void start(); 
	void stop(); 

	// Returns the state
	bool is_stopped() const;

private:
	f64 _seconds_per_tick;

	u64 _start_time;
	u64 _stop_time;
	u64 _delta_time;

	bool _stopped;

};

class Timer final
{
public:
	// -------------------------
	// Constructors & Destructor
	// -------------------------
	Timer();
	~Timer();

	// -------------------------
	// General Methods
	// -------------------------

	// Stores the start time
	void Start(); // Call when unpaused.
	// Stores the current time
	void Stop();  // Call when paused.

	// Returns the state
	bool IsStopped() const { return m_StopTime; }

	double GetTime() const 
	{ 
		LARGE_INTEGER freq;
		::QueryPerformanceFrequency(&freq);
		return (double)(m_StopTime - m_BaseTime) / freq.QuadPart;
	}

	double GetTimeInMS() const
	{
		return GetTime() * 1000.0;
	}
	


private:

	__int64 m_BaseTime;
	__int64 m_StopTime;

	bool m_bStopped;

	Timer(const Timer& sRef);
	Timer& operator=(const Timer& sRef);
};