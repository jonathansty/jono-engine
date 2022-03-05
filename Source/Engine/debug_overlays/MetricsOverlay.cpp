#include "engine.pch.h"
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
		f64 fps = 1.0 / (m_Times[Timer::FrameTime] * 0.001);
		ImGui::Text("FPS: %d", int(fps));
		ImGui::Text("FrameTime: %.2f", m_Times[Timer::FrameTime].average());
		ImGui::Text("GameUpdate: %.2f", m_Times[Timer::GameUpdateCPU].average());
		ImGui::Text("RenderCPU: %.2f", m_Times[Timer::RenderCPU].average());
		ImGui::Text("RenderGPU: %.2f", m_Times[Timer::RenderGPU].average());

		ImGui::End();
	}
}

void MetricsOverlay::UpdateTimer(Timer timer, f64 time)
{
	assert(timer >= 0 && timer < Timer::Num);
	m_Times[timer].add_sample(time);
}

const std::string MetricsOverlay::m_Name = "Metrics";

