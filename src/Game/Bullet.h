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
// Bullet Class									
//-----------------------------------------------------
class Bullet : ContactListener
{
public:
	Bullet(DOUBLE2 startPosition, DOUBLE2 direction );
	virtual ~Bullet( );

	// C++11 make the class non-copyable
	Bullet( const Bullet& ) = delete;
	Bullet& operator=( const Bullet& ) = delete;

	//--------------------------------------------------------
	// ContactListener overloaded member function declarations
	//--------------------------------------------------------
	virtual void BeginContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr); 
	virtual void EndContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr);   
	virtual void ContactImpulse(PhysicsActor *actThisPtr, double impulse);

    
private: 
	//-------------------------------------------------
	// Datamembers								
	//-------------------------------------------------
    bool m_IsHit = false;
    DOUBLE2 m_startPosition;
    DOUBLE2 m_Direction;

    PhysicsActor* m_ActPtr = nullptr;
    std::vector<PhysicsActor*>m_FriendActorPtr;
};

 
