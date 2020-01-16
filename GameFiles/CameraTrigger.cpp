//-----------------------------------------------------
// Name: Steyfkens
// First name: Jonathan
// Group: 1DAE01
//-----------------------------------------------------
#include "stdafx.h"		
	
//---------------------------
// Includes
//---------------------------
#include "CameraTrigger.h"

//---------------------------
// Defines
//---------------------------
#define GAME_ENGINE (GameEngine::GetSingleton())

//---------------------------
// Constructor & Destructor
//---------------------------
CameraTrigger::CameraTrigger(DOUBLE2 position, int width, int height):
m_Position(position),
m_Width(width),
m_Height(height)
{
	// nothing to create
	// m_ActCirclePtr->AddContactListener(this);
}

CameraTrigger::~CameraTrigger()
{
    if (m_ActPtr != nullptr)
    {
        m_ActPtr->RemoveContactListener(m_ActPtr->GetContactListener());
        delete m_ActPtr;
        m_ActPtr = nullptr;
    }
}

//-------------------------------------------------------
// ContactListener overloaded member function definitions
//-------------------------------------------------------
void CameraTrigger::BeginContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr)
{

}

void CameraTrigger::EndContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr)
{

}

void CameraTrigger::ContactImpulse(PhysicsActor *actThisPtr, double impulse)
{

}

bool CameraTrigger::IsHit()
{
    return m_IsHit;
}
void CameraTrigger::SetCamera(Camera* tmpCameraPtr)
{
    m_CameraPtr = tmpCameraPtr;
}
void CameraTrigger::Reset()
{
    m_IsHit = false;
}

