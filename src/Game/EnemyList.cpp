#include "game.stdafx.h"		
	
#include "EnemyList.h"
#include "Enemy.h"
#include "Avatar.h"
#include "EnemyRocketLauncher.h"

EnemyList::EnemyList()
{
	// nothing to create
	// m_ActCirclePtr->AddContactListener(this);
}

EnemyList::~EnemyList()
{
    RemoveAll();
}

//-------------------------------------------------------
// ContactListener overloaded member function definitions
//-------------------------------------------------------
//void EnemyList::BeginContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr)
//{
//
//}
//
//void EnemyList::EndContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr)
//{
//
//}
//
//void EnemyList::ContactImpulse(PhysicsActor *actThisPtr, double impulse)
//{
//
//}
void EnemyList::Paint(graphics::D2DRenderContext& ctx)
{
    for (size_t i = 0; i < m_EnemiesPtrArr.size(); i++)
    {
        if (m_EnemiesPtrArr[i] != nullptr)
        {
            m_EnemiesPtrArr[i]->Paint(ctx);
        }
    }
}
void EnemyList::PaintRockets(graphics::D2DRenderContext& ctx)
{
    for (size_t i = 0; i < m_EnemiesPtrArr.size(); i++)
    {
        if (m_EnemiesPtrArr[i] != nullptr && m_EnemiesPtrArr[i]->GetActor() != nullptr && m_EnemiesPtrArr[i]->GetActor()->GetName() == String("EnemyRocketLauncher"))
        {
            EnemyRocketLauncher* tmpRocketLauncher = reinterpret_cast<EnemyRocketLauncher*>(m_EnemiesPtrArr[i]);
            tmpRocketLauncher->PaintRockets(ctx);
        }
    }
}
void EnemyList::Tick(double deltaTime)
{
    if (m_IsRemoveReset)
    {
        // Always delete in the tick
        // First delete contactlisteners of the enemies
        for (size_t i = 0; i < m_EnemiesPtrArr.size(); i++)
        {
            if (m_EnemiesPtrArr[i] != nullptr)
            {
                m_EnemiesPtrArr[i]->RemoveContactListener();
            }
        }
        // Then delete all the enemies seperately. For some reason it doesn't work when you just remove the contact listener in the destructor
        RemoveAll();
        m_IsRemoveReset = false;
    }
    std::vector<Enemy*>tmpEnemyPtrArr;
    for (size_t i = 0; i < m_EnemiesPtrArr.size(); i++)
    {
        if (m_EnemiesPtrArr[i] != nullptr)
        {
            m_EnemiesPtrArr[i]->Tick(deltaTime);
        }
    }
    /*for (size_t i = 0; i < m_EnemiesPtrArr.size(); i++)
    {
        if (m_EnemiesPtrArr[i] != nullptr && m_EnemiesPtrArr[i]->GetAttackByAvatar() && m_AvatarPtr->GetMoveState() == Avatar::moveState::ATTACK)
        {
            tmpEnemyPtrArr.push_back(m_EnemiesPtrArr[i]);
        }
    }
    for (size_t i = 0; i < tmpEnemyPtrArr.size(); i++)
    {
        tmpEnemyPtrArr[i]->RemoveContactListener();
        Remove(tmpEnemyPtrArr[i]);
    }*/
    


}
void EnemyList::Add(Enemy* tmpEnemyPtr)
{
    m_NumberOfEnemies++;
    for (size_t i = 0; i < m_EnemiesPtrArr.size(); i++)
    {
        if (m_EnemiesPtrArr[i] == nullptr)
        {
            m_EnemiesPtrArr[i] = tmpEnemyPtr;
            return;
        }
    }
    m_EnemiesPtrArr.push_back(tmpEnemyPtr);
    
}
void EnemyList::Remove(Enemy* tmpEnemyPtr)
{
    m_NumberOfEnemies--;
    for (size_t i = 0; i < m_EnemiesPtrArr.size(); i++)
    {
        if (m_EnemiesPtrArr[i] == tmpEnemyPtr)
        {
            m_EnemiesPtrArr[i]->RemoveContactListener();
            delete m_EnemiesPtrArr[i];
            m_EnemiesPtrArr[i] = nullptr;
        }
    }
}
void EnemyList::RemoveAll()
{
    m_NumberOfEnemies = 0;
    for (size_t i = 0; i < m_EnemiesPtrArr.size(); i++)
    {
        if (m_EnemiesPtrArr[i] != nullptr)
        {
            
            delete m_EnemiesPtrArr[i];
        }
        
    }
    m_EnemiesPtrArr.clear();
}
Enemy* EnemyList::IsAttackedByAvatar()
{
    for (size_t i = 0; i < m_EnemiesPtrArr.size(); i++)
    {
        if (m_EnemiesPtrArr[i] != nullptr)
        {
            if (m_EnemiesPtrArr[i]->GetAttackByAvatar())
            {
                return m_EnemiesPtrArr[i];
            }
        }
    }
    return nullptr;
}
void EnemyList::SetAvatar(Avatar* avatarPtr)
{
    m_AvatarPtr = avatarPtr;
}
void EnemyList::Reset()
{
    for (size_t i = 0; i < m_EnemiesPtrArr.size(); i++)
    {
        if (m_EnemiesPtrArr[i] != nullptr)
        {
            m_EnemiesPtrArr[i]->Reset();
        }
    }
}
void EnemyList::RemoveContactListeners()
{
    for (size_t i = 0; i < m_EnemiesPtrArr.size(); i++)
    {
        if (m_EnemiesPtrArr[i] != nullptr && m_EnemiesPtrArr[i]->GetActor()->GetContactListener())
        {
            m_EnemiesPtrArr[i]->RemoveContactListener();
        }
    }
}
int EnemyList::GetSize() const
{
    return m_NumberOfEnemies;
}
void EnemyList::PaintDebug(graphics::D2DRenderContext& ctx)
{
    for (size_t i = 0; i < m_EnemiesPtrArr.size(); i++)
    {
        if (m_EnemiesPtrArr[i] != nullptr)
        {
            m_EnemiesPtrArr[i]->PaintDebug(ctx);
        }
    }
}

Enemy* EnemyList::IsHit()
{
    for (size_t i = 0; i < m_EnemiesPtrArr.size(); i++)
    {
        if (m_EnemiesPtrArr[i] != nullptr)
        {
            if (m_EnemiesPtrArr[i] != nullptr && m_EnemiesPtrArr[i]->GetAttackByAvatar() && m_AvatarPtr->GetMoveState() == Avatar::moveState::ATTACK)
            {
                return m_EnemiesPtrArr[i];
            }
        }
    }
    return nullptr;
}

void EnemyList::SafeRemoveAll()
{
    m_IsRemoveReset = true;
}