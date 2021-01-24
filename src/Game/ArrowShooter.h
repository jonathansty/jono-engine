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
	ArrowShooter(float2 position, float2 direction,double m_IntervalTime);
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
    static const int SPEED = 100;
    static const int WIDTH = 15;
    static const int HEIGHT = 50;

    void Add(Arrow* tmpArrowPtr);

    int _n_arrows = 0;
    int _push_power = 0;
    double _timer = 0;
    double _angle = 0;
    double _interval_time = 1;
    float2 _direction;

    std::vector<Arrow*>m_ArrowsPtrArr;
};

 
