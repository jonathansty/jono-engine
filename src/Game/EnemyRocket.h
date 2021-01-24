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
// EnemyRocket Class									
//-----------------------------------------------------
class Avatar;
class Trail;
class EnemyRocket : public Enemy
{
public:
	EnemyRocket(float2 startPosition, float2 startVelocity);
	virtual ~EnemyRocket( );

	// C++11 make the class non-copyable
	EnemyRocket( const EnemyRocket& ) = delete;
	EnemyRocket& operator=( const EnemyRocket& ) = delete;

	//--------------------------------------------------------
	// ContactListener overloaded member function declarations
	//--------------------------------------------------------
	virtual void BeginContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr); 
	virtual void EndContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr);   
	virtual void ContactImpulse(PhysicsActor *actThisPtr, double impulse);
    virtual PhysicsActor* GetActor();
    virtual void Reset();
    void Tick(double deltaTime);
    void Paint(graphics::D2DRenderContext& ctx);

    bool GetAttackByAvatar();
    void SetSpeed(double speed);

private: 
	//-------------------------------------------------
	// Datamembers								
	//-------------------------------------------------
    static const int WIDTH = 30;
    static const int HEIGHT = 15;
    float2 m_Direction;
    float2 m_AdjustedVelocity;

    Trail* m_TrailPtr = nullptr;
    bool m_IsHitByAvatar = false;

    double m_Speed = 250;
    double m_AngularVelocity = 3;
    double m_Angle = 0;
};

 
