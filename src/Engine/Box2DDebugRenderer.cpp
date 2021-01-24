#include "stdafx.h"
#include "Box2DDebugRenderer.h"

#include "graphics/2DRenderContext.h"


Box2DDebugRenderer::Box2DDebugRenderer()
{
	m_Color = COLOR(255, 255, 255, 187);
	_mat_scale = float3x3::scale(PhysicsActor::SCALE);
}

Box2DDebugRenderer::~Box2DDebugRenderer()
{

}

/// Draw a closed polygon provided in CCW order.
void Box2DDebugRenderer::DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color)
{
	std::vector<float2> vertexsArr;
	for (int i = 0; i < vertexCount; i++)
	{
		vertexsArr.push_back(float2(vertices[i].x, vertices[i].y));
	}
	_context->set_world_matrix(_mat_scale);
	_context->set_color(COLOR((byte)color.r, (byte)color.g, (byte)color.b, m_Color.alpha));
	_context->draw_polygon(vertexsArr, vertexCount, true, 1 / PhysicsActor::SCALE);
}

//------------------------------------------------------------------------------
// Box2DDebugRenderer class definitions. Encapsulated the user defined game settings
//------------------------------------------------------------------------------
/// Draw a solid closed polygon provided in CCW order.
void Box2DDebugRenderer::DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color)
{
	UNREFERENCED_PARAMETER(color);
	std::vector<float2> vertexsArr;
	for (int i = 0; i < vertexCount; i++)
	{
		vertexsArr.push_back(float2(vertices[i].x, vertices[i].y));
	}
	_context->set_world_matrix(_mat_scale);
	_context->set_color(m_Color);
	_context->fill_polygon(vertexsArr, vertexCount);
}

/// Draw a circle.
void Box2DDebugRenderer::DrawCircle(const b2Vec2& center, float radius, const b2Color& color)
{
	UNREFERENCED_PARAMETER(color);

	_context->set_world_matrix(_mat_scale);
	_context->set_color(m_Color);
	_context->draw_ellipse(float2(center.x, center.y), radius, radius, (double)1 / PhysicsActor::SCALE);
}

/// Draw a solid circle.
void Box2DDebugRenderer::DrawSolidCircle(const b2Vec2& center, float radius, const b2Vec2& axis, const b2Color& color)
{
	UNREFERENCED_PARAMETER(color);
	UNREFERENCED_PARAMETER(axis);


	_context->set_world_matrix(_mat_scale);
	_context->set_color(m_Color);
	_context->fill_ellipse(float2(center.x, center.y), radius, radius);
}

/// Draw a line segment.
void Box2DDebugRenderer::DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color)
{
	UNREFERENCED_PARAMETER(color);

	_context->set_world_matrix(_mat_scale);
	_context->set_color(m_Color);
	_context->draw_line(float2(p1.x, p1.y), float2(p2.x, p2.y), (double)1 / PhysicsActor::SCALE);
}

/// Draw a transform. Choose your own length scale.
/// @param xf a transform.
void Box2DDebugRenderer::DrawTransform(const b2Transform& xf)
{
	_context->set_world_matrix(_mat_scale);
	_context->set_color(m_Color);
	// the position
	_context->set_color(COLOR(0, 0, 0));
	_context->draw_ellipse(float2(xf.p.x, xf.p.y), (double)2 / PhysicsActor::SCALE, (double)2 / PhysicsActor::SCALE, (double)1 / PhysicsActor::SCALE); // float2(p2.x, p2.y), (double)1 / PhysicsActor::SCALE);
	// the x axis
	float2 x(xf.q.GetXAxis().x, xf.q.GetXAxis().y);
	x = x / PhysicsActor::SCALE * 20;
	_context->set_color(COLOR(255,0,187));
	_context->draw_line(float2(xf.p.x, xf.p.y), float2(xf.p.x, xf.p.y) + x, (double)1 / PhysicsActor::SCALE);

	// the y axis
	float2 y(xf.q.GetYAxis().x, xf.q.GetYAxis().y);
	y = y / PhysicsActor::SCALE * 20;
	_context->set_color(COLOR(0, 255, 187));
	_context->draw_line(float2(xf.p.x, xf.p.y), float2(xf.p.x, xf.p.y) + y, (double)1 / PhysicsActor::SCALE);

}

void Box2DDebugRenderer::DrawPoint(const b2Vec2 &pos, float size, const b2Color &color) {
	assert(_context);

	_context->set_world_matrix(_mat_scale);
	float x = pos.x;
	float y = pos.y;

	_context->set_color(m_Color);
	_context->draw_ellipse(float2(x, y), size / (2.0*PhysicsActor::SCALE), size / (2.0 * PhysicsActor::SCALE));
}

void Box2DDebugRenderer::set_draw_ctx(class graphics::D2DRenderContext *context) {
	_context = context;
}
