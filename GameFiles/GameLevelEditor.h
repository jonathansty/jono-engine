#pragma once
//-----------------------------------------------------
// Name: Steyfkens
// First name: Jonathan
// Group: 1DAE01
//-----------------------------------------------------

//-----------------------------------------------------
// Include Files
//-----------------------------------------------------

#include "ContactListener.h"

//-----------------------------------------------------
// Game Class									
//-----------------------------------------------------
class Avatar;
class Entity;
class HUD;
class Enemy;
class Level;
class Camera;
class Arrow;
class BitmapManager;
class EnemyShooter;
class CheckPoint;
class RotLight;
class EnemyRotater;
class EnemyHorizontal;
class Coin;
class CoinList;
class AttackBeamList;
class EntityList;
class Teleport;
class CheckPointBg;
class EnemyList;
class EnemyRocketLauncher;
class EnemyRocket;
class Animation;
class AnimationList;
class CombRotLightCpBg;
class LevelEnd;
class TriggerList;
class FileManager;
class Game //: public ContactListener
{
public:
	Game();
	virtual ~Game( );

	// C++11 make the class non-copGameable
	Game( const Game& ) = delete;
	Game& operator=( const Game& ) = delete;

	//--------------------------------------------------------
	// ContactListener overloaded member function declarations
	//--------------------------------------------------------
	//virtual void BeginContact(PhGamesicsActor *actThisPtr, PhGamesicsActor *actOtherPtr); 
	//virtual void EndContact(PhGamesicsActor *actThisPtr, PhGamesicsActor *actOtherPtr);   
	//virtual void ContactImpulse(PhGamesicsActor *actThisPtr, double impulse);
    void Paint();
    void Tick(double deltaTime);
    void Pause();
    void UnPause();
    void Unload();
    void LinkHUD(HUD* hudPtr);
    void Reset(const String& fileName);
    bool GetGameOver();
    bool GetLevelEnd();
    void Restart();

    /*
    * InitializeAll() : Removes everything (lists)and initializes it again
    * LoadLevel() : Takes care of the not list objects
    */
    void Initializeall(const String& fileName);
    
    void LoadLevel(const String& filePath);


    void SetFileManager(FileManager* tmpFileManager);
    Avatar* GetAvatar();
    Level* GetLevel();
    double GetAccuTime();
    int GetTotalDeaths();
    double GetTotalTime();
private: 
    // -------------------------
    // Private Member functions
    // -------------------------

    /*
    * Objects: Ticks every Object
    * GameChecks: Checks for game Logic
    * keyChecks = Keeps checking what keys are being pressed
    * drawMode: Everything related to gameState
    */
    void UpdateObjects(double deltaTime);
    void UpdateGameChecks(double deltaTime);
    void UpdateKeyChecks(double deltaTime);
    void UpdateDrawMode();

    void drawBackgroundGradient(int levels);

    // -------------------------
    // Private Datamembers
    // -------------------------
    double m_TimeMultiplier = 1;
    String m_Level;
    int m_TotalDeaths = 0;
    Avatar* m_AvatarPtr = nullptr;
    Level* m_LevelPtr = nullptr;
    Camera* m_CameraPtr = nullptr;

    FileManager* m_FileManagerPtr = nullptr;
    AnimationList* m_AnimationListPtr = nullptr;
    TriggerList* m_TriggerListPtr = nullptr;
    LevelEnd* m_LevelEndPtr = nullptr;
    CoinList* m_CoinListPtr = nullptr;
    AttackBeamList* m_AttackBeamListPtr = nullptr;
    EntityList* m_EntityListPtr = nullptr;
    CheckPoint* m_EntityLastHitCheckpointPtr = nullptr;
    EnemyList* m_EnemyListPtr = nullptr;
    CheckPointBg* m_CheckPointBgPtr = nullptr;
    RotLight* m_CheckPointRotLightPtr = nullptr;
    Sound* m_SndBgMusicPtr = nullptr;
    Sound* m_SndEpicModePtr = nullptr;


    double m_AccuTime = 0;
    double m_TotalTime = 0; 

    bool m_GameOver = false;
    bool m_LoadNextLevel = false;
    enum class DrawMode{
        PHYSICS,
        PHYSICS_BITMAP,
        BITMAP
    }m_DrawMode = DrawMode::BITMAP;

    // HUD pause menu
    // PhysicsActor setActive false
    enum class GameState{
        RUNNING,
        PAUSED,
    }m_GameState = GameState::RUNNING;
    HUD* m_HudPtr = nullptr;
};

 
