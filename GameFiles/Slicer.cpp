//-----------------------------------------------------
// Name: Steyfkens
// First name: Jonathan
// Group: 1DAE01
//-----------------------------------------------------
#include "stdafx.h"		
	
//---------------------------
// Includes
//---------------------------
#include "Slicer.h"
#include "Avatar.h"
//---------------------------
// Defines
//---------------------------
#define GAME_ENGINE (GameEngine::GetSingleton())

//---------------------------
// Constructor & Destructor
//---------------------------
Slicer::Slicer(DOUBLE2 position,DOUBLE2 barPoint,int radius): Enemy(position),
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
    m_JntPtr = new PhysicsRevoluteJoint(m_ActBarPoint, DOUBLE2(), m_ActBarPtr, DOUBLE2(-m_Radius, 0));
    m_JntBarPlatform = new PhysicsRevoluteJoint(m_ActBarPtr, DOUBLE2(m_Radius, 0), m_ActPtr, DOUBLE2());
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
    DOUBLE2 direction = m_ActBarPoint->GetPosition() - m_ActPtr->GetPosition();
    m_Position = m_ActPtr->GetPosition();
}
void Slicer::Paint()
{
    GAME_ENGINE->DrawLine(m_ActBarPoint->GetPosition(), m_ActPtr->GetPosition());

    MATRIX3X2 matTranlate, matRotate, matPivot;
    matTranlate.SetAsTranslate(m_Position);
    matRotate.SetAsRotate(m_ActPtr->GetAngle());
    matPivot.SetAsTranslate(DOUBLE2(-BLADEWIDTH / 2, -BLADEHEIGHT / 2));
    GAME_ENGINE->SetWorldMatrix(matPivot * matRotate * matTranlate);
    GAME_ENGINE->SetColor(COLOR(0, 0, 0));
    GAME_ENGINE->FillRect(0, 0, BLADEWIDTH, BLADEHEIGHT);

    matRotate.SetAsRotate(m_ActPtr->GetAngle() + M_PI_2);
    GAME_ENGINE->SetWorldMatrix(matPivot * matRotate * matTranlate);
    GAME_ENGINE->FillRect(0, 0, BLADEWIDTH, BLADEHEIGHT);
    GAME_ENGINE->SetWorldMatrix(MATRIX3X2::CreateIdentityMatrix());
}
PhysicsActor* Slicer::GetActor()
{
    return m_ActPtr;
}
