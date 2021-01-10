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
// MetalFans Class									
//-----------------------------------------------------
class MetalFans : public Entity
{
public:
    MetalFans(DOUBLE2 position, double angle);
	virtual ~MetalFans( );

	// C++11 make the class non-copyable
	MetalFans( const MetalFans& ) = delete;
	MetalFans& operator=( const MetalFans& ) = delete;

	//--------------------------------------------------------
	// ContactListener overloaded member function declarations
	//--------------------------------------------------------
	virtual void BeginContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr); 
	virtual void EndContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr);   
	virtual void ContactImpulse(PhysicsActor *actThisPtr, double impulse);
    virtual void Paint();
    virtual void Tick(double deltaTime);
    virtual void Reset();

    void SetRepulsionForce(double force);
    void SetFrictionForce(double force);

private: 
	//-------------------------------------------------
	// Datamembers								
	//-------------------------------------------------
    static const int WIDTH = 200;
    static const int HEIGHT = 25;
    int m_TriggerHeight = 400;
    double m_RepulsionForce = 50;
    double m_FrictionForce = 25;
    double m_Angle = 0;
    DOUBLE2 m_Direction;
    PhysicsActor* m_ActTriggerPtr = nullptr;
};

 
