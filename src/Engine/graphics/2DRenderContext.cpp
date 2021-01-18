#include "stdafx.h"

#include "2DRenderContext.h"

namespace graphics {

D2DRenderContext::D2DRenderContext(ID2D1Factory *factory, ID2D1RenderTarget *rt, ID2D1SolidColorBrush *brush, Font* font) :
		_rt(rt), _brush(brush), _factory(factory), _interpolation_mode(D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR), _default_font(font), _font(font), _mat_view(Matrix3x2F::Identity()), _mat_world(Matrix3x2F::Identity()) {
}

bool D2DRenderContext::begin_paint() {
	if (_rt) {
		_rt->BeginDraw();
		_rt->SetTransform(D2D1::Matrix3x2F::Identity());

		// set black as initial brush color
		_brush->SetColor(D2D1::ColorF((FLOAT)(0.0), (FLOAT)(0.0), (FLOAT)(0.0), (FLOAT)(1.0)));
	}
	return true;
}

bool D2DRenderContext::end_paint() {
	HRESULT hr = S_OK;
	hr = _rt->EndDraw();

	if (hr == D2DERR_RECREATE_TARGET) {
		printf(" Direct2D error: RenderTarget lost.\nThe GameEngine terminates the game.\n");
		return false; //app should close or re-initialize
	}
	return true;
}

bool D2DRenderContext::draw_background(COLOR backgroundColor) {
	_rt->Clear(D2D1::ColorF((FLOAT)(backgroundColor.red / 255.0), (FLOAT)(backgroundColor.green / 255.0), (FLOAT)(backgroundColor.blue / 255.0), (FLOAT)(backgroundColor.alpha)));
	return true;
}

bool D2DRenderContext::draw_line(DOUBLE2 p1, DOUBLE2 p2, double strokeWidth /*= 1.0*/) {
	_rt->DrawLine(Point2F((FLOAT)p1.x, (FLOAT)p1.y), Point2F((FLOAT)p2.x, (FLOAT)p2.y), _brush, (FLOAT)strokeWidth);
	return true;
}

bool D2DRenderContext::draw_line(int x1, int y1, int x2, int y2) {
	return draw_line(DOUBLE2(x1, y1), DOUBLE2(x2, y2), 1.0);
}

bool D2DRenderContext::draw_polygon(const std::vector<DOUBLE2> &ptsArr, unsigned int count, bool close /*= true*/, double strokeWidth /*= 1.0*/) {
	if (count < 2) {
		return false;
	}

	for (unsigned int countLoop = 0; countLoop < count - 1; ++countLoop) {
		draw_line(ptsArr[countLoop].x, ptsArr[countLoop].y, ptsArr[countLoop + 1].x, ptsArr[countLoop + 1].y);
	}
	if (close) {
		draw_line(ptsArr[0].x, ptsArr[0].y, ptsArr[count - 1].x, ptsArr[count - 1].y);
	}

	return true;
}

bool D2DRenderContext::draw_polygon(const std::vector<POINT> &ptsArr, unsigned int count, bool close /*= true*/) {
	if (count < 2) {
		return false;
	}

	for (unsigned int countLoop = 0; countLoop < count - 1; ++countLoop) {
		draw_line(ptsArr[countLoop].x, ptsArr[countLoop].y, ptsArr[countLoop + 1].x, ptsArr[countLoop + 1].y);
	}
	if (close) {
		draw_line(ptsArr[0].x, ptsArr[0].y, ptsArr[count - 1].x, ptsArr[count - 1].y);
	}
	return true;
}

bool D2DRenderContext::fill_polygon(const std::vector<DOUBLE2> &ptsArr, unsigned int count) {
	if (count < 2) {
		return false;
	}

	HRESULT hr;

	// Create path geometry
	ID2D1PathGeometry *geometryPtr;
	hr = _factory->CreatePathGeometry(&(geometryPtr));
	if (FAILED(hr)) {
		geometryPtr->Release();
		GameEngine::instance()->message_box(String("Failed to create path geometry"));

		return false;
	}

	// Write to the path geometry using the geometry sink.
	ID2D1GeometrySink *geometrySinkPtr = nullptr;
	hr = geometryPtr->Open(&geometrySinkPtr);
	if (FAILED(hr)) {
		geometrySinkPtr->Release();
		geometryPtr->Release();
		GameEngine::instance()->message_box(String("Failed to open path geometry"));
		return false;
	}

	if (SUCCEEDED(hr)) {
		geometrySinkPtr->BeginFigure(
				D2D1::Point2F((FLOAT)ptsArr[0].x, (FLOAT)ptsArr[0].y),
				D2D1_FIGURE_BEGIN_FILLED);

		for (unsigned int i = 0; i < count; ++i) {
			geometrySinkPtr->AddLine(D2D1::Point2F((FLOAT)ptsArr[i].x, (FLOAT)ptsArr[i].y));
		}

		geometrySinkPtr->EndFigure(D2D1_FIGURE_END_CLOSED);

		hr = geometrySinkPtr->Close();
	}

	geometrySinkPtr->Release();

	if (SUCCEEDED(hr)) {
		_rt->FillGeometry(geometryPtr, _brush);
		geometryPtr->Release();
		return true;
	}

	geometryPtr->Release();
	return false;
}

bool D2DRenderContext::fill_polygon(const std::vector<POINT> &ptsArr, unsigned int count) {
	//do not fill an empty polygon
	if (count < 2) {
		return false;
	}

	HRESULT hr;

	// Create path geometry
	ID2D1PathGeometry *geometryPtr;
	hr = _factory->CreatePathGeometry(&geometryPtr);
	if (FAILED(hr)) {
		geometryPtr->Release();
		GameEngine::instance()->message_box(String("Failed to create path geometry"));
		return false;
	}

	// Write to the path geometry using the geometry sink.
	ID2D1GeometrySink *geometrySinkPtr = nullptr;
	hr = geometryPtr->Open(&geometrySinkPtr);
	if (FAILED(hr)) {
		geometrySinkPtr->Release();
		geometryPtr->Release();
		GameEngine::instance()->message_box(String("Failed to open path geometry"));
		return false;
	}
	if (SUCCEEDED(hr)) {
		geometrySinkPtr->BeginFigure(
				D2D1::Point2F((FLOAT)ptsArr[0].x, (FLOAT)ptsArr[0].y),
				D2D1_FIGURE_BEGIN_FILLED);

		for (unsigned int i = 0; i < count; ++i) {
			geometrySinkPtr->AddLine(D2D1::Point2F((FLOAT)ptsArr[i].x, (FLOAT)ptsArr[i].y));
		}

		geometrySinkPtr->EndFigure(D2D1_FIGURE_END_CLOSED);

		hr = geometrySinkPtr->Close();
		geometrySinkPtr->Release();
	}
	if (SUCCEEDED(hr)) {
		_rt->FillGeometry(geometryPtr, _brush);
		geometryPtr->Release();
		return true;
	}

	geometryPtr->Release();
	return false;
}

bool D2DRenderContext::draw_rect(RECT2 rect, double strokeWidth /*= 1*/) {
	if ((rect.right < rect.left) || (rect.bottom < rect.top)) {
		GameEngine::instance()->message_box(String("GameEngine::DrawRect error: invalid dimensions!"));
		return false;
	}

	D2D1_RECT_F d2dRect = D2D1::RectF((FLOAT)rect.left, (FLOAT)rect.top, (FLOAT)rect.right, (FLOAT)rect.bottom);
	_rt->DrawRectangle(d2dRect, _brush, (FLOAT)strokeWidth);
	return true;
}

bool D2DRenderContext::draw_rect(DOUBLE2 topLeft, DOUBLE2 rightbottom, double strokeWidth /*= 1.0*/) {
	RECT2 rect2(topLeft.x, topLeft.y, rightbottom.x, rightbottom.y);
	return draw_rect(rect2, strokeWidth);
}

bool D2DRenderContext::draw_rect(RECT rect) {
	RECT2 rect2(rect.left, rect.top, rect.right, rect.bottom);
	return draw_rect(rect2, 1.0);
}

bool D2DRenderContext::draw_rect(int left, int top, int right, int bottom) {
	RECT2 rect2(left, top, right, bottom);
	return draw_rect(rect2, 1.0);
}

bool D2DRenderContext::fill_rect(RECT2 rect) {
	if ((rect.right < rect.left) || (rect.bottom < rect.top)) {
		GameEngine::instance()->message_box(String("GameEngine::DrawRect error: invalid dimensions!"));
		return false;
	}

	D2D1_RECT_F d2dRect = D2D1::RectF((FLOAT)rect.left, (FLOAT)rect.top, (FLOAT)rect.right, (FLOAT)rect.bottom);
	_rt->FillRectangle(d2dRect, _brush);

	return true;
}

bool D2DRenderContext::fill_rect(DOUBLE2 topLeft, DOUBLE2 rightbottom) {
	RECT2 rect2(topLeft.x, topLeft.y, rightbottom.x, rightbottom.y);
	return fill_rect(rect2);
}

bool D2DRenderContext::fill_rect(RECT rect) {
	RECT2 rect2(rect.left, rect.top, rect.right, rect.bottom);
	return fill_rect(rect2);
}

bool D2DRenderContext::fill_rect(int left, int top, int right, int bottom) {
	RECT2 rect2(left, top, right, bottom);
	return fill_rect(rect2);
}

bool D2DRenderContext::draw_rounded_rect(RECT2 rect, int radiusX, int radiusY, double strokeWidth /*= 1.0*/) {
	D2D1_RECT_F d2dRect = D2D1::RectF((FLOAT)rect.left, (FLOAT)rect.top, (FLOAT)rect.right, (FLOAT)rect.bottom);
	D2D1_ROUNDED_RECT d2dRoundedRect = D2D1::RoundedRect(d2dRect, (FLOAT)radiusX, (FLOAT)radiusY);
	_rt->DrawRoundedRectangle(d2dRoundedRect, _brush, 1.0);
	return true;
}

bool D2DRenderContext::draw_rounded_rect(DOUBLE2 topLeft, DOUBLE2 rightbottom, int radiusX, int radiusY, double strokeWidth /*= 1.0*/) {
	D2D1_RECT_F d2dRect = D2D1::RectF((FLOAT)topLeft.x, (FLOAT)topLeft.y, (FLOAT)(rightbottom.x), (FLOAT)(rightbottom.y));
	D2D1_ROUNDED_RECT d2dRoundedRect = D2D1::RoundedRect(d2dRect, (FLOAT)radiusX, (FLOAT)radiusY);
	_rt->DrawRoundedRectangle(d2dRoundedRect, _brush, (FLOAT)strokeWidth);
	return true;
}

bool D2DRenderContext::draw_rounded_rect(RECT rect, int radiusX, int radiusY) {
	D2D1_RECT_F d2dRect = D2D1::RectF((FLOAT)rect.left, (FLOAT)rect.top, (FLOAT)rect.right, (FLOAT)rect.bottom);
	D2D1_ROUNDED_RECT d2dRoundedRect = D2D1::RoundedRect(d2dRect, (FLOAT)radiusX, (FLOAT)radiusY);
	_rt->DrawRoundedRectangle(d2dRoundedRect, _brush, 1.0);
	return true;
}

bool D2DRenderContext::draw_rounded_rect(int left, int top, int right, int bottom, int radiusX, int radiusY) {
	D2D1_RECT_F d2dRect = D2D1::RectF((FLOAT)left, (FLOAT)top, (FLOAT)(right), (FLOAT)(bottom));
	D2D1_ROUNDED_RECT d2dRoundedRect = D2D1::RoundedRect(d2dRect, (FLOAT)radiusX, (FLOAT)radiusY);
	_rt->DrawRoundedRectangle(d2dRoundedRect, _brush, 1.0);
	return true;
}

bool D2DRenderContext::fill_rounded_rect(RECT2 rect, int radiusX, int radiusY) {
	D2D1_RECT_F d2dRect = D2D1::RectF((FLOAT)rect.left, (FLOAT)rect.top, (FLOAT)rect.right, (FLOAT)rect.bottom);
	D2D1_ROUNDED_RECT d2dRoundedRect = D2D1::RoundedRect(d2dRect, (FLOAT)radiusX, (FLOAT)radiusY);
	_rt->FillRoundedRectangle(d2dRoundedRect, _brush);
	return true;
}

bool D2DRenderContext::fill_rounded_rect(DOUBLE2 topLeft, DOUBLE2 rightbottom, int radiusX, int radiusY) {
	D2D1_RECT_F d2dRect = D2D1::RectF((FLOAT)topLeft.x, (FLOAT)topLeft.y, (FLOAT)(rightbottom.x), (FLOAT)(rightbottom.y));
	D2D1_ROUNDED_RECT d2dRoundedRect = D2D1::RoundedRect(d2dRect, (FLOAT)radiusX, (FLOAT)radiusY);
	_rt->FillRoundedRectangle(d2dRoundedRect, _brush);
	return true;
}

bool D2DRenderContext::fill_rounded_rect(RECT rect, int radiusX, int radiusY) {
	D2D1_RECT_F d2dRect = D2D1::RectF((FLOAT)rect.left, (FLOAT)rect.top, (FLOAT)rect.right, (FLOAT)rect.bottom);
	D2D1_ROUNDED_RECT d2dRoundedRect = D2D1::RoundedRect(d2dRect, (FLOAT)radiusX, (FLOAT)radiusY);
	_rt->FillRoundedRectangle(d2dRoundedRect, _brush);
	return true;
}

bool D2DRenderContext::fill_rounded_rect(int left, int top, int right, int bottom, int radiusX, int radiusY) {
	D2D1_RECT_F d2dRect = D2D1::RectF((FLOAT)left, (FLOAT)top, (FLOAT)(right), (FLOAT)(bottom));
	D2D1_ROUNDED_RECT d2dRoundedRect = D2D1::RoundedRect(d2dRect, (FLOAT)radiusX, (FLOAT)radiusY);
	_rt->FillRoundedRectangle(d2dRoundedRect, _brush);
	return true;
}

bool D2DRenderContext::draw_ellipse(DOUBLE2 centerPt, double radiusX, double radiusY, double strokeWidth /*= 1.0*/) {
	D2D1_ELLIPSE ellipse = D2D1::Ellipse(D2D1::Point2F((FLOAT)centerPt.x, (FLOAT)centerPt.y), (FLOAT)radiusX, (FLOAT)radiusY);
	_rt->DrawEllipse(ellipse, _brush, (FLOAT)strokeWidth);
	return true;
}

bool D2DRenderContext::draw_ellipse(int centerX, int centerY, int radiusX, int radiusY) {
	D2D1_ELLIPSE ellipse = D2D1::Ellipse(D2D1::Point2F((FLOAT)centerX, (FLOAT)centerY), (FLOAT)radiusX, (FLOAT)radiusY);
	_rt->DrawEllipse(ellipse, _brush, 1.0);
	return true;
}

bool D2DRenderContext::fill_ellipse(DOUBLE2 centerPt, double radiusX, double radiusY) {
	D2D1_ELLIPSE ellipse = D2D1::Ellipse(D2D1::Point2F((FLOAT)centerPt.x, (FLOAT)centerPt.y), (FLOAT)radiusX, (FLOAT)radiusY);
	_rt->FillEllipse(ellipse, _brush);
	return true;
}

bool D2DRenderContext::fill_ellipse(int centerX, int centerY, int radiusX, int radiusY) {
	D2D1_ELLIPSE ellipse = D2D1::Ellipse(D2D1::Point2F((FLOAT)centerX, (FLOAT)centerY), (FLOAT)radiusX, (FLOAT)radiusY);
	_rt->FillEllipse(ellipse, _brush);
	return true;
}

bool D2DRenderContext::draw_string(std::string text, DOUBLE2 topLeft, double right /*= -1*/, double bottom /*= -1*/) {
	D2D1_SIZE_F dstSize_f = _rt->GetSize();
	D2D1_DRAW_TEXT_OPTIONS options = D2D1_DRAW_TEXT_OPTIONS_CLIP;
	if (right == -1 || bottom == -1) //ignore the right and bottom edge to enable drawing in entire Level
	{
		options = D2D1_DRAW_TEXT_OPTIONS_NONE;
		right = bottom = FLT_MAX;
	}
	D2D1_RECT_F layoutRect = (RectF)((FLOAT)topLeft.x, (FLOAT)topLeft.y, (FLOAT)(right), (FLOAT)(bottom));

	tstring wText(text.begin(), text.end());
	_rt->DrawText(wText.c_str(), wText.length(), _font->GetTextFormat(), layoutRect, _brush, options);

	return true;
}

bool D2DRenderContext::draw_string(std::string text, int xPos, int yPos, int right /*= -1*/, int bottom /*= -1*/) {
	return draw_string(text, DOUBLE2(xPos, yPos), right, bottom);
}

bool D2DRenderContext::draw_string(const String &textRef, RECT boundingRect) {
	return draw_string(textRef, boundingRect.left, boundingRect.top, boundingRect.right, boundingRect.bottom);
}

bool D2DRenderContext::draw_string(const String &textRef, RECT2 boundingRect) {
	return draw_string(textRef, (int)boundingRect.left, (int)boundingRect.top, (int)boundingRect.right, (int)boundingRect.bottom);
}

bool D2DRenderContext::draw_string(const String &textRef, DOUBLE2 topLeft, double right /*= -1*/, double bottom /*= -1*/) {
	tstring stext(textRef.C_str(), textRef.C_str() + textRef.Length());

	D2D1_DRAW_TEXT_OPTIONS options = D2D1_DRAW_TEXT_OPTIONS_CLIP;
	if (right == -1 || bottom == -1) //ignore the right and bottom edge to enable drawing in entire Level
	{
		options = D2D1_DRAW_TEXT_OPTIONS_NONE;
		right = bottom = FLT_MAX;
	}
	D2D1_RECT_F layoutRect = (RectF)((FLOAT)topLeft.x, (FLOAT)topLeft.y, (FLOAT)(right), (FLOAT)(bottom));

	_rt->DrawText(stext.c_str(), UINT32(stext.length()), _font->GetTextFormat(), layoutRect, _brush, options);
	return true;
}

bool D2DRenderContext::draw_string(const String &textRef, int xPos, int yPos, int right /*= -1*/, int bottom /*= -1*/) {
	return draw_string(textRef, DOUBLE2(xPos, yPos), right, bottom);
}

bool D2DRenderContext::draw_bitmap(Bitmap *imagePtr, DOUBLE2 position, RECT2 srcRect) {
	if (imagePtr == nullptr) {
		GameEngine::instance()->message_box(String("DrawBitmap called using a bitmap pointer that is a nullptr\nThe MessageBox that will appear after you close this MessageBox is the default error message from visual studio."));
		return false;
	}
	//The size and position, in device-independent pixels in the bitmap's coordinate space, of the area within the bitmap to draw.
	D2D1_RECT_F srcRect_f;
	srcRect_f.left = (FLOAT)srcRect.left;
	srcRect_f.right = (FLOAT)srcRect.right;
	srcRect_f.top = (FLOAT)srcRect.top;
	srcRect_f.bottom = (FLOAT)srcRect.bottom;

	//http://msdn.microsoft.com/en-us/library/dd371880(v=VS.85).aspx
	//The size and position, in device-independent pixels in the render target's coordinate space,
	//of the area to which the bitmap is drawn. If the rectangle is not well-ordered, nothing is drawn,
	//but the render target does not enter an error state.
	D2D1_RECT_F dstRect_f;
	dstRect_f.left = (FLOAT)position.x;
	dstRect_f.right = dstRect_f.left + (FLOAT)(srcRect.right - srcRect.left);
	dstRect_f.top = (FLOAT)position.y;
	dstRect_f.bottom = dstRect_f.top + (FLOAT)(srcRect.bottom - srcRect.top);

	_rt->DrawBitmap(imagePtr->GetBitmapPtr(), dstRect_f, (FLOAT)imagePtr->GetOpacity(), _interpolation_mode, srcRect_f);

	return true;
}

bool D2DRenderContext::draw_bitmap(Bitmap *imagePtr, DOUBLE2 position) {
	if (imagePtr == nullptr) {
		GameEngine::instance()->message_box(String("DrawBitmap called using a bitmap pointer that is a nullptr\nThe MessageBox that will appear after you close this MessageBox is the default error message from visual studio."));
		return false;
	}

	RECT2 srcRect2(0, 0, imagePtr->GetWidth(), imagePtr->GetHeight());
	return draw_bitmap(imagePtr, position, srcRect2);
}

bool D2DRenderContext::draw_bitmap(Bitmap *imagePtr, int x, int y, RECT srcRect) {
	RECT2 srcRect2(srcRect.left, srcRect.top, srcRect.right, srcRect.bottom);
	return draw_bitmap(imagePtr, DOUBLE2(x, y), srcRect2);
}

bool D2DRenderContext::draw_bitmap(Bitmap *imagePtr, int x, int y) {
	if (imagePtr == nullptr) {
		GameEngine::instance()->message_box(String("DrawBitmap called using a bitmap pointer that is a nullptr\nThe MessageBox that will appear after you close this MessageBox is the default error message from visual studio."));
		return false;
	}

	RECT2 srcRect2(0, 0, imagePtr->GetWidth(), imagePtr->GetHeight());
	return draw_bitmap(imagePtr, DOUBLE2(x, y), srcRect2);
}

bool D2DRenderContext::draw_bitmap(Bitmap *imagePtr, RECT srcRect) {
	RECT2 srcRect2(srcRect.left, srcRect.top, srcRect.right, srcRect.bottom);
	return draw_bitmap(imagePtr, DOUBLE2(0, 0), srcRect2);
}

bool D2DRenderContext::draw_bitmap(Bitmap *imagePtr) {
	if (imagePtr == nullptr) {
		GameEngine::instance()->message_box(String("DrawBitmap called using a bitmap pointer that is a nullptr\nThe MessageBox that will appear after you close this MessageBox is the default error message from visual studio."));
		return false;
	}

	RECT2 srcRect2(0, 0, imagePtr->GetWidth(), imagePtr->GetHeight());
	return draw_bitmap(imagePtr, DOUBLE2(0, 0), srcRect2);
}

void D2DRenderContext::set_color(COLOR color) {
	_brush->SetColor(D2D1::ColorF((FLOAT)(color.red / 255.0), (FLOAT)(color.green / 255.0), (FLOAT)(color.blue / 255.0), (FLOAT)(color.alpha / 255.0)));
}

COLOR D2DRenderContext::get_color() const {
	D2D1_COLOR_F dColor = _brush->GetColor();
	return COLOR((unsigned char)(dColor.r * 255), (unsigned char)(dColor.g * 255), (unsigned char)(dColor.b * 255), (unsigned char)(dColor.a * 255));
}

void D2DRenderContext::set_world_matrix(const MATRIX3X2 &mat) {
	_mat_world = mat;
	D2D1::Matrix3x2F matDirect2D = (_mat_world * _mat_view).ToMatrix3x2F();
	_rt->SetTransform(matDirect2D);
}
MATRIX3X2 D2DRenderContext::get_world_matrix() const {
	return _mat_world;
}
void D2DRenderContext::set_view_matrix(const MATRIX3X2 &mat) {
	_mat_view = mat;
	D2D1::Matrix3x2F matDirect2D = (_mat_world * _mat_view).ToMatrix3x2F();
	_rt->SetTransform(matDirect2D);
}

MATRIX3X2 D2DRenderContext::get_view_matrix() const {
	return _mat_view;
}

void D2DRenderContext::set_bitmap_interpolation_mode(bitmap_interpolation_mode mode) {
	switch (mode) {
		case bitmap_interpolation_mode::linear:
			_interpolation_mode = D2D1_BITMAP_INTERPOLATION_MODE_LINEAR;
			break;
		case bitmap_interpolation_mode::nearest_neighbor:
			_interpolation_mode = D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR;
			break;
		default:
			assert("Case not supported");
			break;
	}
}

} // namespace graphics