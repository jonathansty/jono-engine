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
// AnimationList Class									
//-----------------------------------------------------
class AnimationList //: public ContactListener
{
public:
	AnimationList( );
	virtual ~AnimationList( );

	// C++11 make the class non-copyable
	AnimationList( const AnimationList& ) = delete;
	AnimationList& operator=( const AnimationList& ) = delete;

	//--------------------------------------------------------
	// ContactListener overloaded member function declarations
	//--------------------------------------------------------
	//virtual void BeginContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr); 
	//virtual void EndContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr);   
	//virtual void ContactImpulse(PhysicsActor *actThisPtr, double impulse);
    void Paint();
    void Tick(double deltaTime);

    void Add(Animation* tmpAnimationPtr);
    void Remove(Animation* tmpAnimationPtr);
    void RemoveAll();

private: 
	//-------------------------------------------------
	// Datamembers								
	//-------------------------------------------------
    std::vector<Animation *>m_AnimationsPtrArr;
};

 
