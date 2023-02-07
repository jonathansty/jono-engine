#pragma once

#if FEATURE_D2D

#include "Graphics.h"
#include "Renderer.h"
#include "Shader.h"
#include "ShaderTypes.h"

struct ID2D1Factory;
struct ID2D1RenderTarget;
struct ID2D1SolidColorBrush;
class Font;
class Bitmap;
class GraphicsThread;

using hlslpp::float2;

namespace Graphics
{

// Bitmap interpolation mode enum
enum class bitmap_interpolation_mode
{
	linear,
	nearest_neighbor
};

struct SimpleVertex2D
{
	Shaders::float2 pos;
	Shaders::float2 uv;
};

struct DrawCmd
{
	enum Command
	{
		DC_MESH,
		DC_CLEAR
	};

	Command m_Type = DC_MESH;

	std::vector<u32> m_IdxBuffer;
	std::vector<SimpleVertex2D> m_VertexBuffer;

	u32 m_IdxOffset;
	u32 m_VertexOffset;

	float4 m_Colour;
	float4x4 m_WorldViewMatrix;

	GraphicsResourceHandle m_TextureSRV = GraphicsResourceHandle::Invalid();

	static constexpr u8 c_DrawCmdSize = 48;
	u8 m_Data[c_DrawCmdSize];
};


// Lightweight 2D render context that can be used to draw to a D2D target.
class ENGINE_API D2DRenderContext
{
public:
	D2DRenderContext(Renderer* renderer, ID2D1Factory* factory, ID2D1RenderTarget* rt, ID2D1SolidColorBrush* brush, Font* font);
	D2DRenderContext();

	D2DRenderContext(D2DRenderContext&) = delete;
	D2DRenderContext& operator=(D2DRenderContext const&) = delete;
	~D2DRenderContext() = default;

	bool begin_paint(Renderer* renderer, ID2D1Factory* factory, ID2D1RenderTarget* rt, ID2D1SolidColorBrush* brush, Font* font);
	bool end_paint();

	bool draw_background(u32 backgroundColor);
	//! Draws a line from p1 to p2 using the strokewidth
	bool draw_line(float2 p1, float2 p2, float strokeWidth = 1.0f);
	//! Draws a line from the coordinate defined by x1 and y1 to the coordinate define by x2 and y2
	bool draw_line(int x1, int y1, int x2, int y2);

	//! Draws a polygon defined by the coordinates in ptsArr
	//! count is the number of points that must be drawn
	//! If close is true then it will connect the start and end coordinate
	bool draw_polygon(const std::vector<float2>& ptsArr, unsigned int count, bool close = true, double strokeWidth = 1.0);

	//! Draws a polygon defined by the coordinates in ptsArr
	//! count is the number of points that must be drawn
	//! If close is true then it will connect the start and end coordinate
	bool draw_polygon(const std::vector<POINT>& ptsArr, unsigned int count, bool close = true);

	//! Fills the interior of a polygon defined by the coordinates in ptsArr
	//! count is the number of points that must be drawn
	//! If close is true then it will connect the start and end coordinate
	bool fill_polygon(const std::vector<float2>& ptsArr, unsigned int count);

	//! Fills the interior of a polygon defined by the coordinates in ptsArr
	//! count is the number of points that must be drawn
	//! If close is true then it will connect the start and end coordinate
	bool fill_polygon(const std::vector<POINT>& ptsArr, unsigned int count);

	using Rect = D2D1_RECT_F;

	bool draw_rect(Rect rect, float strokeWidth = 1);
	bool draw_rect(float2 topLeft, float2 rightbottom, float strokeWidth = 1.0f);
	bool draw_rect(RECT rect);
	bool draw_rect(int left, int top, int right, int bottom);

	bool fill_rect(Rect rect);
	bool fill_rect(float2 topLeft, float2 rightbottom);
	bool fill_rect(RECT rect);
	bool fill_rect(int left, int top, int right, int bottom);

	bool draw_rounded_rect(Rect rect, int radiusX, int radiusY, float strokeWidth = 1.0f);
	bool draw_rounded_rect(float2 topLeft, float2 rightbottom, int radiusX, int radiusY, float strokeWidth = 1.0f);
	bool draw_rounded_rect(RECT rect, int radiusX, int radiusY);
	bool draw_rounded_rect(int left, int top, int right, int bottom, int radiusX, int radiusY);

