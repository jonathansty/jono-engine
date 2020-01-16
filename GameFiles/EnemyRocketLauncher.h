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
// EnemyRocketLauncher Class									
//-----------------------------------------------------
class EnemyRocket;
class EnemyList;
class AnimationList;
class Trail;
class EntityDestroy;
class EnemyRocketLauncher : public Enemy
{
public:
	EnemyRocketLauncher(DOUBLE2 position, double angle);
	virtual ~EnemyRocketLauncher( );

	// C++11 make the class non-copyable
	EnemyRocketLauncher( const EnemyRocketLauncher& ) = delete;
	EnemyRocketLauncher& operator=( const EnemyRocketLauncher& ) = delete;

	//--------------------------------------------------------
	// ContactListener overloaded member function declarations
	//--------------------------------------------------------
	virtual void BeginContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr); 
	virtual void EndContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr);   
	virtual void ContactImpulse(PhysicsActor *actThisPtr, double impulse);

    void Tick(double deltaTime);
    void Paint();
    void PaintRockets();

    enum class actionState
    {
        SHOOTING,
        WAITING,
        PAUSE
    };

    virtual PhysicsActor* GetActor();

    virtual void RemoveContactListener();
    virtual void SetActActive(bool tmpBool);
    virtual void Reset();
    void SetAvatar(Avatar* avatarPtr);

    virtual bool GetAttackByAvatar();
    
    virtual void PaintDebug();


private: 
    RECT updateFrameDisplay(int frameNr);
	//-------------------------------------------------
	// Datamembers								
	//-------------------------------------------------
    static const int WIDTH = 89;
    static const int HEIGHT = 72;
    static const int MAXROCKETS = 5;
    static const int DETECTIONZONE = 560;
    static const int MAXFRAMES = 11;
    static const int FRAMERATE = 6;
    int m_FrameNr = 0;
    Bitmap* m_BmpPtr = nullptr;
    int m_AmountOfRockets = 0;
    double m_Angle = 0;
    double m_AccuShootTime = 2;
    double m_AccuTime = 0;
    double m_IntervalTime = 2;
    DOUBLE2 m_Direction;
    EnemyList* m_EnemyListPtr = nullptr;
    AnimationList* m_AnimationListPtr = nullptr;
    bool m_IsReset = false;

    actionState m_ActionState = actionState::WAITING;
};

 
