//-----------------------------------------------------
// Name: Steyfkens
// First name: Jonathan
// Group: 1DAE.
//-----------------------------------------------------
#include "stdafx.h"		
	
//---------------------------
// Includes
//---------------------------
#include "Teleport.h"
#include "Avatar.h"
//---------------------------
// Defines
//---------------------------
#define GAME_ENGINE (GameEngine::GetSingleton())

//---------------------------
// Constructor & Destructor
//---------------------------
Teleport::Teleport(DOUBLE2 teleportEntrance, DOUBLE2 teleportExit, Bitmap* bmpPtr) :
Entity((teleportExit - teleportEntrance)/2),
m_TeleEntrance(teleportEntrance),
m_TeleExit(teleportExit),
m_BmpPtr(bmpPtr)
{
    m_ActEntrancePtr = new PhysicsActor(teleportEntrance, 0, BodyType::STATIC);
    m_ActEntrancePtr->AddCircleShape(20);
    m_ActEntrancePtr->AddContactListener(this);
    m_ActEntrancePtr->SetTrigger(true);
    m_ActExitPtr = new PhysicsActor(teleportExit, 0, BodyType::STATIC);
    m_ActExitPtr->AddCircleShape(20);
    m_ActExitPtr->SetTrigger(true);
    m_ActExitPtr->AddContactListener(this);
}

Teleport::~Teleport()
{
    m_ActEntrancePtr->RemoveContactListener(this);
    delete m_ActEntrancePtr;
    m_ActEntrancePtr = nullptr;

    m_ActExitPtr->RemoveContactListener(this);
    delete m_ActExitPtr;
    m_ActEntrancePtr = nullptr;
}

//-------------------------------------------------------
// ContactListener overloaded member function definitions
//-------------------------------------------------------
void Teleport::BeginContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr)
{
    if (!m_TeleportedToEntrance && !m_TeleportedToExit && actThisPtr == m_ActEntrancePtr && actOtherPtr == m_AvatarPtr->GetActor())
    {
        m_AvatarPtr->GetActor()->SetPosition(m_TeleExit);
        m_AvatarPtr->GetActor()->SetLinearVelocity(DOUBLE2(0, 0));
        m_TeleportedToExit = true;
    }
    /*if (!m_TeleportedToExit && !m_TeleportedToEntrance && actThisPtr == m_ActExitPtr && actOtherPtr == m_AvatarPtr->GetActor())
    {
        m_AvatarPtr->GetActor()->SetPosition(m_TeleEntrance);
        m_AvatarPtr->GetActor()->SetLinearVelocity(DOUBLE2(0, 0));
        m_TeleportedToEntrance = true;
    }*/
}

void Teleport::EndContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr)
{
    /*if (actThisPtr == m_ActExitPtr && actOtherPtr == m_AvatarPtr->GetActor())
    {
        m_TeleportedToExit = false;
    }*/
    if (actThisPtr == m_ActEntrancePtr && actOtherPtr == m_AvatarPtr->GetActor())
    {
        m_TeleportedToEntrance = false;
    }
}

void Teleport::ContactImpulse(PhysicsActor *actThisPtr, double impulse)
{

}
void Teleport::Tick(double deltaTime)
{
    m_Angle += 25*deltaTime;
}
void Teleport::Paint()
{
    MATRIX3X2 matTranslate, matRotate, matPivot, matWorldTransform;
    matTranslate.SetAsTranslate(m_TeleEntrance);
    matRotate.SetAsRotate(m_Angle);
    matPivot.SetAsTranslate(DOUBLE2(-m_BmpPtr->GetWidth() / 2, -m_BmpPtr->GetHeight() / 2));
    matWorldTransform = matPivot * matRotate * matTranslate;
    GAME_ENGINE->SetWorldMatrix(matWorldTransform);
    GAME_ENGINE->DrawBitmap(m_BmpPtr);
    matTranslate.SetAsTranslate(m_TeleExit);
    matRotate.SetAsRotate(m_Angle + 1.5);
    matWorldTransform = matPivot * matRotate * matTranslate;
    GAME_ENGINE->SetWorldMatrix(matWorldTransform);
    GAME_ENGINE->DrawBitmap(m_BmpPtr);
    GAME_ENGINE->SetWorldMatrix(MATRIX3X2::CreateIdentityMatrix());
}
void Teleport::Reset()
{
    m_TeleportedToEntrance = false;
    m_TeleportedToExit = false;
}
