#include "stdafx.h"
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

void OverlayManager::render_overlay() 
{
	if (_isOpen)
	{
		ImGui::Begin(_name.c_str(), &_isOpen);

		for (auto& overlay : _overlays)
		{
			ImGui::Checkbox("", &overlay.second->_isOpen); ImGui::SameLine(); ImGui::Text(overlay.second->get_name());
		}
		// 
		ImGui::End();
	}
}