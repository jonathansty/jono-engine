#pragma once

#if FEATURE_D2D


struct ID2D1Factory;
struct ID2D1RenderTarget;
struct ID2D1SolidColorBrush;
class Font;
class Bitmap;

using hlslpp::float2;

namespace Graphics {

// Bitmap interpolation mode enum
enum class bitmap_interpolation_mode {
	linear,
	nearest_neighbor
};


// Lightweight 2D render context that can be used to draw to a D2D target.
class D2DRenderContext {

public:
	D2DRenderContext(ID2D1Factory* factory,ID2D1RenderTarget *rt, ID2D1SolidColorBrush *brush, Font* font);
	~D2DRenderContext() = default;

	bool begin_paint();
	bool end_paint();

	bool draw_background(u32 backgroundColor);
	//! Draws a line from p1 to p2 using the strokewidth
	bool draw_line(float2 p1, float2 p2, double strokeWidth = 1.0);
	//! Draws a line from the coordinate defined by x1 and y1 to the coordinate define by x2 and y2
	bool draw_line(int x1, int y1, int x2, int y2);

	//! Draws a polygon defined by the coordinates in ptsArr
	//! count is the number of points that must be drawn
	//! If close is true then it will connect the start and end coordinate
	bool draw_polygon(const std::vector<float2> &ptsArr, unsigned int count, bool close = true, double strokeWidth = 1.0);

	//! Draws a polygon defined by the coordinates in ptsArr
	//! count is the number of points that must be drawn
	//! If close is true then it will connect the start and end coordinate
	bool draw_polygon(const std::vector<POINT> &ptsArr, unsigned int count, bool close = true);

	//! Fills the interior of a polygon defined by the coordinates in ptsArr
	//! count is the number of points that must be drawn
	//! If close is true then it will connect the start and end coordinate
	bool fill_polygon(const std::vector<float2> &ptsArr, unsigned int count);

	//! Fills the interior of a polygon defined by the coordinates in ptsArr
	//! count is the number of points that must be drawn
	//! If close is true then it will connect the start and end coordinate
	bool fill_polygon(const std::vector<POINT> &ptsArr, unsigned int count);

	using Rect = D2D1_RECT_F;
	//! Draws a rectangle defined by a RECT2 struct
	bool draw_rect(Rect rect, double strokeWidth = 1);
	//! Draws a rectangle defined by two coordinates: topleft and rightbottom
	bool draw_rect(float2 topLeft, float2 rightbottom, double strokeWidth = 1.0);
	//! Draws a rectangle defined by a RECT struct
	bool draw_rect(RECT rect);
	//! Draws a rectangle defined by 4 numbers representing the left side, top side, the right side and the bottom side
	bool draw_rect(int left, int top, int right, int bottom);

	//! Fills the interior of a rectangle defined by a RECT2 struct
	bool fill_rect(Rect rect);
	//! Fills the interior of a rectangle defined by two coordinates: topleft and rightbottom
	bool fill_rect(float2 topLeft, float2 rightbottom);
	//! Fills the interior of a rectangle defined by a RECT struct
	bool fill_rect(RECT rect);
	//! Fills the interior of a rectangle defined by 4 numbers representing the left side, top side, the right side and the bottom side
	bool fill_rect(int left, int top, int right, int bottom);

	//! Draws a rounded rectangle defined by a RECT2 struct,
	//!   the x-radius for the quarter ellipse that is drawn to replace every corner of the rectangle.
	//!   the y-radius for the quarter ellipse that is drawn to replace every corner of the rectangle.
	bool draw_rounded_rect(Rect rect, int radiusX, int radiusY, double strokeWidth = 1.0);

	//! Draws a rounded rectangle defined by two coordinates: topleft and rightbottom,
	//!   the x-radius for the quarter ellipse that is drawn to replace every corner of the rectangle.
	//!   the y-radius for the quarter ellipse that is drawn to replace every corner of the rectangle.
	bool draw_rounded_rect(float2 topLeft, float2 rightbottom, int radiusX, int radiusY, double strokeWidth = 1.0);

	//! Draws a rounded rectangle defined by a RECT struct
	//!   the x-radius for the quarter ellipse that is drawn to replace every corner of the rectangle.
	//!   the y-radius for the quarter ellipse that is drawn to replace every corner of the rectangle.
	bool draw_rounded_rect(RECT rect, int radiusX, int radiusY);

	//! Draws a rounded rectangle defined by 4 numbers representing the left side, top side, the right side and the bottom side
	//!   the x-radius for the quarter ellipse that is drawn to replace every corner of the rectangle.
	//!   the y-radius for the quarter ellipse that is drawn to replace every corner of the rectangle.
	bool draw_rounded_rect(int left, int top, int right, int bottom, int radiusX, int radiusY);

	//! Fills the interior of a rounded rectangle defined by a RECT2 struct,
	//!   the x-radius for the quarter ellipse that is drawn to replace every corner of the rectangle.
	//!   the y-radius for the quarter ellipse that is drawn to replace every corner of the rectangle.
	bool fill_rounded_rect(Rect rect, int radiusX, int radiusY);

