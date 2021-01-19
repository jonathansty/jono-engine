#pragma once
//-----------------------------------------------------
// Name:
// First name:
// Group: 1DAE.
//-----------------------------------------------------

//-----------------------------------------------------
// Include Files
//-----------------------------------------------------

//#include "ContactListener.h"
#include "Enemy.h"
//-----------------------------------------------------
// EnemyHorizontal Class									
//-----------------------------------------------------
class EnemyHorizontal : public Enemy
{
public:
    EnemyHorizontal(DOUBLE2 position, Bitmap* bmpEnemyPtr, Avatar* avatarPtr);
	virtual ~EnemyHorizontal( );

	// C++11 make the class non-copEnemyable
	EnemyHorizontal( const EnemyHorizontal& ) = delete;
	EnemyHorizontal& operator=( const EnemyHorizontal& ) = delete;

	//--------------------------------------------------------
	// ContactListener overloaded member function declarations
	//--------------------------------------------------------
    virtual void BeginContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr);
    virtual void EndContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr);
    virtual void ContactImpulse(PhysicsActor *actThisPtr, double impulse);
    virtual PhysicsActor* GetActor();
    virtual void Reset();
    void Tick(double deltatime);
    void Paint(graphics::D2DRenderContext& ctx);

    void SetVelocity(DOUBLE2 velocity);
    void SetOffSet(DOUBLE2 offset);
    void SetAvatar(Avatar* m_AvatarPtr);
    void SetLifes(int lifes);
    bool GetAttackByAvatar();
    int GetLifes();
private: 
	//-------------------------------------------------
	// Datamembers								
	//-------------------------------------------------
    Bitmap* m_BmpPtr = nullptr;
    Avatar* m_AvatarPtr = nullptr;
    DOUBLE2 m_StartPosition;
    DOUBLE2 m_Velocity;
    DOUBLE2 m_OffSet = DOUBLE2(30, 0);
    int m_Lifes = 1;
    bool m_boolAttackContact = false;
};

 
