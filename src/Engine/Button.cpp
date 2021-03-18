#include "engine.pch.h"    // for compiler

#include "GameEngine.h"
#include "Button.h"
#include "Bitmap.h"

//-----------------------------------------------------------------
// Button methods
//-----------------------------------------------------------------
Button::Button() :
GUIBase(),
m_bArmed(false),
m_BmpReleasedPtr(nullptr),
m_BmpPressedPtr(nullptr),
m_bImageMode(false)
{
	m_DefaultBackColor = m_BackColor = COLOR(227, 227, 227);
}

Button::Button(const String& text) :
GUIBase(text),
m_bArmed(false),
m_BmpReleasedPtr(nullptr),
m_BmpPressedPtr(nullptr),
m_bImageMode(false)
{
	m_DefaultBackColor = m_BackColor = COLOR(227, 227, 227);
}

//Button::Button(const string& text) :
//	GUIBase(text),
//	m_bArmed(false),
//	m_BmpReleasedPtr(nullptr),
//	m_BmpPressedPtr(nullptr),
//	m_bImageMode(false)
//{
//	m_DefaultBackColor = m_BackColor = COLOR(227, 227, 227);
//}


Button::~Button()
{
}

//---------------------------
// Methods - Member functions
//---------------------------
void Button::Paint(graphics::D2DRenderContext& ctx)
{
	if (m_BoundingRect.bottom - m_BoundingRect.top <= 0 ||
		m_BoundingRect.right - m_BoundingRect.left <= 0)
	{
		MessageBoxA(NULL, "Impossible to draw the Button, it has no valid bounds!", "GameEngine says NO", MB_OK);
		exit(-1);
	}
	// store original font
	Font *originalFont = GameEngine::instance()->get_font();

	//automatically enable bitmapmode when the pointers are not nullptr
	if (m_BmpPressedPtr != nullptr && m_BmpReleasedPtr != nullptr) m_bImageMode = true;
	else m_bImageMode = false;

	if (!m_bImageMode) {
		DrawClassicButton(ctx);
	} else {
		DrawImageButton(ctx);
	}

	//restore font
	GameEngine::instance()->set_font(originalFont);
}

void Button::DrawClassicButton(graphics::D2DRenderContext& ctx)
{
	// Draw the borders
	RECT r = m_BoundingRect;
	ctx.set_color(COLOR(101, 101, 101));
	ctx.fill_rect(r.left, r.top, r.right, r.bottom);

	++r.left; ++r.top; --r.right; --r.bottom;
	if (!m_bArmed) ctx.set_color(COLOR(254, 254, 254));
	else ctx.set_color(COLOR(101, 101, 101));
	ctx.fill_rect(r.left, r.top, r.right, r.bottom);

	// Fill interior
	++r.left; ++r.top; --r.right; --r.bottom;
	ctx.set_color(m_BackColor);
	ctx.fill_rect(r.left, r.top, r.right, r.bottom);

	// Set the Font
	ctx.set_font(m_FontPtr);

	if (!m_bArmed)
	{

		ctx.set_color(COLOR(101, 101, 101));
		ctx.draw_line(m_BoundingRect.right - 1, m_BoundingRect.top + 1, m_BoundingRect.right - 1, m_BoundingRect.bottom - 1);
		ctx.draw_line(m_BoundingRect.left + 1, m_BoundingRect.bottom - 1, m_BoundingRect.right - 1, m_BoundingRect.bottom - 1);

		ctx.set_color(COLOR(160, 160, 160));
		ctx.draw_line(m_BoundingRect.right - 2, m_BoundingRect.top + 2, m_BoundingRect.right - 2, m_BoundingRect.bottom - 2);
		ctx.draw_line(m_BoundingRect.left + 2, m_BoundingRect.bottom - 2, m_BoundingRect.right - 2, m_BoundingRect.bottom - 2);

		// Draw fore color when this is enabled
		if (m_bEnabled)ctx.set_color(m_ForeColor);

		// gray when disabled
		else ctx.set_color(COLOR(187, 187, 187));

		ctx.draw_string(m_Text, m_BoundingRect);
	}
	else
	{
		ctx.set_color(COLOR(101, 101, 101));
		ctx.draw_line(m_BoundingRect.left + 2, m_BoundingRect.top + 2, m_BoundingRect.right - 2, m_BoundingRect.top + 2);
		ctx.draw_line(m_BoundingRect.left + 2, m_BoundingRect.top + 2, m_BoundingRect.left + 2, m_BoundingRect.bottom - 2);

		ctx.set_color(COLOR(160, 160, 160));
		ctx.draw_line(m_BoundingRect.left + 2, m_BoundingRect.top + 3, m_BoundingRect.right - 3, m_BoundingRect.top + 3);
		ctx.draw_line(m_BoundingRect.left + 3, m_BoundingRect.top + 3, m_BoundingRect.left + 3, m_BoundingRect.bottom - 3);

		//++r.left; ++r.top; --r.right; --r.bottom;
		//if (!m_bArmed) GameEngine::GetSingleton()->SetColor(COLOR(240, 240, 240));
		//else GameEngine::GetSingleton()->SetColor(COLOR(180, 180, 180));
		////GameEngine::GetSingleton()->SetColor(COLOR(240, 240, 240));
		//GameEngine::GetSingleton()->FillRect(r.left, r.top, r.right, r.bottom);
		ctx.set_color(m_ForeColor);
		ctx.draw_string(m_Text, m_BoundingRect.left + 2, m_BoundingRect.top + 2, m_BoundingRect.right + 2, m_BoundingRect.bottom + 2);
	}
}

