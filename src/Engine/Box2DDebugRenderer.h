#pragma once

namespace graphics {
	class D2DRenderContext;
}


class Box2DDebugRenderer : public b2Draw
{
public:
	Box2DDebugRenderer();
	virtual ~Box2DDebugRenderer();

	// C++11 make the class non-copyable
	Box2DDebugRenderer(const Box2DDebugRenderer&) = delete;
	Box2DDebugRenderer& operator=(const Box2DDebugRenderer&) = delete;

	// Draw a closed polygon provided in CCW order.
	void DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color) override;

	// Draw a solid closed polygon provided in CCW order.
	void DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color) override;

	// Draw a circle.
	void DrawCircle(const b2Vec2& center, float radius, const b2Color& color) override;

	// Draw a solid circle.
	void DrawSolidCircle(const b2Vec2& center, float radius, const b2Vec2& axis, const b2Color& color) override;

	// Draw a line segment.
	void DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color) override;

	// Draw a transform. Choose your own length scale.
	void DrawTransform(const b2Transform& xf) override;

	void DrawPoint(const b2Vec2& pos, float size, const b2Color& color) override;;

	void set_draw_ctx(class graphics::D2DRenderContext* context);

private:
	class graphics::D2DRenderContext *_context;
	COLOR m_Color;
	MATRIX3X2 m_MatScale;
};
