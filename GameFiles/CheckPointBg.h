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
// CheckPointBg Class									
//-----------------------------------------------------
class CheckPointBg : public Animation
{
public:
    CheckPointBg(DOUBLE2 position, Bitmap* bmpPtr);
	virtual ~CheckPointBg( );

	// C++11 make the class non-copyable
	CheckPointBg( const CheckPointBg& ) = delete;
	CheckPointBg& operator=( const CheckPointBg& ) = delete;

	//--------------------------------------------------------
	// ContactListener overloaded member function declarations
	//--------------------------------------------------------
	//virtual void BeginContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr); 
	//virtual void EndContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr);   
	//virtual void ContactImpulse(PhysicsActor *actThisPtr, double impulse);
    enum class drawState{
        SPAWNING,
        MAXRADIUS
    };
    void Paint();
    void Tick(double deltaTime);
    void SetPosition(DOUBLE2 position);
    void SetDrawState(drawState drawState);
    DOUBLE2 GetPosition();
private: 
	//-------------------------------------------------
	// Datamembers								
	//-------------------------------------------------
    static const int MINRADIUS = 20;
    static const double SCALESPEED;
    Bitmap* m_BmpPtr = nullptr;
    double m_Angle = 0;
    double m_Scale = 0.01;
    double m_MaxScale = 1;
    double m_ScaleOffSet = 0.1;
    double m_ScaleSpeed = 0.5;
    drawState m_DrawState = drawState::SPAWNING;
};

 
