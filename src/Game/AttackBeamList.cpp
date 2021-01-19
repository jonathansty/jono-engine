#include "stdafx.h"		
	
#include "AttackBeamList.h"
#include "AttackBeam.h"

AttackBeamList::AttackBeamList()
{
	// nothing to create
	// m_ActCirclePtr->AddContactListener(this);
}

AttackBeamList::~AttackBeamList()
{
    for (size_t i = 0; i < m_AttackBeamsPtrArr.size(); i++)
    {
        delete m_AttackBeamsPtrArr[i];
    }
    m_AttackBeamsPtrArr.clear();
}

//-------------------------------------------------------
// ContactListener overloaded member function definitions
//-------------------------------------------------------
//void AttackBeamList::BeginContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr)
//{
//
//}
//
//void AttackBeamList::EndContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr)
//{
//
//}
//
//void AttackBeamList::ContactImpulse(PhysicsActor *actThisPtr, double impulse)
//{
//
//}
void AttackBeamList::Paint(graphics::D2DRenderContext& ctx)
{
    for (size_t i = 0; i < m_AttackBeamsPtrArr.size(); i++)
    {
        if (m_AttackBeamsPtrArr[i] != nullptr)
        {
            m_AttackBeamsPtrArr[i]->Paint();
        }
        
    }
}
void AttackBeamList::Tick(double deltaTime)
{

    for (size_t i = 0; i < m_AttackBeamsPtrArr.size(); i++)
    {
        if (m_AttackBeamsPtrArr[i] != nullptr)
        {
            m_AttackBeamsPtrArr[i]->Tick(deltaTime);
        }
        
        if (m_AttackBeamsPtrArr[i] != nullptr && m_AttackBeamsPtrArr[i]->GetLifeTime() > LIFETIME)
        {
            delete m_AttackBeamsPtrArr[i];
            m_AttackBeamsPtrArr[i] = nullptr;
        }
    }
}
void AttackBeamList::Add(AttackBeam* attackBeamPtr)
{
    m_AttackBeamsPtrArr.push_front(attackBeamPtr);
}
void AttackBeamList::Remove()
{
    for (size_t i = 0; i < m_AttackBeamsPtrArr.size(); i++)
    {
        delete m_AttackBeamsPtrArr[i];

    }
    m_AttackBeamsPtrArr.clear();
}

