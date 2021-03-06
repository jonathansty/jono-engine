#include "engine.stdafx.h"    // for compiler
#include "GameEngine.h"
#include "GUIBase.h"
#include "Font.h"

GUIBase::GUIBase() :
	m_FontPtr(nullptr),
	m_bTriggered(false),
	m_bEnabled(true)
{
	Initialize();
}

GUIBase::GUIBase(const String& text) :
	m_FontPtr(nullptr),
	m_bTriggered(false),
	m_bEnabled(true)
{
	m_OriginalText = text;
	Initialize();
}

//GUIBase::GUIBase(const string& text) :
//m_FontPtr(nullptr),
//m_bTriggered(false),
//m_bEnabled(true)
//{
//	m_Text = String(text.c_str());
//	Initialize();
//}

void GUIBase::Initialize()
{
	m_DefaultForeColor = COLOR(0, 0, 0);
	m_DefaultBackColor = COLOR(240, 240, 240);
	m_ForeColor = COLOR(0, 0, 0);
	m_BackColor = COLOR(240, 240, 240);
	m_ClientRect = m_BoundingRect = {};
	m_FontPtr = new Font(String("Consolas"), 14);
	m_FontPtr->SetAlignVCenter();
	m_FontPtr->SetAlignHCenter();
	GameEngine::instance()->register_gui(this);
}

GUIBase::~GUIBase()
{
	GameEngine::instance()->unregister_gui(this);
	delete m_FontPtr;
}

//---------------------------
// Methods - Member functions
//---------------------------

// Sets a new font for this GUI element. 
// Params: String type: the font typetype
//         int height: the font height in pixels
// Example: myPtr->SetFont("Consolas", 14);
void GUIBase::SetFont(const String& typeRef, int height)
{
	delete m_FontPtr;
	m_FontPtr = new Font(typeRef, (float)height);
	m_FontPtr->SetAlignVCenter();
	m_FontPtr->SetAlignHCenter();
	LimitTextLengthToClientArea();
}

void GUIBase::SetText(const String& text)
{

	m_OriginalText = text;
	LimitTextLengthToClientArea();
}

String GUIBase::GetText() const
{

	return m_OriginalText;
}

void GUIBase::SetBounds(int left, int top, int width, int height)
{
	m_BoundingRect = { left, top, left + width, top + height };
	m_ClientRect = m_BoundingRect;
	LimitTextLengthToClientArea();
}

bool GUIBase::PointInRect(RECT r, POINT pt) const
{
	if (pt.x<r.left || pt.x>r.right || pt.y<r.top || pt.y>r.bottom)return false;
	return true;
	//OR
	//if (pt.x>r.left && pt.x<r.right && pt.y>r.top && pt.y<r.bottom)return true;
	//return false;
}

bool GUIBase::PointInRect(RECT r, float2 pt) const
{
	if (pt.x<r.left || pt.x>r.right || pt.y<r.top || pt.y>r.bottom)return false;
	return true;
	//OR
	//if (pt.x>r.left && pt.x<r.right && pt.y>r.top && pt.y<r.bottom)return true;
	//return false;
}

void GUIBase::SetEnabled(bool bEnable)
{
	m_bEnabled = bEnable;
}

void GUIBase::SetBackColor(COLOR backColor)
{
	m_BackColor = backColor;
}

void GUIBase::SetForeColor(COLOR foreColor)
{
	m_ForeColor = foreColor;
}

void GUIBase::SetDefaultBackColor()
{
	m_BackColor = m_DefaultBackColor;
}

void GUIBase::SetDefaultForeColor()
{
	m_ForeColor = m_DefaultForeColor;
}

void GUIBase::LimitTextLengthToClientArea()
{
	// check that text fits in text box: 1 line and no spaces causing caret to float ouside bounds

	//first copy original back to m_Text
	m_Text = m_OriginalText;
	// create a text layout object to retrieve info about the layout
	IDWriteTextLayout *textLayoutPtr = nullptr;
	DWRITE_TEXT_METRICS textMetrix = {};
	bool bRepeat = false;
	do
	{
		GameEngine::instance()->GetDWriteFactory()->CreateTextLayout(m_Text.C_str(), m_Text.Length(), m_FontPtr->GetTextFormat(),
			(FLOAT)(m_ClientRect.right - m_ClientRect.left),
			(FLOAT)(m_ClientRect.bottom - m_ClientRect.top),
			&textLayoutPtr);

		textLayoutPtr->GetMetrics(&textMetrix);
		textLayoutPtr->Release();
		bRepeat = false;
		if (textMetrix.lineCount > 1 ||
			textMetrix.widthIncludingTrailingWhitespace > (m_ClientRect.right - m_ClientRect.left) /*||
																								   ((textMetrix.widthIncludingTrailingWhitespace > 0) && (textMetrix.height > (m_ClientRect.bottom - m_ClientRect.top)))*/
																								   )
		{
			// OutputDebugStringA("Text too big for GUI\n");
			// remove last character
			if (m_Text.Length() > 0) m_Text = m_Text.SubStr(0, m_Text.Length() - 1);
			bRepeat = true;
		}
	} while (bRepeat);
}