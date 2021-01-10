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
#include <deque>
//-----------------------------------------------------
// Trail Class									
//-----------------------------------------------------
class Trail 
{
public:
    Trail(DOUBLE2 position);
	virtual ~Trail( );

	// C++11 make the class non-copyable
	Trail( const Trail& ) = delete;
	Trail& operator=( const Trail& ) = delete;

	//--------------------------------------------------------
	// ContactListener overloaded member function declarations
	//--------------------------------------------------------
	//virtual void BeginContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr); 
	//virtual void EndContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr);   
	//virtual void ContactImpulse(PhysicsActor *actThisPtr, double impulse);
    void Paint(DOUBLE2 position);

    void SetSize(double size);
    void SetTrailLength(double length);
    void SetInterpolation(int numberOfCircles);
    // Opacity has to be a number between 255 and 0
    void SetOpacity(int opacity);
private: 
	//-------------------------------------------------
	// Datamembers								
	//-------------------------------------------------
    DOUBLE2 m_Position;
    double m_TrailLength = 100;
    int m_Opacity = 255;
    double m_Size = 30;
    double m_AmountOfInterpolation = 2;
    std::deque<DOUBLE2>m_deqTrailPtrArr;
};

 
