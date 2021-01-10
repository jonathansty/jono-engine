#pragma once
//-----------------------------------------------------
// Name:
// First name:
// Group: 1DAE.
//-----------------------------------------------------

//-----------------------------------------------------
// Include Files
//-----------------------------------------------------

//#include "ContactListener.h"
#include "Entity.h"
//-----------------------------------------------------
// Gate Class									
//-----------------------------------------------------
class Gate : public Entity
{
public:
    Gate(DOUBLE2 position, DOUBLE2 triggerposition);
	virtual ~Gate( );

	// C++11 make the class non-copyable
	Gate( const Gate& ) = delete;
	Gate& operator=( const Gate& ) = delete;

	//--------------------------------------------------------
	// ContactListener overloaded member function declarations
	//--------------------------------------------------------
	virtual void BeginContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr); 
	virtual void EndContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr);   
	virtual void ContactImpulse(PhysicsActor *actThisPtr, double impulse);
    virtual void Tick(double deltaTime);
    virtual void Paint();
    virtual void Reset();

private: 
	//-------------------------------------------------
	// Datamembers								
	//-------------------------------------------------
    
    DOUBLE2 m_TriggerPosition;
    int m_Width;
    int m_Height;
    PhysicsActor* m_ActGatePtr = nullptr;
    PhysicsActor* m_ActBasePtr = nullptr;
    PhysicsPrismaticJoint* m_JntGatePtr = nullptr;
};

 
