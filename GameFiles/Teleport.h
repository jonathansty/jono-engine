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
#include "Entity.h"
//-----------------------------------------------------
// Teleport Class									
//-----------------------------------------------------
class Teleport : public Entity
{
public:
	Teleport(DOUBLE2 teleportEntrance,DOUBLE2 teleportExit, Bitmap* bmpPtr);
	virtual ~Teleport( );

	// C++11 make the class non-copyable
	Teleport( const Teleport& ) = delete;
	Teleport& operator=( const Teleport& ) = delete;

	//--------------------------------------------------------
	// ContactListener overloaded member function declarations
	//--------------------------------------------------------
	virtual void BeginContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr); 
	virtual void EndContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr);   
	virtual void ContactImpulse(PhysicsActor *actThisPtr, double impulse);

    virtual void Paint();
    virtual void Tick(double deltaTime);
    virtual void Reset();

    void SetEntrancePos(DOUBLE2 position);
    void SetExitPos(DOUBLE2 position);
private: 
	//-------------------------------------------------
	// Datamembers								
	//-------------------------------------------------
    DOUBLE2 m_TeleEntrance, m_TeleExit;
    Bitmap* m_BmpPtr = nullptr;
    double m_Angle = 0;
    bool m_TeleportedToExit = false;
    bool m_TeleportedToEntrance = false;
    PhysicsActor* m_ActEntrancePtr = nullptr;
    PhysicsActor* m_ActExitPtr = nullptr;

   
};

 
