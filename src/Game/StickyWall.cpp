#include "game.stdafx.h"		
	
#include "StickyWall.h"
#include "Avatar.h"

StickyWall::StickyWall(float2 position, int width, int height):
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
void StickyWall::Paint(graphics::D2DRenderContext& ctx)
{
    ctx.set_color(COLOR(0, 0, 0));
    float3x3 matTranslate, matPivot;
    matTranslate= float3x3::translation(m_Position);
    matPivot= float3x3::translation(float2(-m_Width / 2, -m_Height / 2));
    ctx.set_world_matrix(matPivot*matTranslate);
    ctx.fill_rect(0, 0, (int)m_Width, (int)m_Height);
    ctx.set_color(COLOR(0, 125, 50));
    ctx.fill_rect(0, 0, (int)(m_Width * 0.2), (int)(m_Height));
    ctx.fill_rect((int)(m_Width - (m_Width*0.2)), 0, (int)m_Width, (int)m_Height);
    ctx.set_color(COLOR(0, 0, 0));
    ctx.set_world_matrix(float3x3::identity());
}
void StickyWall::Tick(double deltaTime)
{
    PhysicsActor* tmpActAvatarPtr = m_AvatarPtr->GetActor();
    float2 velocity = tmpActAvatarPtr->GetLinearVelocity();
    m_Position = m_ActPtr->GetPosition();
    if (m_IsHit)
    {
        m_AvatarPtr->SetNrJumps(0);
        if (m_AvatarPtr->GetMoveState() != Avatar::moveState::JUMPING && velocity.y > 0)
        {
            tmpActAvatarPtr->ApplyLinearImpulse(float2(0,-velocity.y*tmpActAvatarPtr->GetMass()/PhysicsActor::SCALE));
        }
    }
}
void StickyWall::Reset()
{

}

