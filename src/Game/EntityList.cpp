#include "stdafx.h"		
	
#include "EntityList.h"
#include "Entity.h"

EntityList::EntityList()
{
	// nothing to create
	// m_ActCirclePtr->AddContactListener(this);
}

EntityList::~EntityList()
{
    RemoveAll();
}

//-------------------------------------------------------
// ContactListener overloaded member function definitions
//-------------------------------------------------------
//void EntityList::BeginContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr)
//{
//
//}
//
//void EntityList::EndContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr)
//{
//
//}
//
//void EntityList::ContactImpulse(PhysicsActor *actThisPtr, double impulse)
//{
//
//}
void EntityList::Paint(graphics::D2DRenderContext& ctx)
{
    for (size_t i = 0; i < m_EntityPtrArr.size(); i++)
    {
        if (m_EntityPtrArr[i] != nullptr)
        {
            m_EntityPtrArr[i]->Paint(ctx);
        }
    }
}
void EntityList::PaintDebug(graphics::D2DRenderContext& ctx)
{
    for (size_t i = 0; i < m_EntityPtrArr.size(); i++)
    {
        if (m_EntityPtrArr[i] != nullptr)
        {
            m_EntityPtrArr[i]->PaintDebug(ctx);
        }
    }
}
void EntityList::Tick(double deltaTime)
{
    for (size_t i = 0; i < m_EntityPtrArr.size(); i++)
    {
        if (m_EntityPtrArr[i] != nullptr)
        {
            m_EntityPtrArr[i]->Tick(deltaTime);
            if (m_EntityPtrArr[i]->GetIsDead())
            {
                delete m_EntityPtrArr[i];
                m_EntityPtrArr[i] = nullptr;
            }
        }
    }
}
void EntityList::Add(Entity* tmpEntityPtr)
{
    for (size_t i = 0; i < m_EntityPtrArr.size(); i++)
    {
        if (m_EntityPtrArr[i] == nullptr)
        {
            m_EntityPtrArr[i] = tmpEntityPtr;
            return;
        }
    }
    m_EntityPtrArr.push_back(tmpEntityPtr);
}
void EntityList::Remove(Entity* tmpEntityPtr)
{
    for (size_t i = 0; i < m_EntityPtrArr.size(); i++)
    {
        if (m_EntityPtrArr[i] == tmpEntityPtr)
        {
            delete m_EntityPtrArr[i];
            m_EntityPtrArr[i] = nullptr;
        }
    }
}
void EntityList::RemoveAll()
{
    for (size_t i = 0; i < m_EntityPtrArr.size(); i++)
    {
        if (m_EntityPtrArr[i] != nullptr)
        {
            delete m_EntityPtrArr[i];
        }      
    }
    m_EntityPtrArr.clear();
}
Entity* EntityList::isHit()
{
    for (size_t i = 0; i < m_EntityPtrArr.size(); i++)
    {
        if (m_EntityPtrArr[i] != nullptr)
        {
            if (m_EntityPtrArr[i]->isHit())
            {
                return m_EntityPtrArr[i];
            }
        }
    }
    return nullptr;
}
Entity* EntityList::IsDead()
{
    for (size_t i = 0; i < m_EntityPtrArr.size(); i++)
    {
        if (m_EntityPtrArr[i] != nullptr)
        {
            if (m_EntityPtrArr[i]->GetIsDead())
            {
                return m_EntityPtrArr[i];
            }
        }
    }
    return nullptr;
}
void EntityList::Reset()
{
    for (size_t i = 0; i < m_EntityPtrArr.size(); i++)
    {
        if (m_EntityPtrArr[i] != nullptr)
        {
            m_EntityPtrArr[i]->Reset();
        }
    }
}
