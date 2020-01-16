//-----------------------------------------------------
// Name: Steyfkens
// First name: Jonathan
// Group: 1DAE01
//-----------------------------------------------------
#include "stdafx.h"		
	
//---------------------------
// Includes
//---------------------------
#include "PickUp.h"

//---------------------------
// Defines
//---------------------------
#define GAME_ENGINE (GameEngine::GetSingleton())

//---------------------------
// Constructor & Destructor
//---------------------------
PickUp::PickUp(DOUBLE2 position):
m_Position(position)
{
	// nothing to create
	// m_ActCirclePtr->AddContactListener(this);
    m_SndHitPtr = SoundManager::GetSingleton()->LoadSound(String("Resources/Sound/Entity/CoinPickup.wav"));
}

PickUp::~PickUp()
{
    if (m_ActPtr != nullptr)
    {
        delete m_ActPtr;
        m_ActPtr = nullptr;
    }
}

//-------------------------------------------------------
// ContactListener overloaded member function definitions
//-------------------------------------------------------
void PickUp::BeginContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr)
{

}

void PickUp::EndContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr)
{

}

void PickUp::ContactImpulse(PhysicsActor *actThisPtr, double impulse)
{

}
bool PickUp::IsHit()
{
    return m_IsHit;
}
void PickUp::SetName(String name)
{
    m_Name = name;
}
String PickUp::GetName()
{
    return m_Name;
}
PhysicsActor* PickUp::GetActor()
{
    return m_ActPtr;
}