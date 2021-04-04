#pragma once

struct ID2D1Bitmap;

class Bitmap
{
public:
	Bitmap();
	virtual ~Bitmap();

	static unique_ptr<Bitmap> load(string const& filename);


	// C++11 make the class non-copyable
	Bitmap(const Bitmap&) = delete;
	Bitmap& operator=(const Bitmap&) = delete;

	//-------------------------------------------------
	// Methods							
	//-------------------------------------------------
	// Internal use only. Do not use this method unless you really know what you are doing
	ID2D1Bitmap*	GetBitmapPtr() const;

	//! Returns the width of this image
	int	get_width() const;

	//! Returns the height of this image
	int	get_height() const;

	//! Returns the opacity of this image
	double GetOpacity() const;

	//! The opacity level of an image has a range from 0 to 1
	//! The GameEngine uses the opacity level to blend the image with the background
	void SetOpacity(double opacity);

	//! Scans all pixels and turns the pixels with the given color transparent.
	//! Be carefull!! this is a very expensive operation. Use it in the GameStart only.
	//! alpha is ignored
	void SetTransparencyColor(u32 transparentColor);

    //! Gets the filename of the bitmap.
    string GetFileName();
private:
	static HRESULT LoadBitmapFromFile(ID2D1RenderTarget* renderTargetPtr, IWICImagingFactory* wICFactoryPtr, const string& uriRef, UINT destinationWidth, UINT destinationHeight, IWICFormatConverter** formatConvertorPtrPtr);

	//-------------------------------------------------
	// Datamembers								
	//-------------------------------------------------
	ComPtr<ID2D1Bitmap>			m_BitmapPtr;
	ComPtr<IWICFormatConverter>	m_ConvertorPtr;
	double					m_Opacity;//range: between 0 and 1
	string					m_FileName;
	int						m_ResourceID;
};
