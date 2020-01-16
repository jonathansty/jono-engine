//-----------------------------------------------------
// Name: Steyfkens	
// First name: Jonathan
// Group: 1DAE5
//-----------------------------------------------------
#include "stdafx.h"
//---------------------------
// Includes
//---------------------------
#include "Slider.h"

//---------------------------
// Defines
//---------------------------
#define GAME_ENGINE (GameEngine::GetSingleton())

//---------------------------
// Constructor & Destructor
//---------------------------

Slider::Slider(int minValue, int maxValue,const String & textRef):
m_MinValue(minValue),
m_MaxValue(maxValue),
m_String(textRef),
m_Value((maxValue + minValue)/2)
{

}
Slider::Slider(int minValue, int maxValue) :
Slider(minValue, maxValue, String("Value"))
{
	// nothing to create
}
Slider::~Slider()
{
	// nothing to destroy
}

//---------------------------
// Methods - Member functions
//---------------------------

// Add here the methods - Member functions
void Slider::SetText(const String & textRef){
	m_String = textRef;
}
void Slider::SetBounds(int posX, int posY, int width, int height)
{
	m_PosX = posX;
	m_PosY = posY;
	m_Width = width;
	m_Height = height;
}
void Slider::SetValueColor(const COLOR & valueColorRef){
	m_ValueColor = valueColorRef;
}
void Slider::SetBackColor(const COLOR & backColorRef){
	m_BackgroundColor = backColorRef;
}
int Slider::GetValue(){
	return m_Value;
}
void Slider::Tick(double deltaTime){
	POINT p = GAME_ENGINE->GetMousePosition();
	if (p.x > m_PosX &&
		p.x < m_PosX + m_Width &&
		p.y > m_PosY &&
		p.y < m_PosY + m_Height)
	{
		if (GAME_ENGINE->IsMouseButtonDown(VK_LBUTTON))
		{
            m_IsPressed = true;
			m_Value = m_MinValue + (p.x - m_PosX) * (m_MaxValue - m_MinValue) / m_Width; // See Paint for how to draw this. pixels to value
		}
        if (GAME_ENGINE->IsMouseButtonReleased(VK_LBUTTON))
        {
            m_IsPressed = false;
        }
	}
	
	
}
void Slider::Paint(){
	GAME_ENGINE->SetColor(m_BackgroundColor);
	GAME_ENGINE->FillRect(m_PosX, m_PosY, m_PosX + m_Width, m_PosY + m_Height);

	GAME_ENGINE->SetColor(m_ValueColor);
	GAME_ENGINE->FillRect(m_PosX, m_PosY, m_PosX + (m_Value - m_MinValue) * m_Width / (m_MaxValue - m_MinValue) + 1, m_PosY + m_Height - 1); // See this. value to pixels

	GAME_ENGINE->SetColor(COLOR(0, 0, 0));
	GAME_ENGINE->DrawRect(m_PosX, m_PosY, m_PosX + m_Width, m_PosY + m_Height);
	

	GAME_ENGINE->SetColor(COLOR(0, 0, 0));
    GAME_ENGINE->SetColor(m_ValueColor);
	GAME_ENGINE->DrawString(String(m_MinValue), m_PosX, m_PosY + m_Height + 2);
	GAME_ENGINE->DrawString(String(m_MaxValue), m_PosX + m_Width, m_PosY + m_Height + 2);
	GAME_ENGINE->DrawString(String(m_String) + String(": ") + String(m_Value) , m_PosX, m_PosY - m_Height);
}
bool Slider::IsPressed()
{
    return m_IsPressed;
}
void Slider::SetValue(int value)
{
    if (m_Value > m_MinValue && m_Value < m_MaxValue)
    {
        m_Value = value;
    }
    else
    {
        std::cout << "The value is not in range for " << m_String.C_str() << std::endl;
    }
}