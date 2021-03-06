#pragma once
#include "GUIBase.h"

class TextLabel : public GUIBase
{
public:
	TextLabel(std::string const& text);
	virtual ~TextLabel();

	virtual void Paint(graphics::D2DRenderContext& ctx);
	virtual void Tick(double) {}
	// initializes members for all constructors 
	virtual void HandleKeyInput(TCHAR) {};
	virtual void ConsumeEvent() {}

};
