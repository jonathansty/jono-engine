#pragma once


struct IDWriteTextFormat;

class Font
{
public:
	//! Provide a String containing a type name, 
	//! and a number representing the desired size of the font
	//! Example m_MyFont = new Font(String("Consolas"), 96);
	Font(const string& fontNameRef, float size);

	// Not intended to be used by students
	explicit Font(IDWriteTextFormat *textFormatPtr);

	virtual ~Font();

	// C++11 make the class non-copyable
	Font(const Font&) = delete;
	Font& operator=(const Font&) = delete;

	//-------------------------------------------------
	// Methods							
	//-------------------------------------------------
	// Not intended to be used by students
	IDWriteTextFormat*	GetTextFormat() const;

	//! Horizontal left align 
	void	SetAlignHLeft();

	//! Horizontal center align 
	void	SetAlignHCenter();

	//! Horizontal right align 
	void	SetAlignHRight();

	//! Vertical top align
	void	SetAlignVTop();

	//! Vertical center align
	void	SetAlignVCenter();

	//! Vertical bottom allign
	void	SetAlignVBottom();

private:
	//!---------------------------
	//! Private methods
	//!---------------------------
	// Not intended to be used by students
	void LoadTextFormat(const wchar_t* fontName, float size);

	//-------------------------------------------------
	// Datamembers								
	//-------------------------------------------------
	IDWriteTextFormat* m_TextFormatPtr;

};