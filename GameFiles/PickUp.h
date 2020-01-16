#pragma once
//-----------------------------------------------------
// Name: Steyfkens
// First name: Jonathan
// Group: 1DAE01
//-----------------------------------------------------

//-----------------------------------------------------
// Include Files
//-----------------------------------------------------

#include "ContactListener.h"
//-----------------------------------------------------
// PickUp Class									
//-----------------------------------------------------
class PickUp : public ContactListener
{
public:
	PickUp(DOUBLE2 position);
	virtual ~PickUp( );

	// C++11 make the class non-copyable
	PickUp( const PickUp& ) = delete;
	PickUp& operator=( const PickUp& ) = delete;

	//--------------------------------------------------------
	// ContactListener overloaded member function declarations
	//--------------------------------------------------------
	virtual void BeginContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr); 
	virtual void EndContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr);   
	virtual void ContactImpulse(PhysicsActor *actThisPtr, double impulse);

    virtual void Tick(double deltaTime) = 0;
    virtual void Paint() = 0;
    virtual bool IsHit();

    virtual PhysicsActor* GetActor();
    virtual void SetName(String name);
    virtual String GetName();

protected:

    //-------------------------------------------------
    // Datamembers								
    //-------------------------------------------------
    String m_Name = String("NaN");
    PhysicsActor* m_ActPtr = nullptr;
    DOUBLE2 m_Position;
    bool m_IsHit = false;
    
    Sound* m_SndHitPtr = nullptr;
    b2Filter m_CollisionFilter;
private: 
	//-------------------------------------------------
	// Datamembers								
	//-------------------------------------------------

};

 
