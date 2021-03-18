#include "engine.pch.h"    // for compiler

#include "Font.h"
#include "TextBox.h"
#include "GameEngine.h"

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
	if (GameEngine::instance()->is_mouse_button_down(VK_LBUTTON))
	{

		float2 mouseScreenSpace(GameEngine::instance()->get_mouse_pos_in_viewport().x, GameEngine::instance()->get_mouse_pos_in_viewport().y);
		float2 mouseViewSpace = mouseScreenSpace;
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

void TextBox::Paint(graphics::D2DRenderContext& ctx)
{
	if (m_BoundingRect.bottom - m_BoundingRect.top <= 0 ||
		m_BoundingRect.right - m_BoundingRect.left <= 0)
	{
		MessageBoxA(NULL, "Impossible to draw the TextBox, it has no valid bounds!", "GameEngine says NO", MB_OK);
		exit(-1);
	}

	//Store font
	Font *originalFont = ctx.get_font();
	// make sure that the text is left aligned
	m_FontPtr->SetAlignHLeft();
	// working copy of the bounds
	RECT r = m_BoundingRect;

	//border color depends on armed state: filling is faster than line drawing
	if (!m_bArmed)
	{
		ctx.set_color(COLOR(201, 201, 201));
		ctx.fill_rect(r.left, r.top, r.right, r.bottom);
	}
	else
	{
		ctx.set_color(COLOR(101, 101, 101));
		ctx.fill_rect(r.left, r.top, r.right, r.bottom);
	}

	// fill interior
	++r.left; ++r.top; --r.right; --r.bottom;
	ctx.set_color(m_BackColor);
	ctx.fill_rect(r.left, r.top, r.right, r.bottom);

	ctx.set_font(m_FontPtr);

	// Draw forecolor when this is enabled
	if (m_bEnabled)ctx.set_color(m_ForeColor);

	// GRAY when disabled
	else ctx.set_color(COLOR(127, 127, 127));

	// Draw the text
	ctx.draw_string(m_Text, m_BoundingRect.left + 2, m_BoundingRect.top + 2, m_BoundingRect.right - 2, m_BoundingRect.bottom - 2);

	// draw caret
	if (m_bArmed) DrawCaret(ctx);

	//restore font
	ctx.set_font(originalFont);
}

void TextBox::DrawCaret(graphics::D2DRenderContext& ctx)
{
	// only if blinking state is "on"
	if (m_bCaretBlinkState)
	{
		// create a text layout object to retrieve info about the layout
		IDWriteTextLayout *textLayoutPtr;
		GameEngine::instance()->GetDWriteFactory()->CreateTextLayout(m_Text.C_str(), m_Text.Length(), m_FontPtr->GetTextFormat(),
			(FLOAT)(m_BoundingRect.right - m_BoundingRect.left),
			(FLOAT)(m_BoundingRect.bottom - m_BoundingRect.top),
			&textLayoutPtr);

		// use the text layout object to retrieve info about the caret coordinates
		DWRITE_HIT_TEST_METRICS hitTestMetrics = {};
		FLOAT caretX = 0, caretY = 0;
		textLayoutPtr->HitTestTextPosition(m_Text.Length() - 1, true, &caretX, &caretY, &hitTestMetrics);

		// draw the caret
		ctx.draw_line(
			float2(m_BoundingRect.left + 2 + caretX, m_BoundingRect.top + caretY),
			float2(m_BoundingRect.left + 2 + caretX, m_BoundingRect.top + caretY + hitTestMetrics.height)
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
