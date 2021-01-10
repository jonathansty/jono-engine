#pragma once
//-----------------------------------------------------
// Name: Steyfkens
// First name: Jonathan
// Group: 1DAE01
//-----------------------------------------------------

//-----------------------------------------------------
// Include Files
//-----------------------------------------------------
#include <deque>
//#include "ContactListener.h"
//-----------------------------------------------------
// AttackBeamList Class									
//-----------------------------------------------------
class AttackBeam;
class AttackBeamList //: public ContactListener
{
public:
	AttackBeamList( );
	virtual ~AttackBeamList( );

	// C++11 make the class non-copyable
	AttackBeamList( const AttackBeamList& ) = delete;
	AttackBeamList& operator=( const AttackBeamList& ) = delete;

	//--------------------------------------------------------
	// ContactListener overloaded member function declarations
	//--------------------------------------------------------
	//virtual void BeginContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr); 
	//virtual void EndContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr);   
	//virtual void ContactImpulse(PhysicsActor *actThisPtr, double impulse);

    void Add(AttackBeam* attackBeamPtr);
    void Remove();

    void Paint();
    void Tick(double deltaTime);
private: 
	//-------------------------------------------------
	// Datamembers								
	//-------------------------------------------------
    static const int LIFETIME = 4;
    std::deque<AttackBeam*>m_AttackBeamsPtrArr;
};

 
