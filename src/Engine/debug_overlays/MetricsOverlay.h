#pragma once
#include "debug_overlays/OverlayManager.h"

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

	void UpdateTimer(Timer timer, float time);

private:
	static const std::string m_Name;

	std::array<float, Timer::Num> m_Times;

};
