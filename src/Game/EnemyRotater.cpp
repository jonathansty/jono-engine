//-----------------------------------------------------
// Name: Steyfkens
// First name: Jonathan
// Group: 1DAE01
//-----------------------------------------------------
#include "game.stdafx.h"		
	
//---------------------------
// Includes
//---------------------------
#include "EnemyRotater.h"

//---------------------------
// Constructor & Destructor
//---------------------------
EnemyRotater::EnemyRotater(float2 position, Bitmap* bmpPtr): Enemy(position), m_BmpPtr(bmpPtr)
{
    m_ActPtr = new PhysicsActor(position, 0, BodyType::KINEMATIC);
    m_ActPtr->AddCircleShape(bmpPtr->GetWidth()/2,float2(m_Radius,0),0,0);
    m_ActPtr->SetName(String("EnemyRotater"));
}

EnemyRotater::~EnemyRotater()
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
void EnemyRotater::BeginContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr)
{

}

void EnemyRotater::EndContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr)
{

}

void EnemyRotater::ContactImpulse(PhysicsActor *actThisPtr, double impulse)
{

}
void EnemyRotater::Paint(graphics::D2DRenderContext& ctx)
{
    float2 position = m_ActPtr->GetPosition();
    float3x3 matRotation, matOrbitRadius, matOrbitCenter, matCenter,matBitmapRotation;
    matCenter = float3x3::translation(float2(-m_BmpPtr->GetWidth() / 2, -m_BmpPtr->GetHeight() / 2));
    matRotation = float3x3::rotation_z(m_Angle);
    matOrbitRadius = float3x3::translation(float2(m_Radius,0));
    matOrbitCenter = float3x3::translation(position);
    matBitmapRotation = hlslpp::inverse(matRotation);
    ctx.set_world_matrix(matCenter *matBitmapRotation * matOrbitRadius*matRotation*matOrbitCenter);
    ctx.draw_bitmap(m_BmpPtr);
    ctx.set_world_matrix(float3x3::identity());
}
void EnemyRotater::Tick(double deltaTime)
{
    m_Angle += SPEEDROTATE*deltaTime;
    m_ActPtr->SetAngle(m_Angle);
}

PhysicsActor* EnemyRotater::GetActor()
{
    return m_ActPtr;
}
void EnemyRotater::Reset()
{

}
void EnemyRotater::SetSpawnPosition(float2 respawnPosition)
{
    m_Position = respawnPosition;
}
