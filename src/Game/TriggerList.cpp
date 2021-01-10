//-----------------------------------------------------
// Name:
// First name:
// Group: 1DAE.
//-----------------------------------------------------
#include "stdafx.h"		
	
//---------------------------
// Includes
//---------------------------
#include "TriggerList.h"
#include "CameraTrigger.h"
//---------------------------
// Defines
//---------------------------
//#define game_engine::instance() (GameEngine::GetSingleton())

//---------------------------
// Constructor & Destructor
//---------------------------
TriggerList::TriggerList()
{
	// nothing to create
	// m_ActCirclePtr->AddContactListener(this);
}

TriggerList::~TriggerList()
{
    for (size_t i = 0; i < m_CameraTriggersPtrArr.size(); i++)
    {
        delete m_CameraTriggersPtrArr[i];
    }
    m_CameraTriggersPtrArr.clear();
}

//-------------------------------------------------------
// ContactListener overloaded member function definitions
//-------------------------------------------------------
//void TriggerList::BeginContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr)
//{
//
//}
//
//void TriggerList::EndContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr)
//{
//
//}
//
//void TriggerList::ContactImpulse(PhysicsActor *actThisPtr, double impulse)
//{
//
//}
void TriggerList::Add(CameraTrigger* cameraTriggerPtr)
{
    for (size_t i = 0; i < m_CameraTriggersPtrArr.size(); i++)
    {
        if (m_CameraTriggersPtrArr[i] == nullptr)
        {
            m_CameraTriggersPtrArr[i] = cameraTriggerPtr;
            return;
        }
    }
    m_CameraTriggersPtrArr.push_back(cameraTriggerPtr);
    return;
}
void TriggerList::Tick(double deltaTime)
{
    for (size_t i = 0; i < m_CameraTriggersPtrArr.size(); i++)
    {
        if (m_CameraTriggersPtrArr[i] != nullptr)
        {
            m_CameraTriggersPtrArr[i]->Tick(deltaTime);
        }
    }
}

void TriggerList::RemoveAll()
{
    for (size_t i = 0; i < m_CameraTriggersPtrArr.size(); i++)
    {
        if (m_CameraTriggersPtrArr[i] != nullptr)
        {
            delete m_CameraTriggersPtrArr[i];
        }
    }
    m_CameraTriggersPtrArr.clear();
}
void TriggerList::Reset()
{
    for (size_t i = 0; i < m_CameraTriggersPtrArr.size(); i++)
    {
        if (m_CameraTriggersPtrArr[i] != nullptr)
        {
            m_CameraTriggersPtrArr[i]->Reset();
        }
    }
}