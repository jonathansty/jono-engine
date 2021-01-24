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
#include "Enemy.h"
//-----------------------------------------------------
// EnemyLaser Class									
//-----------------------------------------------------
class EnemyLaser : public Enemy
{
public:
	EnemyLaser(float2 position);
	virtual ~EnemyLaser( );

	// C++11 make the class non-copyable
	EnemyLaser( const EnemyLaser& ) = delete;
	EnemyLaser& operator=( const EnemyLaser& ) = delete;

	//--------------------------------------------------------
	// ContactListener overloaded member function declarations
	//--------------------------------------------------------
	virtual void BeginContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr); 
	virtual void EndContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr);   
	virtual void ContactImpulse(PhysicsActor *actThisPtr, double impulse);
    virtual void Paint(graphics::D2DRenderContext& ctx);
    virtual void Tick(double deltaTime);
    virtual void Reset();
    virtual PhysicsActor* GetActor();
    void SetAngularVelocity(double angularVelocity);
private: 
	//-------------------------------------------------
	// Datamembers								
	//-------------------------------------------------
    static const int RAYLENGTH = 250;
    double m_AngularVelocity = 2;
    float2 m_RayStart;
    float2 m_RayEnd;
    float2 m_RayEndRight;
    float2 m_IntersectionLeft;
    float2 m_IntersectionRight;
    
};

 
