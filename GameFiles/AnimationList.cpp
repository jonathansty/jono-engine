//-----------------------------------------------------
// Name: Steyfkens
// First name: Jonathan
// Group: 1DAE01
//-----------------------------------------------------
#include "stdafx.h"		
	
//---------------------------
// Includes
//---------------------------
#include "AnimationList.h"

//---------------------------
// Defines
//---------------------------
#define GAME_ENGINE (GameEngine::GetSingleton())

//---------------------------
// Constructor & Destructor
//---------------------------
AnimationList::AnimationList()
{
	// nothing to create
	// m_ActCirclePtr->AddContactListener(this);
}

AnimationList::~AnimationList()
{
    RemoveAll();
}

//-------------------------------------------------------
// ContactListener overloaded member function definitions
//-------------------------------------------------------
//void AnimationList::BeginContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr)
//{
//
//}
//
//void AnimationList::EndContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr)
//{
//
//}
//
//void AnimationList::ContactImpulse(PhysicsActor *actThisPtr, double impulse)
//{
//
//}
void AnimationList::Paint()
{
    for (size_t i = 0, n = m_AnimationsPtrArr.size(); i <n; i++)
    {
        if (m_AnimationsPtrArr[i] != nullptr)
        {
            m_AnimationsPtrArr[i]->Paint();
            
        }
    }
}
void AnimationList::Tick(double deltaTime)
{
    for (size_t i = 0, n = m_AnimationsPtrArr.size(); i <n; i++)
    {
        if (m_AnimationsPtrArr[i] != nullptr)
        {
            m_AnimationsPtrArr[i]->Tick(deltaTime);
            if (m_AnimationsPtrArr[i]->IsEnded())
            {
                Remove(m_AnimationsPtrArr[i]);
            }
        }
    }
}

void AnimationList::Add(Animation* tmpAnimationPtr)
{
    for (size_t i = 0, n = m_AnimationsPtrArr.size(); i <n; i++)
    {
        if (m_AnimationsPtrArr[i] == nullptr)
        {
            m_AnimationsPtrArr[i] = tmpAnimationPtr;
            return;
        }
    }
    m_AnimationsPtrArr.push_back(tmpAnimationPtr);
}
void AnimationList::Remove(Animation* tmpAnimationPtr)
{
    for (size_t i = 0, n = m_AnimationsPtrArr.size(); i <n; i++)
    {
        if (tmpAnimationPtr != nullptr && m_AnimationsPtrArr[i] == tmpAnimationPtr)
        {
            delete m_AnimationsPtrArr[i];
            m_AnimationsPtrArr[i] = nullptr;
        }
    }
}
void AnimationList::RemoveAll()
{
    for (size_t i = 0, n = m_AnimationsPtrArr.size(); i <n; i++)
    {
        if (m_AnimationsPtrArr[i] != nullptr)
        {
            delete m_AnimationsPtrArr[i];
            m_AnimationsPtrArr[i] = nullptr;
        }
    }
    m_AnimationsPtrArr.clear();
}