void Button::DrawImageButton(graphics::D2DRenderContext& ctx)
{
	if (m_bArmed)
	{
		ctx.draw_bitmap(m_BmpPressedPtr, m_BoundingRect.left, m_BoundingRect.top);
	}
	else
	{
		ctx.draw_bitmap(m_BmpReleasedPtr, m_BoundingRect.left, m_BoundingRect.top);
	}
}

void Button::Tick(double)
{
	if (!m_bEnabled)
	{
		m_bArmed = false;
		return;
	}

	float2 mouseScreenSpace(GameEngine::instance()->get_mouse_pos_in_viewport().x, GameEngine::instance()->get_mouse_pos_in_viewport().y);
	float2 mouseViewSpace = mouseScreenSpace;

	//RMB in button rect armes the button and paint will draw the pressed button
	if (GameEngine::instance()->is_mouse_button_down(VK_LBUTTON) && PointInRect(m_BoundingRect, mouseViewSpace))
	{
		m_bArmed = true;
	}
	else
	{
		//if mouse button is released while in rect, then pressed is true
		if (m_bArmed && !GameEngine::instance()->is_mouse_button_down(VK_LBUTTON) && PointInRect(m_BoundingRect, mouseViewSpace))
		{
			m_bTriggered = true;
			m_bArmed = false;
		}
		//while armed the RMB is released or outside the rect, then armed is false
		else if (m_bArmed && (!GameEngine::instance()->is_mouse_button_down(VK_LBUTTON) || !PointInRect(m_BoundingRect, mouseViewSpace)))
		{
			m_bArmed = false;
		}
	}
}

bool Button::IsPressed() const
{
	MessageBoxA(NULL, "TextBox::IsPressed() called from a pointer that is a nullptr\nThe MessageBox that will appear after you close this MessageBox is the default error message from visual studio.", "GameEngine says NO", MB_OK);

	return m_bTriggered;
}

void Button::ConsumeEvent()
{
	m_bTriggered = false;
}

void Button::SetPressedBitmap(const String& filenameRef)
{
	m_BmpPressedPtr = new Bitmap(filenameRef);
}

//OWN METHOD
void Button::SetPressedBitmap(Bitmap* bmpPtr)
{
    m_BmpPressedPtr = bmpPtr;
}
void Button::SetPressedBitmap(int resourceID)
{
	m_BmpPressedPtr = new Bitmap(resourceID);
}

void Button::SetReleasedBitmap(const String& filenameRef)
{
	m_BmpReleasedPtr = new Bitmap(filenameRef);
}

void Button::SetReleasedBitmap(int resourceID)
{
	m_BmpReleasedPtr = new Bitmap(resourceID);
}
void Button::SetReleasedBitmap(Bitmap* bmpPtr)
{
    m_BmpReleasedPtr = bmpPtr;
}
void Button::SetImageMode(bool bImageMode)
{
	m_bImageMode = bImageMode;
}

TextureButton::TextureButton(std::string const& pressed, std::string const& released)
	: Button(String(""))
{
	m_BmpPressedPtr = new Bitmap(String(pressed.c_str()));
	m_BmpReleasedPtr = new Bitmap(String(released.c_str()));
}

TextureButton::~TextureButton()
{
	delete m_BmpReleasedPtr;
	delete m_BmpPressedPtr;

}

float TextureButton::GetWidth() const {
	return (float)m_BmpPressedPtr->GetWidth();
}

float TextureButton::GetHeight() const {
	return (float)m_BmpPressedPtr->GetHeight();
}
