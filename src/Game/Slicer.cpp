#include "stdafx.h"		
	
#include "Slicer.h"
#include "Avatar.h"

Slicer::Slicer(float2 position,float2 barPoint,int radius): Enemy(position),
m_BarPosition(barPoint), m_Radius(radius)
{
    m_ActPtr = new PhysicsActor(position, 0, BodyType::DYNAMIC);
    m_ActPtr->SetGravityScale(1);
    m_ActPtr->AddBoxShape(BLADEWIDTH, BLADEHEIGHT,0,0);
    m_ActPtr->SetAngularVelocity(5);
    m_ActPtr->SetName(String("Slicer"));
    m_ActPtr->SetTrigger(true);
    m_ActPtr->AddBoxShape(BLADEHEIGHT, BLADEWIDTH, 0, 0);
    m_ActBarPoint = new PhysicsActor(barPoint, 0, BodyType::STATIC);
    m_ActBarPoint->AddBoxShape(10, 10);
    m_ActBarPoint->SetGhost(true);
    
    m_ActBarPtr = new PhysicsActor(position, 0, BodyType::DYNAMIC);
    m_ActBarPtr->AddBoxShape(m_Radius*2, 10);
    m_ActBarPtr->SetTrigger(true);
    m_JntPtr = new PhysicsRevoluteJoint(m_ActBarPoint, float2(), m_ActBarPtr, float2(-m_Radius, 0));
    m_JntBarPlatform = new PhysicsRevoluteJoint(m_ActBarPtr, float2(m_Radius, 0), m_ActPtr, float2());
    m_JntPtr->EnableMotor(true, 10, 2);
   

}

Slicer::~Slicer()
{
    delete m_JntPtr;
    m_JntPtr = nullptr;

    delete m_JntBarPlatform;
    m_JntBarPlatform = nullptr;

    delete m_ActBarPoint;
    m_ActBarPoint = nullptr; 
    
    delete m_ActBarPtr;
    m_ActBarPtr = nullptr;
    
    
}

//-------------------------------------------------------
// ContactListener overloaded member function definitions
//-------------------------------------------------------
void Slicer::BeginContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr)
{

}

void Slicer::EndContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr)
{

}

void Slicer::ContactImpulse(PhysicsActor *actThisPtr, double impulse)
{

}
void Slicer::Tick(double deltaTime)
{
    float2 direction = m_ActBarPoint->GetPosition() - m_ActPtr->GetPosition();
    m_Position = m_ActPtr->GetPosition();
}
void Slicer::Paint(graphics::D2DRenderContext& ctx)
{
    ctx.draw_line(m_ActBarPoint->GetPosition(), m_ActPtr->GetPosition());

    float3x3 matTranlate, matRotate, matPivot;
    matTranlate= float3x3::translation(m_Position);
    matRotate = float3x3::rotation_z(m_ActPtr->GetAngle());
    matPivot= float3x3::translation(float2(-BLADEWIDTH / 2, -BLADEHEIGHT / 2));
    ctx.set_world_matrix(matPivot * matRotate * matTranlate);
    ctx.set_color(COLOR(0, 0, 0));
    ctx.fill_rect(0, 0, BLADEWIDTH, BLADEHEIGHT);

    matRotate = float3x3::rotation_z(m_ActPtr->GetAngle() + M_PI_2);
    ctx.set_world_matrix(matPivot * matRotate * matTranlate);
    ctx.fill_rect(0, 0, BLADEWIDTH, BLADEHEIGHT);
    ctx.set_world_matrix(float3x3::identity());
}
PhysicsActor* Slicer::GetActor()
{
    return m_ActPtr;
}
