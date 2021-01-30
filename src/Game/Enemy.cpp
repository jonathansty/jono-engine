//-----------------------------------------------------
// Name: Steyfkens
// First name: Jonathan
// Group: 1DAE01
//-----------------------------------------------------
#include "game.stdafx.h"		
	
#include "Enemy.h"
#include "Level.h"
#include "Avatar.h"

const double Enemy::GRAVITYCOEFF = 1;
Enemy::Enemy(float2 position)
{
    m_Position = position;
    m_Name = String("NaN");
    OutputDebugString(String("Enemy constructor called.\n"));
}
Enemy::Enemy(float2 position, Level* levelPtr):
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
float2 Enemy::GetPosition() const
{
    return m_Position;
}
int Enemy::distance(Enemy* otherEntityPtr)
{
    float2 pos1 = m_ActPtr->GetPosition();
    float2 pos2 = otherEntityPtr->GetPosition();
    float2 vector = pos2 - pos1;
	return hlslpp::length(vector);
}
void Enemy::setGravityScale(double number)
{
    m_GravityScale = number;
}
void Enemy::SetSpawnPosition(float2 respawnPosition)
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

void Enemy::PaintDebug(graphics::D2DRenderContext& ctx)
{
    auto engine = GameEngine::instance();
    if (m_ActPtr != nullptr)
    {
        float2 position = m_ActPtr->GetPosition();
        ctx.set_font(nullptr);
        ctx.draw_string(String(m_Name), position);
    }
}

String Enemy::GetName() const
{
    return m_Name;
}

bool Enemy::IsHit() const
{
    return m_IsHit;
}

void Enemy::Reset()
{

}