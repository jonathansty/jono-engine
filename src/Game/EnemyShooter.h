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
// EnemyShooter Class									
//-----------------------------------------------------
class Bullet;
class AnimationList;
class EnemyShooter : public Enemy
{
public:
    EnemyShooter(float2 position, Bitmap* bmpPtr, double angle);
	virtual ~EnemyShooter( );

	// C++11 make the class non-copyable
	EnemyShooter( const EnemyShooter& ) = delete;
	EnemyShooter& operator=( const EnemyShooter& ) = delete;

	//--------------------------------------------------------
	// ContactListener overloaded member function declarations
	//--------------------------------------------------------
	virtual void BeginContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr); 
	virtual void EndContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr);   
	virtual void ContactImpulse(PhysicsActor *actThisPtr, double impulse);

    void Paint(graphics::D2DRenderContext& ctx);
    void Tick(double deltaTime);

    PhysicsActor* GetActor();
    float2 GetPosition();

    virtual void RemoveContactListener();
    void SetSpawnPosition(float2 respawnPosition);
    void Reset();
    void SetMirror(bool mirror);
    

private: 
	//-------------------------------------------------
	// Datamembers								
	//-------------------------------------------------
    std::vector<PhysicsActor*>m_ActorToRemovePtrArr;
    static const int BULLET_LIFE = 5;
    static const int FRAMERATE = 6;
    static const int MAXFRAMES = 9;
    static const int CLIPWIDTH = 60;
    static const int CLIPHEIGHT = 62;
    int m_FrameNr = 0;
    
    RECT m_BoundingBox;
    Bitmap* m_BmpEnemyBodyPtr = nullptr;
    double m_AccuTime = 0;
    double m_Angle = 0;

    bool m_Mirror = false;
    bool m_IsHit = false;

    PhysicsActor* m_ActBulletPtr = nullptr;
    AnimationList* m_AnimationListPtr = nullptr;
    enum class state{
        SHOOTING,
        IDLE
    }m_State = state::SHOOTING;
};

 
