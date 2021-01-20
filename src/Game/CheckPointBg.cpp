#include "stdafx.h"		
#include "CheckPointBg.h"

const double CheckPointBg::SCALESPEED = 0.5;

CheckPointBg::CheckPointBg(DOUBLE2 position, Bitmap* bmpPtr)
    : Animation(position)
    , m_BmpPtr(bmpPtr)
{
	// nothing to create
	// m_ActCirclePtr->AddContactListener(this);
    m_Position = position;
    m_ScaleSpeed = SCALESPEED;
}


CheckPointBg::~CheckPointBg()
{
    
}

//-------------------------------------------------------
// ContactListener overloaded member function definitions
//-------------------------------------------------------
//void CheckPointBg::BeginContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr)
//{
//
//}
//
//void CheckPointBg::EndContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr)
//{
//
//}
//
//void CheckPointBg::ContactImpulse(PhysicsActor *actThisPtr, double impulse)
//{
//
//}
void CheckPointBg::Paint(graphics::D2DRenderContext& ctx)
{
    MATRIX3X2 matTranslate,matRotate,matPivot, matScale, matWorldTransform;
    matTranslate.SetAsTranslate(m_Position);
    matPivot.SetAsTranslate(DOUBLE2(-m_BmpPtr->GetWidth() / 2, -m_BmpPtr->GetHeight() / 2));
    matScale.SetAsScale(m_Scale);
    matRotate.SetAsRotate(m_Angle);
    matWorldTransform = matPivot * matScale * matRotate * matTranslate;
    ctx.set_world_matrix(matWorldTransform);
    ctx.draw_bitmap(m_BmpPtr);
    ctx.set_world_matrix(MATRIX3X2::CreateIdentityMatrix());
}
void CheckPointBg::Tick(double deltaTime)
{

    if (m_Scale >= m_MaxScale)
    {
        m_DrawState = drawState::MAXRADIUS;
    }
    if (m_DrawState == drawState::SPAWNING)
    {
        m_Scale += m_ScaleSpeed*5 *deltaTime;
    }
    if (m_DrawState == drawState::MAXRADIUS)
    {
        if (m_Scale < m_MaxScale + m_ScaleOffSet && m_Scale > m_MaxScale - m_ScaleOffSet)
        {
            m_Scale += m_ScaleSpeed*deltaTime;
        }
        else
        {
            m_ScaleSpeed *= -1;
        }
        m_Scale += m_ScaleSpeed*deltaTime;
    }
    m_Angle += deltaTime;
    
    
}
void CheckPointBg::SetPosition(DOUBLE2 position)
{
    m_Scale = 0.01;
    m_ScaleSpeed = SCALESPEED;
    m_DrawState = drawState::SPAWNING;
    m_Position = position;
}
void CheckPointBg::SetDrawState(drawState drawstate)
{
}
DOUBLE2 CheckPointBg::GetPosition()
{
    return m_Position;
}