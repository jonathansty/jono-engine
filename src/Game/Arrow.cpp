#include "stdafx.h"		
	
#include "Arrow.h"
#include "SoundManager.h"
#include "Avatar.h"
#include "Level.h"

int Arrow::m_InstanceCounter = 0;
Arrow::Arrow(float2 position, Bitmap* bmpPtr):
Entity(position),
m_BmpPtr(bmpPtr)
{
    m_InstanceCounter++;

    m_ActPtr = new PhysicsActor(position+ float2(0,10), 0, BodyType::DYNAMIC);
    b2Filter collisionFilter;
    collisionFilter.groupIndex = -5;
    m_ActPtr->AddBoxShape(m_BmpPtr->GetWidth() - 20,m_BmpPtr->GetHeight() - 15);
    m_ActPtr->SetName(String("Arrow"));
    m_ActPtr->AddContactListener(this);
    m_ActPtr->SetGravityScale(0);
    m_ActPtr->SetBullet(true);
    m_ActPtr->SetCollisionFilter(collisionFilter);
    //GameEngine::instance()->ConsolePrintString(m_Position.ToString());

    m_ActBottomTriggerPtr = new PhysicsActor(position + float2(0, 40), 0, BodyType::DYNAMIC);
    //m_ActBottomTriggerPtr->AddBoxShape(m_BmpPtr->GetWidth() / 2, 20);
    std::vector<float2>tmpPointArr;
    double halfBmpWidth = m_BmpPtr->GetWidth() / 2;
    double halfBmpHeight = m_BmpPtr->GetHeight() / 2;
    
    m_ActBottomTriggerPtr->AddBoxShape(halfBmpWidth + 50, halfBmpHeight);
    m_ActBottomTriggerPtr->SetTrigger(true);
    m_ActBottomTriggerPtr->SetGravityScale(0);
    m_ActBottomTriggerPtr->AddContactListener(this);
    m_ActBottomTriggerPtr->SetCollisionFilter(collisionFilter);

    m_SndJumpPtr = SoundManager::instance()->LoadSound(String("Resources/Sound/Entity/Jump2.wav"));
    m_SndJumpPtr->set_volume(0.2);
}

Arrow::~Arrow()
{
    m_ActBottomTriggerPtr->RemoveContactListener(this);
    delete m_ActBottomTriggerPtr;
    m_ActBottomTriggerPtr = nullptr;

    m_ActPtr->RemoveContactListener(this);
    delete m_ActPtr;
    m_ActPtr = nullptr;

}

//-------------------------------------------------------
// ContactListener overloaded member function definitions
//-------------------------------------------------------
void Arrow::BeginContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr)
{
    // We want to apply the force on our avatar and not our avatargroundtrigger
    if ((actOtherPtr->GetName() == String("Avatar") || actOtherPtr->GetName() == String("Level")) && actThisPtr == m_ActPtr )
    {
        m_IsHit = true;
        m_IsDead = true;
        float2 velocity = actOtherPtr->GetLinearVelocity();
        double mass = actOtherPtr->GetMass();

        actOtherPtr->ApplyLinearImpulse(float2(0, ( (- m_PushPower -velocity.y) * mass / PhysicsActor::SCALE)));
    }

    if (actThisPtr == m_ActBottomTriggerPtr)
    {
        m_ActPtr->SetTrigger(true);
    }
}

void Arrow::EndContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr)
{
    if (actThisPtr == m_ActPtr)
    {
        m_ActPtr->SetTrigger(false);
    }
}

void Arrow::ContactImpulse(PhysicsActor *actThisPtr, double impulse)
{

}
void Arrow::Paint(graphics::D2DRenderContext& ctx) 
{
    float3x3 matWorldTransform = float3x3( float3(1,0, 0), float3(0, 1, 0), float3(m_Position - float2(m_BmpPtr->GetWidth()/2,m_BmpPtr->GetHeight()/2), 1.0f) );
    ctx.set_world_matrix(matWorldTransform);
    ctx.draw_bitmap(m_BmpPtr);
    ctx.set_world_matrix(float3x3::identity());
}
void Arrow::Tick(double deltaTime)
{
    if (m_IsDead)
    {
        m_ActPtr->RemoveContactListener(this);
        m_ActBottomTriggerPtr->RemoveContactListener(this);
    }
    m_Position = m_ActPtr->GetPosition();
    m_ActBottomTriggerPtr->SetPosition(m_Position + float2(0, 40));
}
PhysicsActor* Arrow::GetActor()
{
    return m_ActPtr;
}
void Arrow::SetPushPower(double pushPower)
{
    m_PushPower = pushPower;
}

