//-----------------------------------------------------
// Name:
// First name:
// Group: 1DAE.
//-----------------------------------------------------
#include "stdafx.h"		
	
//---------------------------
// Includes
//---------------------------
#include "BlockSlide.h"
#include "Avatar.h"
//---------------------------
// Defines
//---------------------------
#define GAME_ENGINE (GameEngine::GetSingleton())

//---------------------------
// Constructor & Destructor
//---------------------------
BlockSlide::BlockSlide(DOUBLE2 position, int width, int height) :
Entity(position), m_Width(width), m_Height(height)
{
    m_ActPtr = new PhysicsActor(position, 0, BodyType::STATIC);
    m_ActPtr->AddBoxShape(width, height, 0, 1);
    m_ActPtr->SetName(String("BlockSlide"));
    m_Position = position;
}

BlockSlide::~BlockSlide()
{
	
}

//-------------------------------------------------------
// ContactListener overloaded member function definitions
//-------------------------------------------------------
void BlockSlide::BeginContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr)
{
    if (actOtherPtr == m_AvatarPtr->GetActor() && actThisPtr == m_ActPtr)
    {
        m_IsHit = true;
    }
}

void BlockSlide::EndContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr)
{
    if (actOtherPtr == m_AvatarPtr->GetActor() && actThisPtr == m_ActPtr)
    {
        m_IsHit = false;
    }
}

void BlockSlide::ContactImpulse(PhysicsActor *actThisPtr, double impulse)
{

}
void BlockSlide::Paint()
{
    MATRIX3X2 matTranslate, matPivot;
    matTranslate.SetAsTranslate(m_Position);
    matPivot.SetAsTranslate(DOUBLE2(-m_Width / 2, -m_Height / 2));
    GAME_ENGINE->SetWorldMatrix(matPivot * matTranslate);
    GAME_ENGINE->FillRect(0, 0, m_Width, m_Height);
    GAME_ENGINE->SetColor(COLOR(255,255,255));
    GAME_ENGINE->FillRect(0, 0, m_Width, 5);
    GAME_ENGINE->SetColor(COLOR(0, 0, 0));
    GAME_ENGINE->SetWorldMatrix(MATRIX3X2::CreateIdentityMatrix());
}
void BlockSlide::Tick(double deltaTime)
{
    
}
void BlockSlide::Reset()
{

}


