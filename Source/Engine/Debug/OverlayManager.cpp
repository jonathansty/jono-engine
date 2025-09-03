#include "engine.pch.h"
#include "OverlayManager.h"

void OverlayManager::register_overlay(DebugOverlay* overlay)
{
	assert(_overlays.find(overlay->GetName()) == _overlays.end());
	_overlays[overlay->GetName()] = overlay;
}

void OverlayManager::unregister_overlay(DebugOverlay* overlay)
{
	assert(_overlays.find(overlay->GetName()) != _overlays.end());
	_overlays.erase(overlay->GetName());
}

DebugOverlay* OverlayManager::get_overlay(std::string const& name)
{
	return _overlays[name];
}

std::vector<DebugOverlay*> OverlayManager::get_overlays() const
{
	std::vector<DebugOverlay*> overlays;
	std::transform(_overlays.begin(), _overlays.end(), std::back_inserter(overlays), [](std::pair<std::string, DebugOverlay*> const& element)
			{ return element.second; });
	return overlays;
}

OverlayManager::~OverlayManager()
{
	for (auto& overlay : _overlays)
	{
		delete overlay.second;
	}
	_overlays.clear();
}

void OverlayManager::RenderOverlay()
{
	if (m_IsOpen)
	{
		if (ImGui::Begin(m_Name.c_str(), &m_IsOpen))
		{
			for (auto& overlay : _overlays)
			{
				ImGui::PushID(overlay.second);
				ImGui::Checkbox("", &overlay.second->m_IsOpen);
				ImGui::SameLine();
				char const* const d = overlay.second->GetName();
				ImGui::Text(d);
				ImGui::PopID();
			}
			//
		}
		ImGui::End();
	}

	for (auto& overlay : _overlays)
	{
		if (overlay.second->m_IsOpen)
		{
			overlay.second->RenderOverlay();
		}
	}
}

void OverlayManager::RenderViewport()
{
	for (auto& overlay : _overlays)
	{
		if (overlay.second->m_IsOpen)
		{
			overlay.second->RenderViewport();
		}
	}
}

void OverlayManager::Render3D(RenderContext& ctx)
{
	for (auto& overlay : _overlays)
	{
		if (overlay.second->m_IsOpen)
		{
			overlay.second->Render3D(ctx);
		}
	}
}

 DebugOverlay::DebugOverlay(bool isOpen, std::string name)
		: m_IsOpen(isOpen)
		, m_Name(name)
{
}

const char* DebugOverlay::GetName() const
{
	return m_Name.c_str();
}

void DebugOverlay::SetVisible(bool visible)
{
	if (!m_IsOpen)
	{
		ImGui::SetWindowFocus("Overlays");
	}
	m_IsOpen = visible;
}
