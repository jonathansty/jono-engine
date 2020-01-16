//-----------------------------------------------------
// Name: Steyfkens
// First name: Jonathan
// Group: 1DAE01
//-----------------------------------------------------
#include "stdafx.h"		
	
//---------------------------
// Includes
//---------------------------
#include "CheckPointBg.h"

//---------------------------
// Defines
//---------------------------
#define GAME_ENGINE (GameEngine::GetSingleton())

//---------------------------
// Constructor & Destructor
//---------------------------
const double CheckPointBg::SCALESPEED = 0.5;
CheckPointBg::CheckPointBg(DOUBLE2 position, Bitmap* bmpPtr):
Animation(position),
m_BmpPtr(bmpPtr)
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
void CheckPointBg::Paint()
{
    MATRIX3X2 matTranslate,matRotate,matPivot, matScale, matWorldTransform;
    matTranslate.SetAsTranslate(m_Position);
    matPivot.SetAsTranslate(DOUBLE2(-m_BmpPtr->GetWidth() / 2, -m_BmpPtr->GetHeight() / 2));
    matScale.SetAsScale(m_Scale);
    matRotate.SetAsRotate(m_Angle);
    matWorldTransform = matPivot * matScale * matRotate * matTranslate;
    GAME_ENGINE->SetWorldMatrix(matWorldTransform);
    GAME_ENGINE->DrawBitmap(m_BmpPtr);
    GAME_ENGINE->SetWorldMatrix(MATRIX3X2::CreateIdentityMatrix());
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