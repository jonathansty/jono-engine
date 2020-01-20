#pragma once
#include "GUIBase.h"

class TextLabel : public GUIBase
{
public:
	TextLabel(std::string const& text);
	virtual ~TextLabel();

	virtual void Paint();
	virtual void Tick(double deltaTime) {}
	// initializes members for all constructors 
	virtual void HandleKeyInput(TCHAR character) {};
	virtual void ConsumeEvent() {}

};
