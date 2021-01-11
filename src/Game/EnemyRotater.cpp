//-----------------------------------------------------
// Name: Steyfkens
// First name: Jonathan
// Group: 1DAE01
//-----------------------------------------------------
#include "stdafx.h"		
	
//---------------------------
// Includes
//---------------------------
#include "EnemyRotater.h"

//---------------------------
// Constructor & Destructor
//---------------------------
EnemyRotater::EnemyRotater(DOUBLE2 position, Bitmap* bmpPtr): Enemy(position), m_BmpPtr(bmpPtr)
{
    m_ActPtr = new PhysicsActor(position, 0, BodyType::KINEMATIC);
    m_ActPtr->AddCircleShape(bmpPtr->GetWidth()/2,DOUBLE2(m_Radius,0),0,0);
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
void EnemyRotater::Paint()
{
    DOUBLE2 position = m_ActPtr->GetPosition();
    MATRIX3X2 matRotation, matOrbitRadius, matOrbitCenter, matCenter,matBitmapRotation;
    matCenter.SetAsTranslate(DOUBLE2(-m_BmpPtr->GetWidth() / 2, -m_BmpPtr->GetHeight() / 2));
    matRotation.SetAsRotate(m_Angle);
    matOrbitRadius.SetAsTranslate(DOUBLE2(m_Radius,0));
    matOrbitCenter.SetAsTranslate(position);
    matBitmapRotation = matRotation.Inverse();
    GameEngine::instance()->set_world_matrix(matCenter *matBitmapRotation * matOrbitRadius*matRotation*matOrbitCenter);
    GameEngine::instance()->DrawBitmap(m_BmpPtr);
    GameEngine::instance()->set_world_matrix(MATRIX3X2::CreateIdentityMatrix());
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
void EnemyRotater::SetSpawnPosition(DOUBLE2 respawnPosition)
{
    m_Position = respawnPosition;
}
