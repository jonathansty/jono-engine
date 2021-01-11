//-----------------------------------------------------
// Name: Steyfkens
// First name: Jonathan
// Group: 1DAE01
//-----------------------------------------------------
#include "stdafx.h"		
	
#include "Enemy.h"
#include "Level.h"
#include "Avatar.h"

const double Enemy::GRAVITYCOEFF = 1;
Enemy::Enemy(DOUBLE2 position)
{
    m_Position = position;
    m_Name = String("NaN");
    OutputDebugString(String("Enemy constructor called.\n"));
}
Enemy::Enemy(DOUBLE2 position, Level* levelPtr):
m_Position(position),
m_LevelPtr(levelPtr)
{
    m_Name = String("NaN");
}
Enemy::~Enemy()
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
void Enemy::SetLevel(Level* levelPtr)
{
    m_LevelPtr = levelPtr;
}
void Enemy::SetAvatar(Avatar* avatarPtr)
{
    m_AvatarPtr = avatarPtr;
}
DOUBLE2 Enemy::GetPosition()
{
    return m_Position;
}
int Enemy::distance(Enemy* otherEntityPtr)
{
    DOUBLE2 pos1 = m_ActPtr->GetPosition();
    DOUBLE2 pos2 = otherEntityPtr->GetPosition();
    DOUBLE2 vector = pos2 - pos1;
    return (int)(vector.Length());
}
void Enemy::setGravityScale(double number)
{
    m_GravityScale = number;
}
void Enemy::SetSpawnPosition(DOUBLE2 respawnPosition)
{
    m_RespawnPosition = respawnPosition;
}
bool Enemy::GetAttackByAvatar()
{
    return false;
}
void Enemy::RemoveContactListener()
{
    if (m_ActPtr != nullptr &&  m_ActPtr->GetContactListener() != nullptr)
    {
        m_ActPtr->RemoveContactListener(this);
    }
    
}
void Enemy::setName(String name)
{
    m_Name = name;
}

void Enemy::PaintDebug()
{
    auto engine = GameEngine::instance();
    if (m_ActPtr != nullptr)
    {
        DOUBLE2 position = m_ActPtr->GetPosition();
        engine->set_default_font();
        engine->DrawString(String(m_Name), position);
    }
}

String Enemy::GetName()
{
    return m_Name;
}

bool Enemy::IsHit()
{
    return m_IsHit;
}

void Enemy::Reset()
{

}