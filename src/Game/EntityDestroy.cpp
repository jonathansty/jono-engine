#include "stdafx.h"		
#include "EntityDestroy.h"

EntityDestroy::EntityDestroy(DOUBLE2 position) 
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
    MATRIX3X2 matTranslate,matScale, matPivot;
    matTranslate.SetAsTranslate(m_Position);
    matScale.SetAsScale(m_Scale);
    matPivot.SetAsTranslate(DOUBLE2(0,0));
    ctx.set_color(COLOR(255,255,255, m_Opacity * 255));
    GameEngine::instance()->set_world_matrix(matPivot * matScale * matTranslate);
    ctx.fill_ellipse(DOUBLE2(), m_Radius, m_Radius);
    GameEngine::instance()->set_world_matrix(MATRIX3X2::CreateIdentityMatrix());
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