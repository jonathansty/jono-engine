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
	Font* originalFont = GameEngine::GetSingleton()->GetFont();
	// make sure that the text is left aligned
	m_FontPtr->SetAlignHLeft();
	// working copy of the bounds
	RECT r = m_BoundingRect;

	GameEngine::GetSingleton()->SetFont(m_FontPtr);

	// Draw forecolor when this is enabled
	GameEngine::GetSingleton()->SetColor(m_ForeColor);
	// Draw the text
	GameEngine::GetSingleton()->DrawString(m_Text, m_BoundingRect.left + 2, m_BoundingRect.top + 2, m_BoundingRect.right - 2, m_BoundingRect.bottom - 2);

	//restore font
	GameEngine::GetSingleton()->SetFont(originalFont);

}
