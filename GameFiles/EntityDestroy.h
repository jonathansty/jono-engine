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
#include "Animation.h"
//-----------------------------------------------------
// EntityDestroy Class									
//-----------------------------------------------------
class EntityDestroy : public Animation
{
public:
	EntityDestroy(DOUBLE2 position );
	virtual ~EntityDestroy( );

	// C++11 make the class non-copyable
	EntityDestroy( const EntityDestroy& ) = delete;
	EntityDestroy& operator=( const EntityDestroy& ) = delete;

	//--------------------------------------------------------
	// ContactListener overloaded member function declarations
	//--------------------------------------------------------
	//virtual void BeginContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr); 
	//virtual void EndContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr);   
	//virtual void ContactImpulse(PhysicsActor *actThisPtr, double impulse);
    void Tick(double deltaTime);
    void Paint();
    double GetOpacity();
    void SetRadius(int radius);
private: 
	//-------------------------------------------------
	// Datamembers								
	//-------------------------------------------------

    int m_Radius = 250;
    double m_Opacity = 1;
    double m_Scale = 0.2;
};

 
