#include "stdafx.h"    // for compiler

#include "TextBox.h"

//-----------------------------------------------------------------
// TextBox methods
//-----------------------------------------------------------------
TextBox::TextBox() :
	GUIBase(),
	m_bArmed(false),
	m_BlinkFrequency(2),
	m_AccumulatedTime(0),
	m_bCaretBlinkState(true)
{
	m_DefaultBackColor = m_BackColor = COLOR(240, 240, 240);
}

TextBox::TextBox(const String& text) :
	GUIBase(text),
	m_bArmed(false),
	m_BlinkFrequency(2),
	m_AccumulatedTime(0),
	m_bCaretBlinkState(true)
{
	m_DefaultBackColor = m_BackColor = COLOR(240, 240, 240);
}

//TextBox::TextBox(const string& text) :
//	GUIBase(text),
//	m_bArmed(false),
//	m_BlinkFrequency(2),
//	m_AccumulatedTime(0),
//	m_bCaretBlinkState(true)
//{
//	m_DefaultBackColor = m_BackColor = COLOR(240, 240, 240);
//}

TextBox::~TextBox()
{
}

//---------------------------
// Methods - Member functions
//---------------------------

//overriding inherited
void TextBox::SetBounds(int left, int top, int width, int height)
{
	if (this == nullptr) MessageBoxA(NULL, "TextBox::SetBounds() called from a pointer that is a nullptr\nThe MessageBox that will appear after you close this MessageBox is the default error message from visual studio.", "GameEngine says NO", MB_OK);

	m_BoundingRect = { left, top, left + width, top + height };
	m_ClientRect = { left + m_ClientInset, top + m_ClientInset, left + width - m_ClientInset * 2, top + height - m_ClientInset * 2 };
	LimitTextLengthToClientArea();
}

void TextBox::Tick(double deltaTime)
{
	if (!m_bEnabled)
	{
		m_bArmed = false;
		return;
	}

	//RMB in button rect armes the button and paint will draw the pressed button
	if (game_engine::instance()->is_mouse_button_down(VK_LBUTTON))
	{
		MATRIX3X2 matInverse = (game_engine::instance()->get_world_matrix() * game_engine::instance()->get_view_matrix()).Inverse();

		DOUBLE2 mouseScreenSpace(game_engine::instance()->get_mouse_pos_in_viewport().x, game_engine::instance()->get_mouse_pos_in_viewport().y);
		DOUBLE2 mouseViewSpace = matInverse.TransformPoint(mouseScreenSpace);
		if (PointInRect(m_BoundingRect, mouseViewSpace))
		{
			m_bArmed = true;
		}
		else m_bArmed = false;
	}

	// for blinking caret
	m_AccumulatedTime += deltaTime;
	if (m_AccumulatedTime > (1 / m_BlinkFrequency))
	{
		m_AccumulatedTime = 0;
		m_bCaretBlinkState = !m_bCaretBlinkState;
	}
}

void TextBox::Paint()
{
	if (m_BoundingRect.bottom - m_BoundingRect.top <= 0 ||
		m_BoundingRect.right - m_BoundingRect.left <= 0)
	{
		MessageBoxA(NULL, "Impossible to draw the TextBox, it has no valid bounds!", "GameEngine says NO", MB_OK);
		exit(-1);
	}

	//Store font
	Font *originalFont = game_engine::instance()->get_font();
	// make sure that the text is left aligned
	m_FontPtr->SetAlignHLeft();
	// working copy of the bounds
	RECT r = m_BoundingRect;

	//border color depends on armed state: filling is faster than line drawing
	if (!m_bArmed)
	{
		game_engine::instance()->set_color(COLOR(201, 201, 201));
		game_engine::instance()->FillRect(r.left, r.top, r.right, r.bottom);
	}
	else
	{
		game_engine::instance()->set_color(COLOR(101, 101, 101));
		game_engine::instance()->FillRect(r.left, r.top, r.right, r.bottom);
	}

	// fill interior
	++r.left; ++r.top; --r.right; --r.bottom;
	game_engine::instance()->set_color(m_BackColor);
	game_engine::instance()->FillRect(r.left, r.top, r.right, r.bottom);

	game_engine::instance()->set_font(m_FontPtr);

	// Draw forecolor when this is enabled
	if (m_bEnabled)game_engine::instance()->set_color(m_ForeColor);

	// GRAY when disabled
	else game_engine::instance()->set_color(COLOR(127, 127, 127));

	// Draw the text
	game_engine::instance()->DrawString(m_Text, m_BoundingRect.left + 2, m_BoundingRect.top + 2, m_BoundingRect.right - 2, m_BoundingRect.bottom - 2);

	// draw caret
	if (m_bArmed) DrawCaret();

	//restore font
	game_engine::instance()->set_font(originalFont);
}

void TextBox::DrawCaret()
{
	// only if blinking state is "on"
	if (m_bCaretBlinkState)
	{
		// create a text layout object to retrieve info about the layout
		IDWriteTextLayout *textLayoutPtr;
		game_engine::instance()->GetDWriteFactory()->CreateTextLayout(m_Text.C_str(), m_Text.Length(), m_FontPtr->GetTextFormat(),
			(FLOAT)(m_BoundingRect.right - m_BoundingRect.left),
			(FLOAT)(m_BoundingRect.bottom - m_BoundingRect.top),
			&textLayoutPtr);

		// use the text layout object to retrieve info about the caret coordinates
		DWRITE_HIT_TEST_METRICS hitTestMetrics = {};
		FLOAT caretX = 0, caretY = 0;
		textLayoutPtr->HitTestTextPosition(m_Text.Length() - 1, true, &caretX, &caretY, &hitTestMetrics);

		// draw the caret
		game_engine::instance()->DrawLine(
			DOUBLE2(m_BoundingRect.left + 2 + caretX, m_BoundingRect.top + caretY),
			DOUBLE2(m_BoundingRect.left + 2 + caretX, m_BoundingRect.top + caretY + hitTestMetrics.height)
			);
		textLayoutPtr->Release();
	}
}

void TextBox::HandleKeyInput(TCHAR c)
{
	if (m_bArmed)
	{
		switch (c)
		{
		case VK_BACK:
			if (m_OriginalText.Length() > 0) m_OriginalText = m_OriginalText.SubStr(0, m_OriginalText.Length() - 1);
			break;
		case VK_RETURN:
			m_bTriggered = true;
			break;
		default:
			m_OriginalText += String(c);
		}
		LimitTextLengthToClientArea();
	}
}

bool TextBox::IsEntered() const
{
	if (this == nullptr) MessageBoxA(NULL, "TextBox::IsEntered() called from a pointer that is a nullptr\nThe MessageBox that will appear after you close this MessageBox is the default error message from visual studio.", "GameEngine says NO", MB_OK);
	return m_bTriggered;
}

void TextBox::ConsumeEvent()
{
	m_bTriggered = false;
}
bool TextBox::IsFocus()
{
    return m_bArmed;
}
