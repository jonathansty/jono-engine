#include "stdafx.h"		
	
#include "Gate.h"
#include "Avatar.h"

Gate::Gate(float2 position, float2 triggerposition):
Entity(position), m_TriggerPosition(triggerposition)
{
    m_ActPtr = new PhysicsActor(triggerposition, 0, BodyType::STATIC);
    m_ActPtr->AddBoxShape(50, 50);
    m_ActPtr->SetTrigger(true);
    m_ActPtr->AddContactListener(this);
    
    m_ActBasePtr = new PhysicsActor(position, 0, BodyType::STATIC);
    m_ActBasePtr->AddBoxShape(10, 10);
    m_ActBasePtr->SetGhost(true);
    m_ActGatePtr = new PhysicsActor(position - float2(0,GameEngine::instance()->get_height()/2), 0, BodyType::DYNAMIC);
    m_ActGatePtr->SetGravityScale(1);
    b2Filter filter;
    filter.groupIndex = -5;
    m_ActGatePtr->SetCollisionFilter(filter);
    m_Width = 50;
    m_Height = GameEngine::instance()->get_height() + 800;
    m_ActGatePtr->AddBoxShape(m_Width,m_Height);
    m_JntGatePtr = new PhysicsPrismaticJoint(m_ActBasePtr, float2(), m_ActGatePtr, float2(0,(GameEngine::instance()->get_height()+800)/2 ), float2(0, 1), false);
    m_JntGatePtr->EnableJointLimits(true, -GameEngine::instance()->get_height(),0 );
}

Gate::~Gate()
{
    m_ActPtr->RemoveContactListener(this);
    delete m_ActPtr;
    m_ActPtr = nullptr;
    delete m_JntGatePtr;
    m_JntGatePtr = nullptr;
    delete m_ActBasePtr;
    m_ActBasePtr = nullptr;
    delete m_ActGatePtr;
    m_ActGatePtr = nullptr;
    

}

//-------------------------------------------------------
// ContactListener overloaded member function definitions
//-------------------------------------------------------
void Gate::BeginContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr)
{
    if (actThisPtr == m_ActPtr && actOtherPtr == m_AvatarPtr->GetActor())
    {
        m_JntGatePtr->EnableMotor(true, -500, 200);
    }
}

void Gate::EndContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr)
{

}

void Gate::ContactImpulse(PhysicsActor *actThisPtr, double impulse)
{

}
void Gate::Tick(double deltaTime)
{

}
void Gate::Paint(graphics::D2DRenderContext& ctx)
{
    float3x3 matTranslate, matPivot;
    float2 position = m_ActGatePtr->GetPosition();
    matTranslate= float3x3::translation(position);
    matPivot= float3x3::translation(float2(-m_Width / 2, -m_Height / 2));
    ctx.set_world_matrix(matPivot * matTranslate);
    ctx.set_color(COLOR(0, 0, 0, 255));
    ctx.fill_rect(0, 0, m_Width, m_Height);
    ctx.set_world_matrix(float3x3::identity());
}
void Gate::Reset()
{
    m_JntGatePtr->EnableMotor(true, 600, 600);
}
