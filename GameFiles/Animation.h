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
//-----------------------------------------------------
// Animation Class									
//-----------------------------------------------------
class Animation //: public ContactListener
{
public:
	Animation(DOUBLE2 position);
	virtual ~Animation( );

	// C++11 make the class non-copyable
	Animation( const Animation& ) = delete;
	Animation& operator=( const Animation& ) = delete;

	//--------------------------------------------------------
	// ContactListener overloaded member function declarations
	//--------------------------------------------------------
	//virtual void BeginContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr); 
	//virtual void EndContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr);   
	//virtual void ContactImpulse(PhysicsActor *actThisPtr, double impulse);
    virtual void Tick(double deltaTime) = 0;
    virtual void Paint() = 0;
    virtual bool IsEnded();
protected:
    DOUBLE2 m_Position;
    bool m_IsEnded = false;
private: 
	//-------------------------------------------------
	// Datamembers								
	//-------------------------------------------------

};

 
