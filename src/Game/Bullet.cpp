#include "stdafx.h"		
#include "Bullet.h"

Bullet::Bullet(float2 startPosition, float2 velocity)
	: m_startPosition(startPosition)
	, m_Direction(velocity)
{
	// nothing to create
	// m_ActCirclePtr->AddContactListener(this);
    m_ActPtr = new PhysicsActor(startPosition, 0, BodyType::DYNAMIC);
    m_ActPtr->SetBullet(true);
    m_ActPtr->SetTrigger(true);
    m_ActPtr->SetGravityScale(0);
    m_ActPtr->AddBoxShape(10, 20);
    m_ActPtr->SetLinearVelocity(velocity);
    m_ActPtr->AddContactListener(this);
    
}

Bullet::~Bullet()
{
    m_ActPtr->RemoveContactListener(this);
    delete m_ActPtr;
    m_ActPtr = nullptr;

}

//-------------------------------------------------------
// ContactListener overloaded member function definitions
//-------------------------------------------------------
void Bullet::BeginContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr)
{

}

void Bullet::EndContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr)
{

}

void Bullet::ContactImpulse(PhysicsActor *actThisPtr, double impulse)
{

}
