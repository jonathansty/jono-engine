#include "engine.stdafx.h"
#include "GameEngine.h"
#include "MetricsOverlay.h"

MetricsOverlay::MetricsOverlay(bool isOpen)
	: DebugOverlay(isOpen, "MetricsOverlay")
	, m_Times({})
{

}

MetricsOverlay::~MetricsOverlay()
{

}

void MetricsOverlay::render_overlay()
{
	if (_isOpen)
	{
		ImGui::Begin(_name.c_str(), &_isOpen);

		static bool s_EnableVsync = GameEngine::instance()->get_vsync();
		if (ImGui::Checkbox("Enable VSync", &s_EnableVsync))
		{
			GameEngine::instance()->set_vsync(s_EnableVsync);
		}
		double fps = 1.0 / (m_Times[Timer::FrameTime] * 0.001);
		ImGui::Text("FPS: %d", int(fps));
		ImGui::Text("FrameTime: %.2f", m_Times[Timer::FrameTime]);
		ImGui::Text("GameUpdate: %.2f", m_Times[Timer::GameUpdateCPU]);
		ImGui::Text("RenderCPU: %.2f", m_Times[Timer::RenderCPU]);
		ImGui::Text("RenderGPU: %.2f", m_Times[Timer::RenderGPU]);

		ImGui::End();
	}
}

void MetricsOverlay::UpdateTimer(Timer timer, float time)
{
	assert(timer >= 0 && timer < Timer::Num);
	m_Times[timer] = time;
}

const std::string MetricsOverlay::m_Name = "Metrics";

