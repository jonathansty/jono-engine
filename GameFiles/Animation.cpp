#include "stdafx.h"		
#include "Animation.h"

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


