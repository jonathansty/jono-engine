#pragma once
#include "debug_overlays/OverlayManager.h"


template<int ValueCount>
class RunningAverage
{
public:

	void add_sample(f64 value)
	{
		_values[_idx] = value;
		_idx = ++_idx % ValueCount;
	}

	operator f64() {
		return average();
	}

	f64 average()
	{
		f64 result = 0.0;
		for(auto i : _values)
		{
			result += i; 
		}

		result /= (f64)ValueCount;
		return result;
	}

private:
	u32 _idx = 0;
	std::array<f64, ValueCount> _values;

};

// Standard metrics overlay for showing:
// - FPS
// - Frametime
class MetricsOverlay : public DebugOverlay
{
public:
	MetricsOverlay(bool isOpen = false);

	~MetricsOverlay();

	MetricsOverlay(MetricsOverlay const&) = delete;
	MetricsOverlay& operator=(MetricsOverlay const&) = delete;

	void render_overlay() override;

	enum Timer
	{
		FrameTime,
		GameUpdateCPU,
		RenderCPU,
		RenderGPU,
		Num
	};

	void UpdateTimer(Timer timer, f64 time);

private:
	static const std::string m_Name;

	std::array<RunningAverage<144>, Timer::Num> m_Times;

};
