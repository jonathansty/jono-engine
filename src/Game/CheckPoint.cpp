//-----------------------------------------------------
// Name: Steyfkens
// First name: Jonathan
// Group: 1DAE01
//-----------------------------------------------------
#include "stdafx.h"		
	
//---------------------------
// Includes
//---------------------------
#include "CheckPoint.h"
#include "Entity.h"
#include "RotLight.h"
#include "Avatar.h"
#include "SoundManager.h"

CheckPoint::CheckPoint(float2 position, Bitmap* bmpFlagPtr)
	: Entity(position)
    , m_BmpFlagPtr(bmpFlagPtr)
{
	// nothing to create
	// m_ActCirclePtr->AddContactListener(this);
    m_ActPtr = new PhysicsActor(position, 0, BodyType::STATIC);
    m_ActPtr->AddBoxShape(m_BmpFlagPtr->GetWidth(), m_BmpFlagPtr->GetHeight());
    m_ActPtr->SetTrigger(true);
    m_ActPtr->SetName(String("CheckPoint"));
    m_ActPtr->AddContactListener(this);

    m_SndPtr = SoundManager::instance()->LoadSound(String("Resources/Sound/Entity/CheckPoint01.wav"));
}

CheckPoint::~CheckPoint()
{
    m_ActPtr->RemoveContactListener(this);
    delete m_ActPtr;
    m_ActPtr = nullptr;

}

//-------------------------------------------------------
// ContactListener overloaded member function definitions
//-------------------------------------------------------
void CheckPoint::BeginContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr)
{
    if (actOtherPtr == m_AvatarPtr->GetActor())
    {
        m_SndPtr->play();
        m_IsHit = true;
    }
}
void CheckPoint::EndContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr)
{
    if (actOtherPtr == m_AvatarPtr->GetActor())
    {
        m_IsHit = false;
    }
}
void CheckPoint::ContactImpulse(PhysicsActor *actThisPtr, double impulse)
{

}
void CheckPoint::Paint(graphics::D2DRenderContext& ctx)
{
    using hlslpp::float3x3;
    float3x3 matTranslate, matPivot;
    matTranslate = float3x3::translation(m_Position);
    matPivot = float3x3::translation(float2(-20, -m_BmpFlagPtr->GetHeight() / 2));
    ctx.set_world_matrix(matPivot * matTranslate);
    ctx.draw_bitmap(m_BmpFlagPtr);
    ctx.set_world_matrix(float3x3::identity());
}
void CheckPoint::Tick(double deltaTime)
{

}
PhysicsActor* CheckPoint::GetActor()
{
    return m_ActPtr;
}
void CheckPoint::Reset()
{
    m_IsHit = false;
}
void CheckPoint::SetSpawnPosition(float2 respawnPosition)
{

}
bool CheckPoint::isHit()
{
    return m_IsHit;
}
double CheckPoint::GetCameraAngle()
{
    return m_CameraAngle;
}
void CheckPoint::SetCameraAngle(double angle)
{
    m_CameraAngle = angle;
}
void CheckPoint::SetCameraPosition(float2 position)
{
    m_CameraPosition = position;
}
float2 CheckPoint::GetCameraPosition()
{
    return m_CameraPosition;
}