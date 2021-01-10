#include "stdafx.h"
#include "Box2DDebugRenderer.h"


Box2DDebugRenderer::Box2DDebugRenderer()
{
	m_Color = COLOR(255, 255, 255, 187);
	m_MatScale = MATRIX3X2::CreateScalingMatrix(PhysicsActor::SCALE);
}

Box2DDebugRenderer::~Box2DDebugRenderer()
{

}

//------------------------------------------------------------------------------
// Box2DDebugRenderer class definitions. Draws Box2D debug information
//------------------------------------------------------------------------------

/// Draw a closed polygon provided in CCW order.
void Box2DDebugRenderer::DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color)
{
	std::vector<DOUBLE2> vertexsArr;
	for (int i = 0; i < vertexCount; i++)
	{
		vertexsArr.push_back(DOUBLE2(vertices[i].x, vertices[i].y));
	}
	game_engine::instance()->SetWorldMatrix(m_MatScale);
	game_engine::instance()->set_color(COLOR((byte)color.r, (byte)color.g, (byte)color.b, m_Color.alpha));
	game_engine::instance()->DrawPolygon(vertexsArr, vertexCount, true, 1 / PhysicsActor::SCALE);
}

//------------------------------------------------------------------------------
// Box2DDebugRenderer class definitions. Encapsulated the user defined game settings
//------------------------------------------------------------------------------
/// Draw a solid closed polygon provided in CCW order.
void Box2DDebugRenderer::DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color)
{
	UNREFERENCED_PARAMETER(color);
	std::vector<DOUBLE2> vertexsArr;
	for (int i = 0; i < vertexCount; i++)
	{
		vertexsArr.push_back(DOUBLE2(vertices[i].x, vertices[i].y));
	}
	game_engine::instance()->SetWorldMatrix(m_MatScale);
	game_engine::instance()->set_color(m_Color);
	game_engine::instance()->FillPolygon(vertexsArr, vertexCount);
}

/// Draw a circle.
void Box2DDebugRenderer::DrawCircle(const b2Vec2& center, float radius, const b2Color& color)
{
	UNREFERENCED_PARAMETER(color);

	game_engine::instance()->SetWorldMatrix(m_MatScale);
	game_engine::instance()->set_color(m_Color);
	game_engine::instance()->DrawEllipse(DOUBLE2(center.x, center.y), radius, radius, (double)1 / PhysicsActor::SCALE);
}

/// Draw a solid circle.
void Box2DDebugRenderer::DrawSolidCircle(const b2Vec2& center, float radius, const b2Vec2& axis, const b2Color& color)
{
	UNREFERENCED_PARAMETER(color);
	UNREFERENCED_PARAMETER(axis);


	game_engine::instance()->SetWorldMatrix(m_MatScale);
	game_engine::instance()->set_color(m_Color);
	game_engine::instance()->FillEllipse(DOUBLE2(center.x, center.y), radius, radius);
}

/// Draw a line segment.
void Box2DDebugRenderer::DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color)
{
	UNREFERENCED_PARAMETER(color);

	game_engine::instance()->SetWorldMatrix(m_MatScale);
	game_engine::instance()->set_color(m_Color);
	game_engine::instance()->DrawLine(DOUBLE2(p1.x, p1.y), DOUBLE2(p2.x, p2.y), (double)1 / PhysicsActor::SCALE);
}

/// Draw a transform. Choose your own length scale.
/// @param xf a transform.
void Box2DDebugRenderer::DrawTransform(const b2Transform& xf)
{
	game_engine::instance()->SetWorldMatrix(m_MatScale);
	game_engine::instance()->set_color(m_Color);
	// the position
	game_engine::instance()->set_color(COLOR(0, 0, 0));
	game_engine::instance()->DrawEllipse(DOUBLE2(xf.p.x, xf.p.y), (double)2 / PhysicsActor::SCALE, (double)2 / PhysicsActor::SCALE, (double)1 / PhysicsActor::SCALE);// DOUBLE2(p2.x, p2.y), (double)1 / PhysicsActor::SCALE);
	// the x axis
	DOUBLE2 x(xf.q.GetXAxis().x, xf.q.GetXAxis().y);
	x = x / PhysicsActor::SCALE * 20;
	game_engine::instance()->set_color(COLOR(255,0,187));
	game_engine::instance()->DrawLine(DOUBLE2(xf.p.x, xf.p.y), DOUBLE2(xf.p.x, xf.p.y) + x, (double)1 / PhysicsActor::SCALE);

	// the y axis
	DOUBLE2 y(xf.q.GetYAxis().x, xf.q.GetYAxis().y);
	y = y / PhysicsActor::SCALE * 20;
	game_engine::instance()->set_color(COLOR(0,255,187));
	game_engine::instance()->DrawLine(DOUBLE2(xf.p.x, xf.p.y), DOUBLE2(xf.p.x, xf.p.y) + y, (double)1 / PhysicsActor::SCALE);

}