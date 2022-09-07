#include "engine.pch.h"
#include "GameEngine.h"
#include "MetricsOverlay.h"
#include "Memory.h"

#include <inttypes.h>

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

		if(ImGui::CollapsingHeader("Memory", ImGuiTreeNodeFlags_DefaultOpen))
		{
			MemoryTracker* tracker = get_memory_tracker();
			ImGui::Text("Usage: %.4f (MB)", (double)tracker->_total_memory_usage / 1'000'000.0);
			ImGui::Text("Allocations: %" PRIi64, (s64)tracker->_current_allocs);

			std::vector<std::pair<MemoryCategory, size_t>> data{};
			for(u32 i = 0; i < *MemoryCategory::Count; ++i)
			{
				data.push_back({ (MemoryCategory)i, tracker->_total_per_category[i] });
			}
			std::sort(data.begin(), data.end(),[](auto const& lhs, auto const& rhs) { return lhs.second > rhs.second; });
			for(auto const& d : data)
			{
				ImGui::Text("\t%s (KB): %.4f", MemoryCategoryToString(d.first), (double)d.second / 1'000);
			}
		}


		if (ImGui::CollapsingHeader("Timers", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::Text("FrameTime:  %.4f ms", m_Times[Timer::FrameTime].last());
			ImGui::Text("GameUpdate: %.4f ms", m_Times[Timer::GameUpdateCPU].last());
			ImGui::Text("EventProcessing:  %.4f ms", m_Times[Timer::EventHandlingCPU].last());
			ImGui::Text("RenderCPU:  %.4f ms", m_Times[Timer::RenderCPU].last());
			ImGui::Text("RenderGPU:  %.4f ms", m_Times[Timer::RenderGPU].last());
			ImGui::Text("PresentCPU:  %.4f ms", m_Times[Timer::PresentCPU].last());
		}

		ImGui::End();
	}
}

void MetricsOverlay::UpdateTimer(Timer timer, f64 time)
{
	assert(timer >= 0 && timer < Timer::Num);
	m_Times[timer].add_sample(time);
}

const std::string MetricsOverlay::m_Name = "Metrics";

