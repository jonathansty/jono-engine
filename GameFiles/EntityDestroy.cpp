//-----------------------------------------------------
// Name: Steyfkens
// First name: Jonathan
// Group: 1DAE01
//-----------------------------------------------------
#include "stdafx.h"		
	
//---------------------------
// Includes
//---------------------------
#include "EntityDestroy.h"

//---------------------------
// Defines
//---------------------------
#define GAME_ENGINE (GameEngine::GetSingleton())

//---------------------------
// Constructor & Destructor
//---------------------------
EntityDestroy::EntityDestroy(DOUBLE2 position) :
Animation(position)
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
void EntityDestroy::Paint()
{
    MATRIX3X2 matTranslate,matScale, matPivot;
    matTranslate.SetAsTranslate(m_Position);
    matScale.SetAsScale(m_Scale);
    matPivot.SetAsTranslate(DOUBLE2(0,0));
    GAME_ENGINE->SetColor(COLOR(255,255,255, m_Opacity * 255));
    GAME_ENGINE->SetWorldMatrix(matPivot * matScale * matTranslate);
    GAME_ENGINE->FillEllipse(DOUBLE2(), m_Radius, m_Radius);
    GAME_ENGINE->SetWorldMatrix(MATRIX3X2::CreateIdentityMatrix());
    GAME_ENGINE->SetColor(COLOR(0, 0, 0,255));
}
double EntityDestroy::GetOpacity()
{
    return m_Opacity;
}
void EntityDestroy::SetRadius(int radius)
{
    m_Radius = radius;
}