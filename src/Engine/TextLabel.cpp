#include "engine.pch.h"
#include "TextLabel.h"

#include "graphics/2DRenderContext.h"
#include "Font.h"

TextLabel::TextLabel(std::string const& text)
	: GUIBase()
{
	this->SetText(String(text.c_str()));
}

TextLabel::~TextLabel()
{

}

void TextLabel::Paint(graphics::D2DRenderContext& ctx)
{
	Font* originalFont = ctx.get_font();
	// make sure that the text is left aligned
	m_FontPtr->SetAlignHLeft();
	ctx.set_font(m_FontPtr);

	// Draw forecolor when this is enabled
	ctx.set_color(m_ForeColor);
	// Draw the text
	ctx.draw_string(m_Text, m_BoundingRect.left + 2, m_BoundingRect.top + 2, m_BoundingRect.right - 2, m_BoundingRect.bottom - 2);

	//restore font
	ctx.set_font(originalFont);

}
