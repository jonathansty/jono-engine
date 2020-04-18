//-----------------------------------------------------
// Name: Steyfkens
// First name: Jonathan
// Group: 1DAE01
//-----------------------------------------------------
#include "stdafx.h"		
	
#include "CombRotLightCpBg.h"
#include "CheckPointBg.h"
#include "RotLight.h"

CombRotLightCpBg::CombRotLightCpBg(DOUBLE2 position,int radius, Bitmap*  bmpPtr, COLOR color)
    : Animation(position)
{
	// nothing to create
	// m_ActCirclePtr->AddContactListener(this);
    m_CheckPointBgPtr = new CheckPointBg(position, bmpPtr);
    m_RotLightPtr = new RotLight(position);
    m_RotLightPtr->SetRadius(radius);
    m_RotLightPtr->SetColor(color);


}

CombRotLightCpBg::~CombRotLightCpBg()
{
    delete m_RotLightPtr;
    m_RotLightPtr = nullptr;

    delete m_CheckPointBgPtr;
    m_CheckPointBgPtr = nullptr;
}

//-------------------------------------------------------
// ContactListener overloaded member function definitions
//-------------------------------------------------------
//void CombRotLightCpBg::BeginContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr)
//{
//
//}
//
//void CombRotLightCpBg::EndContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr)
//{
//
//}
//
//void CombRotLightCpBg::ContactImpulse(PhysicsActor *actThisPtr, double impulse)
//{
//
//}
void CombRotLightCpBg::Tick(double deltaTime)
{
    m_CheckPointBgPtr->Tick(deltaTime);
    m_RotLightPtr->Tick(deltaTime);
}
void CombRotLightCpBg::Paint()
{
    m_RotLightPtr->Paint();
    m_CheckPointBgPtr->Paint();
}


