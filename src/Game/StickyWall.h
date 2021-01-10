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
// StickyWall Class									
//-----------------------------------------------------
class StickyWall : public Entity
{
public:
	StickyWall(DOUBLE2 position, int width, int height);
	virtual ~StickyWall( );

	// C++11 make the class non-copyable
	StickyWall( const StickyWall& ) = delete;
	StickyWall& operator=( const StickyWall& ) = delete;

	//--------------------------------------------------------
	// ContactListener overloaded member function declarations
	//--------------------------------------------------------
	virtual void BeginContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr); 
	virtual void EndContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr);   
	virtual void ContactImpulse(PhysicsActor *actThisPtr, double impulse);

    void Paint();
    void Tick(double deltaTime);
    void Reset();
private: 
	//-------------------------------------------------
	// Datamembers								
	//-------------------------------------------------
    int m_Width;
    int m_Height;

    
};

 
