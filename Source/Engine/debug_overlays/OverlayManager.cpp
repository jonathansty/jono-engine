#include "engine.pch.h"
#include "OverlayManager.h"

void OverlayManager::register_overlay(DebugOverlay* overlay)
{
	assert(_overlays.find(overlay->get_name()) == _overlays.end());
	_overlays[overlay->get_name()] = overlay;
}

void OverlayManager::unregister_overlay(DebugOverlay* overlay)
{
	assert(_overlays.find(overlay->get_name()) != _overlays.end());
	_overlays.erase(overlay->get_name());
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
	if (_isOpen)
	{
		if (ImGui::Begin(_name.c_str(), &_isOpen))
		{
			for (auto& overlay : _overlays)
			{
				ImGui::PushID(overlay.second);
				ImGui::Checkbox("", &overlay.second->_isOpen);
				ImGui::SameLine();
				char const* const d = overlay.second->get_name();
				ImGui::Text(d);
				ImGui::PopID();
			}
			//
		}
		ImGui::End();
	}

	for (auto& overlay : _overlays)
	{
		if (overlay.second->_isOpen)
		{
			overlay.second->RenderOverlay();
		}
	}
}

void OverlayManager::RenderViewport()
{
	for (auto& overlay : _overlays)
	{
		if (overlay.second->_isOpen)
		{
			overlay.second->RenderViewport();
		}
	}
}

void OverlayManager::Render3D(ID3D11DeviceContext* ctx)
{
	for (auto& overlay : _overlays)
	{
		if (overlay.second->_isOpen)
		{
			overlay.second->Render3D(ctx);
		}
	}
}

 DebugOverlay::DebugOverlay(bool isOpen, std::string name)
		: _isOpen(isOpen)
		, _name(name)
{
}

const char* DebugOverlay::get_name() const
{
	return _name.c_str();
}

void DebugOverlay::set_visible(bool visible)
{
	if (!_isOpen)
	{
		ImGui::SetWindowFocus("Overlays");
	}
	_isOpen = visible;
}
