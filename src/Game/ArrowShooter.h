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
// ArrowShooter Class									
//-----------------------------------------------------
class Arrow;
class ArrowShooter : public Entity
{
public:
	ArrowShooter(DOUBLE2 position, DOUBLE2 direction,double m_IntervalTime);
	virtual ~ArrowShooter( );

	// C++11 make the class non-copyable
	ArrowShooter( const ArrowShooter& ) = delete;
	ArrowShooter& operator=( const ArrowShooter& ) = delete;

	//--------------------------------------------------------
	// ContactListener overloaded member function declarations
	//--------------------------------------------------------
	virtual void BeginContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr); 
	virtual void EndContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr);   
	virtual void ContactImpulse(PhysicsActor *actThisPtr, double impulse);

    virtual void Paint(graphics::D2DRenderContext& ctx);
    virtual void Tick(double deltaTime);
    virtual void Reset();
    void SetPushPower(int pushPower);
private: 
	//-------------------------------------------------
	// Datamembers								
	//-------------------------------------------------
    static const int SPEED = 100;
    void Add(Arrow* tmpArrowPtr);
    int m_AmountOfArrows = 0;
    int m_PushPower = 0;
    double m_AccuTime = 0;
    double m_Angle = 0;
    static const int WIDTH = 15;
    static const int HEIGHT = 50;
    std::vector<Arrow*>m_ArrowsPtrArr;
    DOUBLE2 m_direction;
    double m_IntervalTime = 1;
};

 
