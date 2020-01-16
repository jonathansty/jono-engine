//-----------------------------------------------------
// Name: Steyfkens
// First name: Jonathan
// Group: 1DAE01
//-----------------------------------------------------
#include "stdafx.h"		
	
//---------------------------
// Includes
//---------------------------
#include "Animation.h"

//---------------------------
// Defines
//---------------------------
#define GAME_ENGINE (GameEngine::GetSingleton())

//---------------------------
// Constructor & Destructor
//---------------------------
Animation::Animation(DOUBLE2 position) :
m_Position(position)
{
	// nothing to create
	// m_ActCirclePtr->AddContactListener(this);
}

Animation::~Animation()
{
	// nothing to destroy
}

//-------------------------------------------------------
// ContactListener overloaded member function definitions
//-------------------------------------------------------
//void Animation::BeginContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr)
//{
//
//}
//
//void Animation::EndContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr)
//{
//
//}
//
//void Animation::ContactImpulse(PhysicsActor *actThisPtr, double impulse)
//{
//
//}
bool Animation::IsEnded()
{
    return m_IsEnded;
}


