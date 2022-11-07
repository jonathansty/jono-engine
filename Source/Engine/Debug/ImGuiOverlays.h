#pragma once
#include "OverlayManager.h"


class ImGuiDemoOverlay : public DebugOverlay
{
public:
	ImGuiDemoOverlay() : DebugOverlay(false, "ImGuiDemoOverlay")
	{

	}

	virtual void RenderOverlay() override
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

	virtual void RenderOverlay() override
	{
		ImGui::ShowAboutWindow(&_isOpen);
	}
};
