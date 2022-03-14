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
	auto engine = GameEngine::instance();
	if (_isOpen)
	{

		ImGui::Begin(_name.c_str(), &_isOpen);

		static bool s_EnableVsync = GameEngine::instance()->get_vsync();
		if (ImGui::Checkbox("Enable VSync", &s_EnableVsync))
		{
			engine->set_vsync(s_EnableVsync);
		}

		bool enable_limiter = (GameEngine::instance()->_engine_settings.max_frame_time > 0.0f);
		s32 s_target = enable_limiter ? s32(1.0 / engine->_engine_settings.max_frame_time) : 0;
		if (ImGui::Checkbox("Enable Frame Limiter", &enable_limiter))
		{
			engine->_engine_settings.max_frame_time = m_Times[Timer::FrameTime] * 0.001;
			if(!enable_limiter)
			{
				engine->_engine_settings.max_frame_time = 0.0f;
			}
		}

		ImGui::SameLine();
		if(ImGui::InputInt("Target FPS", &s_target, 1, 5, ImGuiInputTextFlags_EnterReturnsTrue) && enable_limiter)
		{
			s_target = std::max(30, s_target);
			engine->_engine_settings.max_frame_time = 1.0 / (f64)s_target;
		}

		f64 fps = 1.0 / (m_Times[Timer::FrameTime] * 0.001);
		ImGui::Text("FPS: %d", int(fps));


		ImGui::Text("FrameTime:  %.4f ms", m_Times[Timer::FrameTime].average());
		ImGui::Text("GameUpdate: %.4f ms", m_Times[Timer::GameUpdateCPU].average());
		ImGui::Text("RenderCPU:  %.4f ms", m_Times[Timer::RenderCPU].average());
		ImGui::Text("RenderGPU:  %.4f ms", m_Times[Timer::RenderGPU].average());
		ImGui::Text("PresentCPU:  %.4f ms", m_Times[Timer::PresentCPU].average());

		ImGui::End();
	}
}

void MetricsOverlay::UpdateTimer(Timer timer, f64 time)
{
	assert(timer >= 0 && timer < Timer::Num);
	m_Times[timer].add_sample(time);
}

const std::string MetricsOverlay::m_Name = "Metrics";

