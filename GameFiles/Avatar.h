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
#include "FileManager.h"
//-----------------------------------------------------
// Avatar Class									
//-----------------------------------------------------
class EntityList;
class EnemyList;
class Avatar : public Entity
{
public:
    Avatar(DOUBLE2 position, Bitmap* BmpPtr, Bitmap* bmpEpicModePtr);
	virtual ~Avatar( );

	// C++11 make the class non-copyable
	Avatar( const Avatar& ) = delete;
	Avatar& operator=( const Avatar& ) = delete;

	
    enum class moveState
    {
        RUNNING,
        STANDING,
        JUMPING,
        HANGING,
        ATTACK,
        DIE,
        DYING,
        SLIDINGSTANDING,
        SLIDINGWALKING,
        SLIDINGJUMPING,
        STICKYJUMP,
        GOD
    };
    //--------------------------------------------------------
    // ContactListener overloaded member function declarations
    //--------------------------------------------------------
	virtual void BeginContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr); 
	virtual void EndContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr);   
	virtual void ContactImpulse(PhysicsActor *actThisPtr, double impulse);
    
    //! Paint debug information
    void PaintDebug();
    void Paint();
    void Tick(double deltaTime);

    //! Add coins to the avatar
    void AddAmountOfCoins(int amount);

    //! Gets the avatar BoundingBox
    PhysicsActor* GetActor();
    //! Gets the avatar Position
    DOUBLE2 GetPosition();
    //! Gets the avatarRespawnPosition
    DOUBLE2 GetRespawnPosition();
    //! Gets amount of lifes the avatar currently has
    int GetLifes();
    //! Gets amount of deaths the avatar has experienced.
    int GetDeaths();
    //! Gets raw amount of coins. (No subdivision between copper, silver and gold)
    int GetAmountOfCoins();
    
    moveState GetMoveState();
    //! Sets the moveState.
    //! Possible values: RUNNING,STANDING,JUMPING,HANGING,ATTACK,
    //! DIE,DYING,SLIDINGSTANDING,SLIDINGWALKING,SLIDINGJUMPING,GOD
    void SetMoveState(moveState state);
    //! Sets the respawn Position for the avatar
    void SetSpawnPosition(DOUBLE2 respawnPosition);
    //! Resets the avatar back to original
    void Reset();
    //! Sets the jumpHeight of the avatar.
    void SetJumpHeight(double height);
    //! Directly sets the avatarPosition
    void SetPosition(DOUBLE2 position);
    //! Set amount of lifes
    void SetLifes(int lifes);
    //! Sets amount of deaths 
    void SetDeaths(int deaths);
    //! Set the game Over state or not.
    void SetGameOver();
    //! Sets how many times to avatar has jumped
    void SetNrJumps(int nr);
    //! Sets the keybinds the player specified
    void SetKeyBinds(FileManager::KeyMap tmpKeyBindsPtrArr);
private: 
    //-------------------------------------------------
    // private functions							
    //-------------------------------------------------

    //! Handles all the keypressed done by the player
    void checkKeys(double dTime);
    //!Handles the God moveState
    void GodMoveState(double dTime);
    //!Handles the sliding moveState
    void SlidingMoveState(double dTime);
    //! Handles the normalMoveState
    void NormalMoveState(double dTime);
    
    void PaintTrail();

    RECT updateFrameDisplay(int frameNr);
	//-------------------------------------------------
	// Datamembers								
	//-------------------------------------------------
    //Keybinds
    TCHAR m_Jump = 'Z';
    TCHAR m_Attack = 'X';
    TCHAR m_Left = VK_LEFT;
    TCHAR m_Right = VK_RIGHT;
    TCHAR m_God = 'G';


    //Other
    static const int CLIP_WIDTH = 51;
    static const int CLIP_HEIGHT = 61;
    static const int ROWS = 6;
    static const int COLS = 6;
    static const int MAX_JUMPS = 1;
    static const int GODMODE_SPEED = 850;
    static const int MAX_TRAIL = 80;
    static const int MAX_LIFES = 1;
    static const int JUMPSPEEDOFFSET = 50;
    static const int IDLEFRAMERATE = 4;
    double m_FrameRate = 8;

    int m_JumpCounter = 0;
    int m_FrameNr = 0;
    int m_RectangleWidth = 20;

    int m_Lifes = 1;
    int m_Deaths = 0;
    int m_AmountOfCoinsArr[3];
    double m_AccuTime = 0;
    double m_Speed = 350;
    double m_JumpHeight = 460;
    bool m_IsEpicModeOn = false;
    //Sounds
    Sound *m_SndJumpPtr = nullptr, *m_SndAttackPtr = nullptr, *m_SndWalkPtr = nullptr;


    bool m_Mirror = false;
    PhysicsActor* m_ActTriggerPtr = nullptr;
    Bitmap* m_BmpPtr = nullptr;
    Bitmap* m_BmpEpicModePtr = nullptr;
    int m_NumberOfContactPointsWithLevel = 0;

    
    std::deque<DOUBLE2> m_deqTrail;
    moveState m_moveState = moveState::STANDING;
};

 
