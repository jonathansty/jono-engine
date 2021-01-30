#pragma once
//-----------------------------------------------------
// Name: Steyfkens
// First name: Jonathan
// Group: 1DAE5
//-----------------------------------------------------

//-----------------------------------------------------
// Include Files
//-----------------------------------------------------
#include "game.stdafx.h"

//-----------------------------------------------------
// Slider Class									
//-----------------------------------------------------
class Slider
{
public:
	Slider(int minValue, int maxValue );
	Slider(int minValue, int maxValue, const String & textRef);
	virtual ~Slider( );

	// C++11 make the class non-copyable
	Slider( const Slider& ) = delete;
	Slider& operator=( const Slider& ) = delete;
	//-------------------------------------------------
	// Methods - Member functions							
	//-------------------------------------------------
	void Tick(double deltaTime);
	void Paint(graphics::D2DRenderContext& ctx);

	void SetText(const String & textRef);
	void SetBounds(int posX, int posY, int width, int height);
	void SetValueColor(const COLOR & valueColorRef);
	void SetBackColor(const COLOR & backColorRef);
	float GetValue();
    void SetValue(float value);
    bool IsPressed();

private: 
	//-------------------------------------------------
	// Datamembers								
	//-------------------------------------------------
	int m_MinValue, m_MaxValue;

	int m_PosX = 0; 
	int m_PosY = 0; 
	int m_Width = 0; 
	int m_Height = 0;
	float m_Value = 100;
	COLOR m_ValueColor;
	COLOR m_BackgroundColor;
	String m_String;
    bool m_IsPressed = false;
};

 
