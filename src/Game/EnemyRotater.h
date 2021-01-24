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
// EnemyRotater Class									
//-----------------------------------------------------
class EnemyRotater : public Enemy
{
public:
    EnemyRotater(float2 position, Bitmap* bmpPtr);
	virtual ~EnemyRotater( );

	// C++11 make the class non-copyable
	EnemyRotater( const EnemyRotater& ) = delete;
	EnemyRotater& operator=( const EnemyRotater& ) = delete;

	//--------------------------------------------------------
	// ContactListener overloaded member function declarations
	//--------------------------------------------------------
	virtual void BeginContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr); 
	virtual void EndContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr);   
	virtual void ContactImpulse(PhysicsActor *actThisPtr, double impulse);
    virtual PhysicsActor* GetActor();
    virtual void SetSpawnPosition(float2 respawnPosition);
    virtual void Reset();

    virtual void Tick(double deltaTime) override;
    virtual void Paint(graphics::D2DRenderContext& ctx) override;
    

private: 
	//-------------------------------------------------
	// Datamembers								
	//-------------------------------------------------
    static const int SPEEDROTATE = 3;
    static const int RADIUSBALL = 10;
    int m_Radius = 67;
    double m_Angle = 0;

    Bitmap* m_BmpPtr = nullptr;
};

 
