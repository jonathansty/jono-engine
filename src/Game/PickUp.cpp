#include "stdafx.h"		
	
#include "PickUp.h"
#include "SoundManager.h"


//---------------------------
// Constructor & Destructor
//---------------------------
PickUp::PickUp(DOUBLE2 position):
m_Position(position)
{
	// nothing to create
	// m_ActCirclePtr->AddContactListener(this);
    m_SndHitPtr = SoundManager::instance()->LoadSound(String("Resources/Sound/Entity/CoinPickup.wav"));
}

PickUp::~PickUp()
{
    if (m_ActPtr != nullptr)
    {
        delete m_ActPtr;
        m_ActPtr = nullptr;
    }
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