	bool fill_rounded_rect(Rect rect, int radiusX, int radiusY);
	bool fill_rounded_rect(float2 topLeft, float2 rightbottom, int radiusX, int radiusY);
	bool fill_rounded_rect(RECT rect, int radiusX, int radiusY);
	bool fill_rounded_rect(int left, int top, int right, int bottom, int radiusX, int radiusY);

	bool draw_ellipse(float2 centerPt, double radiusX, double radiusY, float strokeWidth = 1.0f);
	bool draw_ellipse(int centerX, int centerY, int radiusX, int radiusY);

	//! Paints the interior of the specified ellipse using the specified position and radius
	bool fill_ellipse(float2 centerPt, double radiusX, double radiusY);

	//! Paints the interior of the specified ellipse using the specified position and radius
	bool fill_ellipse(int centerX, int centerY, int radiusX, int radiusY);

	//! Draws text in the specified rectangle
	bool draw_string(const string& textRef, RECT boundingRect);

	//! Draws text in the specified rectangle
	bool draw_string(const string& textRef, Rect boundingRect);

	//! Draws text in the specified rectangle the topleft corner of the rectange is defined by the param topLeft
	//! The params right and bottom are optional, if left out they are set to the max value of an float type
	bool draw_string(const string& textRef, float2 topLeft, double right = -1, double bottom = -1);

	//! Draws text in the specified rectangle; the topleft corner of the rectange is defined by the params xPos and yPos
	//! The params right and bottom are optional, if left out they are set to the max value of an float type
	bool draw_string(const string& textRef, int xPos, int yPos, int right = -1, int bottom = -1);

	//! Draws an image on the position
	//! srcRect: defines the cliprect on the source image. Allows to draw a part of an image
	bool draw_bitmap(Bitmap* imagePtr, float2 position, Rect srcRect);

	//! Draws an image on the position
	bool draw_bitmap(Bitmap* imagePtr, float2 position);

	//! Draws an image on the position defined by x and y
	//! srcRect: defines the cliprect on the source image. Allows to draw a part of an image
	bool draw_bitmap(Bitmap* imagePtr, int x, int y, RECT srcRect);

	//! Draws an image on the position defined by x and y
	bool draw_bitmap(Bitmap* imagePtr, int x, int y);

	//! Draws an image on position x:0, and y:0. Assuming that matrices are used to define the position.
	//! srcRect: defines the cliprect on the source image. Allows to draw a part of an image
	bool draw_bitmap(Bitmap* imagePtr, RECT srcRect);

	//! Draws an image on position x:0, and y:0. Assuming that matrices are used to define the position.
	bool draw_bitmap(Bitmap* imagePtr);

	void set_color(u32 color);
	u32 get_color() const;

	void set_world_matrix(const hlslpp::float4x4& mat);
	hlslpp::float4x4 get_world_matrix() const;

	void set_view_matrix(hlslpp::float4x4 const& mat);
	hlslpp::float4x4 get_view_matrix() const;

	void set_bitmap_interpolation_mode(bitmap_interpolation_mode mode);

	void set_default_font() { m_Font = m_DefaultFont; }
	void set_font(Font* font)
	{
		m_Font = font;
		if (!m_Font)
			m_Font = m_DefaultFont;
	}
	Font* get_font() const { return m_Font; }

	std::vector<DrawCmd> GetCommands() const { return m_DrawCommands; }

 private:
	Renderer* m_Renderer;
	ID2D1Factory* m_Factory;
	ID2D1RenderTarget* m_RenderTarget;
	Font* m_DefaultFont;

	// Currently in-use brush (non-owning)
	ID2D1SolidColorBrush* m_Brush;
	D2D1::ColorF m_Colour;

	// Currently in-use font (non-owning)
	Font* m_Font;

	D2D1_BITMAP_INTERPOLATION_MODE m_InterpolationMode;

	hlslpp::float4x4 m_WorldMatrix;
	hlslpp::float4x4 m_ViewMatrix;


	std::vector<DrawCmd> m_DrawCommands;

	u32 m_TotalVertices;
	u32 m_TotalIndices;

	u32 m_CurrentVertices;
	u32 m_CurrentIndices;
	ComPtr<ID3D11Buffer> m_VertexBuffer;
	ComPtr<ID3D11Buffer> m_IndexBuffer;
	ComPtr<ID3D11InputLayout> m_InputLayout;

	ConstantBufferRef m_GlobalCB;
	ShaderRef m_VertexShader;
	ShaderRef m_PixelShader;

	hlslpp::float4x4 m_ProjectionMatrix;

	void update_transforms();

	friend GraphicsThread;
};

} // namespace Graphics
#endif