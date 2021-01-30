#include "game.stdafx.h"		
#include "CheckPointBg.h"

const double CheckPointBg::SCALESPEED = 0.5;

CheckPointBg::CheckPointBg(float2 position, Bitmap* bmpPtr)
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
    float3x3 matTranslate,matRotate,matPivot, matScale, matWorldTransform;
    matTranslate = float3x3::translation(m_Position);
    matPivot = float3x3::translation(float2(-m_BmpPtr->GetWidth() / 2, -m_BmpPtr->GetHeight() / 2));
    matScale = float3x3::scale(m_Scale);
    matRotate = float3x3::rotation_z(m_Angle);
    matWorldTransform = matPivot * matScale * matRotate * matTranslate;
    ctx.set_world_matrix(matWorldTransform);
    ctx.draw_bitmap(m_BmpPtr);
	ctx.set_world_matrix(float3x3::identity());
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
void CheckPointBg::SetPosition(float2 position)
{
    m_Scale = 0.01;
    m_ScaleSpeed = SCALESPEED;
    m_DrawState = drawState::SPAWNING;
    m_Position = position;
}
void CheckPointBg::SetDrawState(drawState drawstate)
{
}
float2 CheckPointBg::GetPosition()
{
    return m_Position;
}