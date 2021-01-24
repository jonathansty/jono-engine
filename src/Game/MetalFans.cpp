//-----------------------------------------------------
// Name:
// First name:
// Group: 1DAE.
//-----------------------------------------------------
#include "stdafx.h"		
	
//---------------------------
// Includes
//---------------------------
#include "MetalFans.h"
#include "Avatar.h"

//---------------------------
// Defines
//---------------------------
#define SND_MANAGER (SoundManager::instance())
//---------------------------
// Constructor & Destructor
//---------------------------
MetalFans::MetalFans(float2 position,double angle):
Entity(position), m_Angle(angle)
{
    m_ActPtr = new PhysicsActor(position,angle,BodyType::STATIC);
    m_ActPtr->AddBoxShape(WIDTH,HEIGHT);


    float2 m_Direction = float2(cos(angle - M_PI_2), sin(angle - M_PI_2));
	m_Direction = hlslpp::normalize(m_Direction);
    m_ActTriggerPtr = new PhysicsActor(position+m_Direction*m_TriggerHeight/2, angle, BodyType::STATIC);
    m_ActTriggerPtr->AddBoxShape(WIDTH, m_TriggerHeight - HEIGHT);
    m_ActTriggerPtr->SetName(String("MetalFansTrigger"));
    m_ActTriggerPtr->SetTrigger(true);
    m_ActTriggerPtr->AddContactListener(this);
}

MetalFans::~MetalFans()
{
    delete m_ActPtr;
    m_ActPtr = nullptr;

    m_ActTriggerPtr->RemoveContactListener(this);
    delete m_ActTriggerPtr;
    m_ActTriggerPtr = nullptr;
    
}

//-------------------------------------------------------
// ContactListener overloaded member function definitions
//-------------------------------------------------------
void MetalFans::BeginContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr)
{
    if (actThisPtr == m_ActTriggerPtr && actOtherPtr == m_AvatarPtr->GetActor())
    {
        m_IsHit = true;
    }
    
}

void MetalFans::EndContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr)
{
    if (actThisPtr == m_ActTriggerPtr && actOtherPtr == m_AvatarPtr->GetActor())
    {
        m_IsHit = false;
    }

    
}

void MetalFans::ContactImpulse(PhysicsActor *actThisPtr, double impulse)
{

}
void MetalFans::Paint(graphics::D2DRenderContext& ctx)
{
    float3x3 matTranslate, matRotate, matPivot,matWorldTransform;
    matPivot= float3x3::translation(float2(-WIDTH / 2, -HEIGHT / 2));
    matTranslate= float3x3::translation(m_Position);
    matRotate = float3x3::rotation_z(m_Angle);
    matWorldTransform = matPivot * matRotate * matTranslate;
    ctx.set_world_matrix(matWorldTransform);
    
    ctx.fill_rect(0, 0, WIDTH, HEIGHT);
    ctx.set_color(COLOR(0, 0, 120));
    ctx.fill_rect(0, 0, WIDTH, HEIGHT / 2);
    ctx.set_world_matrix(float3x3::identity());
    
}
void MetalFans::Tick(double deltaTime)
{
    if (m_IsHit == true)
    {
        float3x3 matRotate;
        matRotate = float3x3::rotation_z(m_Angle);
        float2 avatarPosition = m_AvatarPtr->GetActor()->GetPosition();
        //avatarPosition = matRotate.TransformPoint(avatarPosition);
        float2 actorPosition = m_ActPtr->GetPosition();
        //actorPosition = matRotate.TransformPoint(actorPosition);
        double distance = abs(avatarPosition.y - actorPosition.y);
		float2 force;
		float2 friction;
		PhysicsActor* actAvatarPtr = m_AvatarPtr->GetActor();
        if (distance > 30)
        {
            force = float2(0, -m_RepulsionForce * PhysicsActor::SCALE * actAvatarPtr->GetMass() / distance);
            friction = float2(0, -m_FrictionForce * actAvatarPtr->GetLinearVelocity().y);
        }
        else
        {
            force = float2(0, -m_RepulsionForce * actAvatarPtr->GetMass() /30);
            friction = float2(0, -15 * actAvatarPtr->GetLinearVelocity().y);
        }

		force = hlslpp::mul(matRotate, float3(force, 0.0f)).xy;
		friction = hlslpp::mul(matRotate, float3(friction, 0.0f)).xy;
		actAvatarPtr->ApplyForce(force);
		actAvatarPtr->ApplyForce(friction);

        
    }
}
void MetalFans::Reset()
{

}
void MetalFans::SetFrictionForce(double force)
{
    m_FrictionForce = force;
}
void MetalFans::SetRepulsionForce(double force)
{
    m_RepulsionForce = force;
}