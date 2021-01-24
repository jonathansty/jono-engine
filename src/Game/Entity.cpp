#include "stdafx.h"		
	
#include "Entity.h"
#include "Level.h"
#include "SoundManager.h"

const double Entity::GRAVITYCOEFF = 1;

Entity::Entity(float2 position)
    : m_Position(position)
{
	// nothing to create
	// m_ActCirclePtr->AddContactListener(this);
    OutputDebugString(String("Entity constructor called.\n"));
}
Entity::Entity(float2 position, Level* levelPtr):
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
float2 Entity::GetPosition() const
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
    float2 pos1 = m_ActPtr->GetPosition();
    float2 pos2 = otherEntityPtr->GetPosition();
    float2 vector = pos2 - pos1;
	return hlslpp::length(vector);
}
void Entity::SetGravityScale(double number)
{
    m_GravityScale = number;
}
void Entity::SetSpawnPosition(float2 respawnPosition)
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
void Entity::PaintDebug(graphics::D2DRenderContext& ctx)
{
    float3x3 matTranslate;
    matTranslate= float3x3::translation(m_Position);
    ctx.set_world_matrix(matTranslate);
    ctx.draw_string(m_Name,float2());
    ctx.set_world_matrix(float3x3::identity());
}