	//! Fills the interior of a rounded rectangle defined by two coordinates: topleft and rightbottom,
	//!   the x-radius for the quarter ellipse that is drawn to replace every corner of the rectangle.
	//!   the y-radius for the quarter ellipse that is drawn to replace every corner of the rectangle.
	bool fill_rounded_rect(float2 topLeft, float2 rightbottom, int radiusX, int radiusY);

	//! Fills the interior of a rounded rectangle defined by a RECT struct
	//!   the x-radius for the quarter ellipse that is drawn to replace every corner of the rectangle.
	//!   the y-radius for the quarter ellipse that is drawn to replace every corner of the rectangle.
	bool fill_rounded_rect(RECT rect, int radiusX, int radiusY);

	//! Fills the interior of a rounded rectangle defined by 4 numbers representing the left side, top side, the right side and the bottom side
	//!   the x-radius for the quarter ellipse that is drawn to replace every corner of the rectangle.
	//!   the y-radius for the quarter ellipse that is drawn to replace every corner of the rectangle.
	bool fill_rounded_rect(int left, int top, int right, int bottom, int radiusX, int radiusY);

	//! Draws the outline of the specified ellipse using the specified position and radius
	//! strokeWidth: The width of the stroke, in device-independent pixels. The value must be greater than or equal to 0.0f.
	//! If this parameter isn't specified, it defaults to 1.0f. The stroke is centered on the line.
	bool draw_ellipse(float2 centerPt, double radiusX, double radiusY, double strokeWidth = 1.0);

	//! Draws the outline of the specified ellipse using the specified position and radius
	bool draw_ellipse(int centerX, int centerY, int radiusX, int radiusY);

	//! Paints the interior of the specified ellipse using the specified position and radius
	bool fill_ellipse(float2 centerPt, double radiusX, double radiusY);

	//! Paints the interior of the specified ellipse using the specified position and radius
	bool fill_ellipse(int centerX, int centerY, int radiusX, int radiusY);

	//! Draws text in the specified rectangle
	bool draw_string(const string &textRef, RECT boundingRect);

	//! Draws text in the specified rectangle
	bool draw_string(const string &textRef, Rect boundingRect);

	//! Draws text in the specified rectangle the topleft corner of the rectange is defined by the param topLeft
	//! The params right and bottom are optional, if left out they are set to the max value of an float type
	bool draw_string(const string &textRef, float2 topLeft, double right = -1, double bottom = -1);

	//! Draws text in the specified rectangle; the topleft corner of the rectange is defined by the params xPos and yPos
	//! The params right and bottom are optional, if left out they are set to the max value of an float type
	bool draw_string(const string &textRef, int xPos, int yPos, int right = -1, int bottom = -1);

	//! Draws an image on the position
	//! srcRect: defines the cliprect on the source image. Allows to draw a part of an image
	bool draw_bitmap(Bitmap *imagePtr, float2 position, Rect srcRect);

	//! Draws an image on the position
	bool draw_bitmap(Bitmap *imagePtr, float2 position);

	//! Draws an image on the position defined by x and y
	//! srcRect: defines the cliprect on the source image. Allows to draw a part of an image
	bool draw_bitmap(Bitmap *imagePtr, int x, int y, RECT srcRect);

	//! Draws an image on the position defined by x and y
	bool draw_bitmap(Bitmap *imagePtr, int x, int y);

	//! Draws an image on position x:0, and y:0. Assuming that matrices are used to define the position.
	//! srcRect: defines the cliprect on the source image. Allows to draw a part of an image
	bool draw_bitmap(Bitmap *imagePtr, RECT srcRect);

	//! Draws an image on position x:0, and y:0. Assuming that matrices are used to define the position.
	bool draw_bitmap(Bitmap *imagePtr);

	void set_color(u32 color);
	u32 get_color() const;

	void set_world_matrix(const hlslpp::float3x3& mat); 
	hlslpp::float3x3 get_world_matrix() const;

	void set_view_matrix(hlslpp::float3x3 const& mat); 
	hlslpp::float3x3 get_view_matrix() const;

	void set_bitmap_interpolation_mode(bitmap_interpolation_mode mode);

	void set_default_font() { _font = _default_font; }
	void set_font(Font *font) {
		_font = font;
		if (!_font)
			_font = _default_font;
	}
	Font *get_font() const { return _font; }

	private:
	ID2D1Factory *_factory;
	ID2D1RenderTarget* _rt;
	Font *_default_font;

	// Currently in-use brush (non-owning)
	ID2D1SolidColorBrush *_brush;


	// Currently in-use font (non-owning)
	Font *_font;

	D2D1_BITMAP_INTERPOLATION_MODE _interpolation_mode;

	hlslpp::float3x3 _mat_world;
	hlslpp::float3x3 _mat_view;


	void update_transforms();
};

} // namespace graphics
#endif