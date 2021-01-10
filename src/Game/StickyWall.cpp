#include "stdafx.h"		
	
#include "StickyWall.h"
#include "Avatar.h"

StickyWall::StickyWall(DOUBLE2 position, int width, int height):
Entity(position),
m_Width(width),
m_Height(height)
{
    m_ActPtr = new PhysicsActor(position, 0, BodyType::STATIC);
    m_ActPtr->AddBoxShape(width, height,0);
    m_ActPtr->SetName(String("StickyWall"));
    m_ActPtr->AddContactListener(this);
}

StickyWall::~StickyWall()
{
	
}

//-------------------------------------------------------
// ContactListener overloaded member function definitions
//-------------------------------------------------------
void StickyWall::BeginContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr)
{
    if (actOtherPtr == m_AvatarPtr->GetActor() )
    {
        m_AvatarPtr->GetActor()->SetFriction(0);
        m_IsHit = true;
    }
}

void StickyWall::EndContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr)
{
    if (actOtherPtr == m_AvatarPtr->GetActor())
    {
        m_AvatarPtr->GetActor()->SetFriction(0);
        
        m_IsHit = false;
        m_AvatarPtr->SetNrJumps(1);
    }
}

void StickyWall::ContactImpulse(PhysicsActor *actThisPtr, double impulse)
{

}
void StickyWall::Paint()
{
    game_engine::instance()->set_color(COLOR(0, 0, 0));
    MATRIX3X2 matTranslate, matPivot;
    matTranslate.SetAsTranslate(m_Position);
    matPivot.SetAsTranslate(DOUBLE2(-m_Width / 2, -m_Height / 2));
    game_engine::instance()->set_world_matrix(matPivot*matTranslate);
    game_engine::instance()->FillRect(0, 0, (int)m_Width, (int)m_Height);
    game_engine::instance()->set_color(COLOR(0, 125, 50));
    game_engine::instance()->FillRect(0, 0, (int)(m_Width * 0.2), (int)(m_Height));
    game_engine::instance()->FillRect((int)(m_Width - (m_Width*0.2)), 0, (int)m_Width, (int)m_Height);
    game_engine::instance()->set_color(COLOR(0, 0, 0));
    game_engine::instance()->set_world_matrix(MATRIX3X2::CreateIdentityMatrix());
}
void StickyWall::Tick(double deltaTime)
{
    PhysicsActor* tmpActAvatarPtr = m_AvatarPtr->GetActor();
    DOUBLE2 velocity = tmpActAvatarPtr->GetLinearVelocity();
    m_Position = m_ActPtr->GetPosition();
    if (m_IsHit)
    {
        m_AvatarPtr->SetNrJumps(0);
        if (m_AvatarPtr->GetMoveState() != Avatar::moveState::JUMPING && velocity.y > 0)
        {
            tmpActAvatarPtr->ApplyLinearImpulse(DOUBLE2(0,-velocity.y*tmpActAvatarPtr->GetMass()/PhysicsActor::SCALE));
        }
    }
}
void StickyWall::Reset()
{

}

