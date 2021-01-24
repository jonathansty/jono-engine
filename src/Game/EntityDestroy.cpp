#include "stdafx.h"		
#include "EntityDestroy.h"

EntityDestroy::EntityDestroy(float2 position) 
    : Animation(position)
{
	// nothing to create
	// m_ActCirclePtr->AddContactListener(this);
}

EntityDestroy::~EntityDestroy()
{
	// nothing to destroy
}

//-------------------------------------------------------
// ContactListener overloaded member function definitions
//-------------------------------------------------------
//void EntityDestroy::BeginContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr)
//{
//
//}
//
//void EntityDestroy::EndContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr)
//{
//
//}
//
//void EntityDestroy::ContactImpulse(PhysicsActor *actThisPtr, double impulse)
//{
//
//}
void EntityDestroy::Tick(double deltaTime)
{
    if (m_Opacity >0)
    {
        m_Opacity -= deltaTime;
        m_Scale += deltaTime;
    }
    else
    {
        m_Opacity = 0;
        m_IsEnded = true;
    }

    
}
void EntityDestroy::Paint(graphics::D2DRenderContext& ctx)
{
    float3x3 matTranslate,matScale, matPivot;
    matTranslate= float3x3::translation(m_Position);
    matScale = float3x3::scale(m_Scale);
    matPivot= float3x3::translation(float2(0,0));
    ctx.set_color(COLOR(255,255,255, m_Opacity * 255));
    ctx.set_world_matrix(matPivot * matScale * matTranslate);
    ctx.fill_ellipse(float2(), m_Radius, m_Radius);
    ctx.set_world_matrix(float3x3::identity());
    ctx.set_color(COLOR(0, 0, 0,255));
}
double EntityDestroy::GetOpacity()
{
    return m_Opacity;
}
void EntityDestroy::SetRadius(int radius)
{
    m_Radius = radius;
}