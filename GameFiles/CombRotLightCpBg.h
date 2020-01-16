#pragma once
//-----------------------------------------------------
// Name: Steyfkens
// First name: Jonathan
// Group: 1DAE01
//-----------------------------------------------------

//-----------------------------------------------------
// Include Files
//-----------------------------------------------------

//#include "ContactListener.h"
#include "Animation.h"
//-----------------------------------------------------
// CombRotLightCpBg Class									
//-----------------------------------------------------
class RotLight;
class CheckPointBg;
class CombRotLightCpBg : public Animation
{
public:
	CombRotLightCpBg(DOUBLE2 position,int radius,Bitmap* bitmap, COLOR color);
	virtual ~CombRotLightCpBg( );

	// C++11 make the class non-copyable
	CombRotLightCpBg( const CombRotLightCpBg& ) = delete;
	CombRotLightCpBg& operator=( const CombRotLightCpBg& ) = delete;

	//--------------------------------------------------------
	// ContactListener overloaded member function declarations
	//--------------------------------------------------------
	//virtual void BeginContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr); 
	//virtual void EndContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr);   
	//virtual void ContactImpulse(PhysicsActor *actThisPtr, double impulse);
    virtual void Paint();
    virtual void Tick(double deltaTime);

private: 
	//-------------------------------------------------
	// Datamembers								
	//-------------------------------------------------
    RotLight *m_RotLightPtr = nullptr;
    CheckPointBg *m_CheckPointBgPtr = nullptr;
};

 
