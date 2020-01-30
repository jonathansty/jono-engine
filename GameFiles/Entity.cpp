//-----------------------------------------------------
// Name: Steyfkens
// First name: Jonathan
// Group: 1DAE01
//-----------------------------------------------------
#include "stdafx.h"		
	
#include "Entity.h"
#include "Level.h"
#include "SoundManager.h"

#define GAME_ENGINE (GameEngine::GetSingleton())
#define SND_MANAGER (SoundManager::GetSingleton())

const double Entity::GRAVITYCOEFF = 1;
Entity::Entity(DOUBLE2 position)
{
    m_Position = position;
	// nothing to create
	// m_ActCirclePtr->AddContactListener(this);
    OutputDebugString(String("Entity constructor called.\n"));
}
Entity::Entity(DOUBLE2 position, Level* levelPtr):
m_Position(position),
m_LevelPtr(levelPtr)
{
}
Entity::~Entity()
{
    /**
    * Safety measure for removing contactListeners so errors can be avoided. Hopefully...
    */
    if (m_ActPtr != nullptr && m_ActPtr->GetContactListener() != nullptr)
    {
        m_ActPtr->RemoveContactListener(m_ActPtr->GetContactListener());
    }
    if (m_ActPtr != nullptr)
    {
        delete m_ActPtr;
        m_ActPtr = nullptr;
    }

}

//-------------------------------------------------------
// ContactListener overloaded member function definitions
//-------------------------------------------------------
void Entity::SetLevel(Level* levelPtr)
{
    m_LevelPtr = levelPtr;
}
DOUBLE2 Entity::GetPosition()
{
    if (m_ActPtr != nullptr)
    {
        return m_ActPtr->GetPosition();
    }
    else
    {
        return m_Position;
    }
    
}
int Entity::distance(Entity* otherEntityPtr)
{
    DOUBLE2 pos1 = m_ActPtr->GetPosition();
    DOUBLE2 pos2 = otherEntityPtr->GetPosition();
    DOUBLE2 vector = pos2 - pos1;
    return (int)(vector.Length());
}
void Entity::SetGravityScale(double number)
{
    m_GravityScale = number;
}
void Entity::SetSpawnPosition(DOUBLE2 respawnPosition)
{
    m_RespawnPosition = respawnPosition;
}
bool Entity::isHit()
{
    return m_IsHit;
}
void Entity::SetAvatar(Avatar* avatarPtr)
{
    m_AvatarPtr = avatarPtr;
}
PhysicsActor* Entity::GetActor()
{
    return m_ActPtr;
}
void Entity::SetName(String name)
{
    m_Name = name;
}
String Entity::GetName()
{
    return m_Name;
}
bool Entity::GetIsDead()
{
    return m_IsDead;
}
void Entity::PaintDebug()
{
    MATRIX3X2 matTranslate;
    matTranslate.SetAsTranslate(m_Position);
    GAME_ENGINE->SetWorldMatrix(matTranslate);
    GAME_ENGINE->DrawString(m_Name,DOUBLE2());
    GAME_ENGINE->SetWorldMatrix(MATRIX3X2::CreateIdentityMatrix());
}