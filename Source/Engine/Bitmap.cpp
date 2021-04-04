#include "engine.pch.h"    // for compiler

#include "Bitmap.h"
#include "GameEngine.h"

//---------------------------
// Bitmap methods
//---------------------------
// Load an Bitmap using a filename
Bitmap::Bitmap() : m_BitmapPtr(nullptr), m_ConvertorPtr(nullptr), m_Opacity(1.0), m_ResourceID(0)
{
}

Bitmap::~Bitmap()
{
}

unique_ptr<Bitmap> Bitmap::load(string const& filename) {
	auto bmp = make_unique<Bitmap>();

	string path = GameEngine::instance()->get_io()->resolve_path(filename);
	bmp->m_FileName = path;
	//IWICFormatConverter *convertorPtr=nullptr;
	ID2D1RenderTarget* renderTargetPtr = GameEngine::instance()->GetHwndRenderTarget();
	IWICImagingFactory* iWICFactoryPtr = GameEngine::instance()->GetWICImagingFactory();

	HRESULT hr = bmp->LoadBitmapFromFile(renderTargetPtr, iWICFactoryPtr, bmp->m_FileName, 0, 0, &bmp->m_ConvertorPtr);
	if (SUCCEEDED(hr)) {
		// Create a Direct2D bitmap from the WIC bitmap.
		hr = renderTargetPtr->CreateBitmapFromWicBitmap(bmp->m_ConvertorPtr.Get(), bmp->m_BitmapPtr.GetAddressOf());
	}

	if (FAILED(hr)) {
		FAILMSG("IMAGE LOADING ERROR File {}", filename);
	}

	return bmp;
}

HRESULT Bitmap::LoadBitmapFromFile(ID2D1RenderTarget *renderTargetPtr, IWICImagingFactory *wICFactoryPtr, const string& uriRef, UINT destinationWidth, UINT destinationHeight, IWICFormatConverter **FormatConverterPtr)
{
	IWICBitmapDecoder*		decoderPtr = nullptr;
	IWICBitmapFrameDecode*	sourcePtr = nullptr;
	IWICBitmapScaler*		scalerPtr = nullptr;
	//CreateDecoderFromFilename->first param HAS TO BE LPCWSTR
	string tUri(uriRef.c_str());
	std::wstring wUri(tUri.begin(), tUri.end());
	HRESULT hr = wICFactoryPtr->CreateDecoderFromFilename(wUri.c_str(), NULL, GENERIC_READ, WICDecodeMetadataCacheOnLoad, &decoderPtr);

	if (SUCCEEDED(hr))
	{
		// Create the initial frame.
		hr = decoderPtr->GetFrame(0, &sourcePtr);
	}
	if (SUCCEEDED(hr))
	{
		// Convert the image format to 32bppPBGRA
		// (DXGI_FORMAT_B8G8R8A8_UNORM + D2D1_ALPHA_MODE_PREMULTIPLIED).
		hr = wICFactoryPtr->CreateFormatConverter(FormatConverterPtr);
	}

	if (SUCCEEDED(hr))
	{
		// If a new width or height was specified, create an
		// IWICBitmapScaler and use it to resize the image.
		if (destinationWidth != 0 || destinationHeight != 0)
		{
			UINT originalWidth, originalHeight;
			hr = sourcePtr->GetSize(&originalWidth, &originalHeight);
			if (SUCCEEDED(hr))
			{
				if (destinationWidth == 0)
				{
					FLOAT scalar = (FLOAT)destinationHeight / (FLOAT)originalHeight;
					destinationWidth = (UINT)(scalar * (FLOAT)originalWidth);
				}
				else if (destinationHeight == 0)
				{
					FLOAT scalar = (FLOAT)destinationWidth / (FLOAT)originalWidth;
					destinationHeight = (UINT)(scalar * (FLOAT)originalHeight);
				}

				hr = wICFactoryPtr->CreateBitmapScaler(&scalerPtr);
				if (SUCCEEDED(hr))
				{
					hr = scalerPtr->Initialize(sourcePtr, destinationWidth, destinationHeight, WICBitmapInterpolationModeCubic);
				}
				if (SUCCEEDED(hr))
				{
					hr = (*FormatConverterPtr)->Initialize(scalerPtr, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.f, WICBitmapPaletteTypeMedianCut);
				}
			}
		}
		else // Don't scale the image.
		{
			hr = (*FormatConverterPtr)->Initialize(sourcePtr, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.f, WICBitmapPaletteTypeMedianCut);
		}
	}

	if (decoderPtr != nullptr) decoderPtr->Release();
	if (sourcePtr != nullptr)  sourcePtr->Release();
	if (scalerPtr != nullptr)  scalerPtr->Release();

	return hr;
}

ID2D1Bitmap* Bitmap::GetBitmapPtr() const
{
	return m_BitmapPtr.Get();
}

int Bitmap::get_width() const
{
	return m_BitmapPtr->GetPixelSize().width;
}

int	Bitmap::get_height() const
{
	return m_BitmapPtr->GetPixelSize().height;
}

double Bitmap::GetOpacity() const
{
	return m_Opacity;
}

void Bitmap::SetOpacity(double opacity)
{
	m_Opacity = opacity;
}

void Bitmap::SetTransparencyColor(u32 transparentColor)
{
	u8 r = COLOR_R(transparentColor);
	u8 g = COLOR_G(transparentColor);
	u8 b = COLOR_B(transparentColor);
	COLORREF color = RGB(r, g, b);
	UINT width = 0, height = 0;
	WICPixelFormatGUID* pPixelFormatPtr = nullptr;
	m_ConvertorPtr->GetPixelFormat(pPixelFormatPtr);
	m_ConvertorPtr->GetSize(&width, &height);
	UINT bitmapStride = 4 * width;
	UINT size = width * height * 4;
	unsigned char* pixelsPtr = new unsigned char[size]; // create 32 bit buffer
	m_ConvertorPtr->CopyPixels(NULL, bitmapStride, size, pixelsPtr);

	for (unsigned int count = 0; count < width * height; ++count)
	{
		if (RGB(pixelsPtr[count * 4 + 2], pixelsPtr[count * 4 + 1], pixelsPtr[count * 4]) == color) // if the color of this pixel == transparency color
		{
			((int*)pixelsPtr)[count] = 0; // set all four values to zero, this assumes sizeof(int) == 4 on this system
			// setting values to zero means premultiplying the RGB values to an alpha of 0
		}
	}

	//assign modified pixels to bitmap
	IWICImagingFactory* iWICFactoryPtr = GameEngine::instance()->GetWICImagingFactory();
	IWICBitmap* iWICBitmapPtr = nullptr;
	HRESULT hr = iWICFactoryPtr->CreateBitmapFromMemory(width, height, GUID_WICPixelFormat32bppPBGRA, bitmapStride, size, pixelsPtr, &iWICBitmapPtr);
	delete[] pixelsPtr; //destroy buffer
	if (hr == S_OK)
	{
		ID2D1RenderTarget *renderTargetPtr = GameEngine::instance()->GetHwndRenderTarget();
		if (m_BitmapPtr != nullptr) m_BitmapPtr->Release();
		renderTargetPtr->CreateBitmapFromWicBitmap(iWICBitmapPtr, &m_BitmapPtr);
		iWICBitmapPtr->Release();
	}
}
string Bitmap::GetFileName()
{
    return m_FileName;
}