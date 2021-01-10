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
#include "CameraTrigger.h"
//-----------------------------------------------------
// CameraTriggerRotate Class									
//-----------------------------------------------------
class CameraTriggerRotate : public CameraTrigger
{
public:
	CameraTriggerRotate(DOUBLE2 position, int width, int height);
	virtual ~CameraTriggerRotate( );

	// C++11 make the class non-copyable
	CameraTriggerRotate( const CameraTriggerRotate& ) = delete;
	CameraTriggerRotate& operator=( const CameraTriggerRotate& ) = delete;

	//--------------------------------------------------------
	// ContactListener overloaded member function declarations
	//--------------------------------------------------------
	virtual void BeginContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr); 
	virtual void EndContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr);   
	virtual void ContactImpulse(PhysicsActor *actThisPtr, double impulse);
    virtual void Tick(double deltaTime);
    void SetAngleLimit(double lowerAngle,double upperAngle);
    void SetAngularVelocity(double angularVelocity);
private: 
	//-------------------------------------------------
	// Datamembers								
	//-------------------------------------------------
    double m_StartAngle = 0;
    double m_AngularVelocity = 0.5;
    double m_LowerAngle = 0;
    double m_UpperAngle = 0;
};

 
