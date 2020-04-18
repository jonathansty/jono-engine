#include "stdafx.h"    // for compiler
#include "../stdafx.h" // for intellisense

#include "Button.h"

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
void Button::Paint()
{
	if (m_BoundingRect.bottom - m_BoundingRect.top <= 0 ||
		m_BoundingRect.right - m_BoundingRect.left <= 0)
	{
		MessageBoxA(NULL, "Impossible to draw the Button, it has no valid bounds!", "GameEngine says NO", MB_OK);
		exit(-1);
	}
	// store original font
	Font *originalFont = game_engine::instance()->get_font();

	//automatically enable bitmapmode when the pointers are not nullptr
	if (m_BmpPressedPtr != nullptr && m_BmpReleasedPtr != nullptr) m_bImageMode = true;
	else m_bImageMode = false;

	if (!m_bImageMode) DrawClassicButton();
	else DrawImageButton();

	//restore font
	game_engine::instance()->set_font(originalFont);
}

void Button::DrawClassicButton()
{
	// Draw the borders
	RECT r = m_BoundingRect;
	game_engine::instance()->set_color(COLOR(101, 101, 101));
	game_engine::instance()->FillRect(r.left, r.top, r.right, r.bottom);

	++r.left; ++r.top; --r.right; --r.bottom;
	if (!m_bArmed) game_engine::instance()->set_color(COLOR(254, 254, 254));
	else game_engine::instance()->set_color(COLOR(101, 101, 101));
	game_engine::instance()->FillRect(r.left, r.top, r.right, r.bottom);

	// Fill interior
	++r.left; ++r.top; --r.right; --r.bottom;
	game_engine::instance()->set_color(m_BackColor);
	game_engine::instance()->FillRect(r.left, r.top, r.right, r.bottom);

	// Set the Font
	game_engine::instance()->set_font(m_FontPtr);

	if (!m_bArmed)
	{

		game_engine::instance()->set_color(COLOR(101, 101, 101));
		game_engine::instance()->DrawLine(m_BoundingRect.right - 1, m_BoundingRect.top + 1, m_BoundingRect.right - 1, m_BoundingRect.bottom - 1);
		game_engine::instance()->DrawLine(m_BoundingRect.left + 1, m_BoundingRect.bottom - 1, m_BoundingRect.right - 1, m_BoundingRect.bottom - 1);

		game_engine::instance()->set_color(COLOR(160, 160, 160));
		game_engine::instance()->DrawLine(m_BoundingRect.right - 2, m_BoundingRect.top + 2, m_BoundingRect.right - 2, m_BoundingRect.bottom - 2);
		game_engine::instance()->DrawLine(m_BoundingRect.left + 2, m_BoundingRect.bottom - 2, m_BoundingRect.right - 2, m_BoundingRect.bottom - 2);

		// Draw fore color when this is enabled
		if (m_bEnabled)game_engine::instance()->set_color(m_ForeColor);

		// gray when disabled
		else game_engine::instance()->set_color(COLOR(187, 187, 187));

		game_engine::instance()->DrawString(m_Text, m_BoundingRect);
	}
	else
	{
		game_engine::instance()->set_color(COLOR(101, 101, 101));
		game_engine::instance()->DrawLine(m_BoundingRect.left + 2, m_BoundingRect.top + 2, m_BoundingRect.right - 2, m_BoundingRect.top + 2);
		game_engine::instance()->DrawLine(m_BoundingRect.left + 2, m_BoundingRect.top + 2, m_BoundingRect.left + 2, m_BoundingRect.bottom - 2);

		game_engine::instance()->set_color(COLOR(160, 160, 160));
		game_engine::instance()->DrawLine(m_BoundingRect.left + 2, m_BoundingRect.top + 3, m_BoundingRect.right - 3, m_BoundingRect.top + 3);
		game_engine::instance()->DrawLine(m_BoundingRect.left + 3, m_BoundingRect.top + 3, m_BoundingRect.left + 3, m_BoundingRect.bottom - 3);

		//++r.left; ++r.top; --r.right; --r.bottom;
		//if (!m_bArmed) GameEngine::GetSingleton()->SetColor(COLOR(240, 240, 240));
		//else GameEngine::GetSingleton()->SetColor(COLOR(180, 180, 180));
		////GameEngine::GetSingleton()->SetColor(COLOR(240, 240, 240));
		//GameEngine::GetSingleton()->FillRect(r.left, r.top, r.right, r.bottom);
		game_engine::instance()->set_color(m_ForeColor);
		game_engine::instance()->DrawString(m_Text, m_BoundingRect.left + 2, m_BoundingRect.top + 2, m_BoundingRect.right + 2, m_BoundingRect.bottom + 2);
	}
}

void Button::DrawImageButton()
{
	if (m_bArmed)
	{
		game_engine::instance()->DrawBitmap(m_BmpPressedPtr, m_BoundingRect.left, m_BoundingRect.top);
	}
	else
	{
		game_engine::instance()->DrawBitmap(m_BmpReleasedPtr, m_BoundingRect.left, m_BoundingRect.top);
	}
}

void Button::Tick(double deltaTime)
{
	if (!m_bEnabled)
	{
		m_bArmed = false;
		return;
	}

	MATRIX3X2 matInverse = (game_engine::instance()->GetWorldMatrix() * game_engine::instance()->GetViewMatrix()).Inverse();

	DOUBLE2 mouseScreenSpace(game_engine::instance()->GetMousePosition().x, game_engine::instance()->GetMousePosition().y);
	DOUBLE2 mouseViewSpace = matInverse.TransformPoint(mouseScreenSpace);

	//RMB in button rect armes the button and paint will draw the pressed button
	if (game_engine::instance()->IsMouseButtonDown(VK_LBUTTON) && PointInRect(m_BoundingRect, mouseViewSpace))
	{
		m_bArmed = true;
	}
	else
	{
		//if mouse button is released while in rect, then pressed is true
		if (m_bArmed && !game_engine::instance()->IsMouseButtonDown(VK_LBUTTON) && PointInRect(m_BoundingRect, mouseViewSpace))
		{
			m_bTriggered = true;
			m_bArmed = false;
		}
		//while armed the RMB is released or outside the rect, then armed is false
		else if (m_bArmed && (!game_engine::instance()->IsMouseButtonDown(VK_LBUTTON) || !PointInRect(m_BoundingRect, mouseViewSpace)))
		{
			m_bArmed = false;
		}
	}
}

bool Button::IsPressed() const
{
	if (this == nullptr) MessageBoxA(NULL, "TextBox::IsPressed() called from a pointer that is a nullptr\nThe MessageBox that will appear after you close this MessageBox is the default error message from visual studio.", "GameEngine says NO", MB_OK);

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
