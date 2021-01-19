#pragma once
//-----------------------------------------------------
// Name: Steyfkens
// First name: Jonathan
// Group: 1DAE01
//-----------------------------------------------------

//-----------------------------------------------------
// Include Files
//-----------------------------------------------------

#include "Enemy.h"
//-----------------------------------------------------
// Slicer Class									
//-----------------------------------------------------
class Slicer : public Enemy
{
public:
    Slicer(DOUBLE2 position, DOUBLE2 barPoint, int radius);
	virtual ~Slicer( );

	// C++11 make the class non-copyable
	Slicer( const Slicer& ) = delete;
	Slicer& operator=( const Slicer& ) = delete;

	//--------------------------------------------------------
	// ContactListener overloaded member function declarations
	//--------------------------------------------------------
	virtual void BeginContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr); 
	virtual void EndContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr);   
	virtual void ContactImpulse(PhysicsActor *actThisPtr, double impulse);

    virtual void Paint(graphics::D2DRenderContext& ctx);
    virtual void Tick(double deltaTime);
    virtual PhysicsActor* GetActor();



private: 
	//-------------------------------------------------
	// Datamembers								
	//-------------------------------------------------
    static const int BLADEWIDTH = 150;
    static const int BLADEHEIGHT = 20;
    DOUBLE2 m_BarPosition;
    PhysicsActor* m_ActBarPoint = nullptr;
    PhysicsActor* m_ActBarPtr = nullptr;


    PhysicsRevoluteJoint* m_JntPtr = nullptr;
    PhysicsRevoluteJoint* m_JntBarPlatform = nullptr;

    double m_Radius = 67;
};

 
