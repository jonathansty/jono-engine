#include "game.stdafx.h"		
	
#include "CameraTriggerRotate.h"
#include "Camera.h"

CameraTriggerRotate::CameraTriggerRotate(float2 position, int width, int height) :
CameraTrigger(position,width,height)
{
    m_ActPtr = new PhysicsActor(position, 0, BodyType::STATIC);
    m_ActPtr->SetTrigger(true);
    m_ActPtr->AddContactListener(this);
    m_ActPtr->AddBoxShape(width, height);
}

CameraTriggerRotate::~CameraTriggerRotate()
{

}

//-------------------------------------------------------
// ContactListener overloaded member function definitions
//-------------------------------------------------------
void CameraTriggerRotate::BeginContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr)
{
    if (actOtherPtr->GetName() == String("Camera"))
    {
        m_IsHit = true;
        m_StartAngle = m_CameraPtr->GetAngle();
    }
    
}

void CameraTriggerRotate::EndContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr)
{
    if (actOtherPtr->GetName() == String("Camera"))
    {
        m_IsHit = false;
    }
}

void CameraTriggerRotate::ContactImpulse(PhysicsActor *actThisPtr, double impulse)
{

}
void CameraTriggerRotate::Tick(double deltaTime)
{
    if (m_IsHit)
    {
        if (m_CameraPtr != nullptr)
        {
            double angle = m_CameraPtr->GetAngle();
            if (angle > m_LowerAngle && angle < m_UpperAngle)
            {
                m_CameraPtr->SetAngle(angle + m_AngularVelocity * deltaTime);
            }
            
        }
        
    }
}
void CameraTriggerRotate::SetAngleLimit(double lowerangle, double upperangle)
{
    m_LowerAngle = lowerangle;
    m_UpperAngle = upperangle;
}
void CameraTriggerRotate::SetAngularVelocity(double angularVelocity)
{
    m_AngularVelocity = angularVelocity;
}

