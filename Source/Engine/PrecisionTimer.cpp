#include "engine.pch.h"
#include "PrecisionTimer.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include "windows.h"

//=======================================================================================
// PrecisionTimer.cpp by Frank Luna (C) 2008 All Rights Reserved.
// Adapted for DAE GP1 by Bart Uyttenhove
//=======================================================================================
PrecisionTimer::PrecisionTimer() : _seconds_per_tick(0.0), _start_time(0), _stop_time(0), _stopped(false)
{
	__int64 countsPerSec;
	QueryPerformanceFrequency((LARGE_INTEGER*)&countsPerSec);
	_seconds_per_tick = 1.0 / (f64)countsPerSec;

	reset();
}

PrecisionTimer::~PrecisionTimer()
{}

double PrecisionTimer::get_delta_time() 
{
	if(!_stopped)
	{
		QueryPerformanceCounter((LARGE_INTEGER*)&_stop_time);
		_delta_time = _stop_time - _start_time;

	}
	return (f64)_delta_time * _seconds_per_tick;
}

void PrecisionTimer::reset()
{
	u64 currTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&currTime);

	_stop_time = currTime;
	_start_time = currTime;
	_stopped = false;
}

void PrecisionTimer::start()
{
	QueryPerformanceCounter((LARGE_INTEGER*)&_start_time);
	_stop_time = _start_time;
	_stopped = false;
}

void PrecisionTimer::stop()
{
	QueryPerformanceCounter((LARGE_INTEGER*)&_stop_time);

	_delta_time = _stop_time - _start_time;

	_stopped = true;
}

bool PrecisionTimer::is_stopped() const
{
	return _stopped;
}

void Timer::Start()
{
	assert(m_bStopped);
	QueryPerformanceCounter((LARGE_INTEGER*)&m_BaseTime);
	m_bStopped = false;
}

void Timer::Stop()
{
	assert(!m_bStopped);
	QueryPerformanceCounter((LARGE_INTEGER*)&m_StopTime);
	m_bStopped = true;

}

Timer::Timer()
	: m_bStopped(true)
{

}

Timer::~Timer()
{

}
