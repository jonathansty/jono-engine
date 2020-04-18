#include "stdafx.h"
#include "TextLabel.h"

TextLabel::TextLabel(std::string const& text)
	: GUIBase()
{
	this->SetText(String(text.c_str()));
}

TextLabel::~TextLabel()
{

}

void TextLabel::Paint()
{
	Font* originalFont = game_engine::instance()->get_font();
	// make sure that the text is left aligned
	m_FontPtr->SetAlignHLeft();
	// working copy of the bounds
	RECT r = m_BoundingRect;

	game_engine::instance()->set_font(m_FontPtr);

	// Draw forecolor when this is enabled
	game_engine::instance()->set_color(m_ForeColor);
	// Draw the text
	game_engine::instance()->DrawString(m_Text, m_BoundingRect.left + 2, m_BoundingRect.top + 2, m_BoundingRect.right - 2, m_BoundingRect.bottom - 2);

	//restore font
	game_engine::instance()->set_font(originalFont);

}
