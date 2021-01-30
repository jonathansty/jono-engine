#include "game.stdafx.h"

#include "Slider.h"

Slider::Slider(int minValue, int maxValue,const String & textRef):
m_MinValue(minValue),
m_MaxValue(maxValue),
m_String(textRef),
m_Value(float(maxValue + minValue)/2.f)
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
float Slider::GetValue() 
{
	return m_Value;
}

void Slider::Tick(double deltaTime) 
{
	float2 p = GameEngine::instance()->get_mouse_pos_in_viewport();
	if (p.x > m_PosX &&
		p.x < m_PosX + m_Width &&
		p.y > m_PosY &&
		p.y < m_PosY + m_Height)
	{
		if (GameEngine::instance()->is_mouse_button_down(VK_LBUTTON))
		{
            m_IsPressed = true;
			m_Value = m_MinValue + (p.x - m_PosX) * (m_MaxValue - m_MinValue) / m_Width; // See Paint for how to draw this. pixels to value
		}
        if (GameEngine::instance()->is_mouse_button_released(VK_LBUTTON))
        {
            m_IsPressed = false;
        }
	}
	
	
}
void Slider::Paint(graphics::D2DRenderContext& ctx)
{
	ctx.set_color(m_BackgroundColor);
	ctx.fill_rect(m_PosX, m_PosY, m_PosX + m_Width, m_PosY + m_Height);

	ctx.set_color(m_ValueColor);
	ctx.fill_rect(m_PosX, m_PosY, int(m_PosX + (m_Value - m_MinValue) * m_Width / (m_MaxValue - m_MinValue) + 1), m_PosY + m_Height - 1); // See this. value to pixels

	ctx.set_color(COLOR(0, 0, 0));
	ctx.draw_rect(m_PosX, m_PosY, m_PosX + m_Width, m_PosY + m_Height);
	

	ctx.set_color(COLOR(0, 0, 0));
    ctx.set_color(m_ValueColor);
	ctx.draw_string(String(m_MinValue), m_PosX, m_PosY + m_Height + 2);
	ctx.draw_string(String(m_MaxValue), m_PosX + m_Width, m_PosY + m_Height + 2);
	ctx.draw_string(String(m_String) + String(": ") + String(m_Value) , m_PosX, m_PosY - m_Height);
}
bool Slider::IsPressed()
{
    return m_IsPressed;
}
void Slider::SetValue(float value)
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