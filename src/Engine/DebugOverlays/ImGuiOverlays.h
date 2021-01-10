#pragma once
#include "OverlayManager.h"


class ImGuiDemoOverlay : public DebugOverlay
{
public:
	ImGuiDemoOverlay() : DebugOverlay(false, "ImGuiDemoOverlay")
	{

	}

	virtual void render_overlay() override
	{
		ImGui::ShowDemoWindow(&_isOpen);
	}
};

class ImGuiAboutOverlay : public DebugOverlay
{
public:
	ImGuiAboutOverlay() : DebugOverlay(false, "ImGuiAboutOverlay")
	{

	}

	virtual void render_overlay() override
	{
		ImGui::ShowAboutWindow(&_isOpen);
	}
};
