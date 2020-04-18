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

#include "Framework/World.h"
#include "ElectronicJonaJoy.h"

class Avatar;
class Entity;
class HUD;
class Enemy;
class Level;
class Camera;
class Arrow;
class bitmap_manager;
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


class Game final : public IGameState
{
public:
	Game(ElectronicJonaJoy* owner);
	virtual ~Game();

	Game( const Game& ) = delete;
	Game& operator=( const Game& ) = delete;

    void on_activate() override;
    void on_deactivate() override;
	void render_2d() override;
	void update(double deltaTime) override;

    // Legacy code
    void paint();
    void tick(double deltaTime);
    void Pause();
    void UnPause();
    void Unload();
    void LinkHUD(HUD* hudPtr);
    void Reset(const std::string& fileName);
    bool GetGameOver();
    bool GetLevelEnd();
    void Restart();

    // Removes everything (lists)and initializes it again
    void Initializeall(const std::string& fileName);
    
    // Takes care of the not list objects
    void LoadLevel(const std::string& filePath);

    // Links a file manager with the game. Yuk!
    void SetFileManager(FileManager* tmpFileManager);

    Avatar* GetAvatar();
    Level* GetLevel();
    double GetAccuTime();
    int GetTotalDeaths();
    double GetTotalTime();

private: 
    void UpdateObjects(double deltaTime);
    void UpdateGameChecks(double deltaTime);
    void UpdateKeyChecks(double deltaTime);
    void UpdateDrawMode();

    void drawBackgroundGradient(int levels);

    double m_TimeMultiplier = 1;
    std::string m_Level;
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

    enum DrawMode : uint32_t
    {
        Physics = 0x1,
        Bitmap = 0x2
    };
	uint32_t m_DrawMode = DrawMode::Bitmap;

    // HUD pause menu
    // PhysicsActor setActive false
    enum class GameState
	{
        Running,
        Paused,
    };
	GameState m_GameState = GameState::Running;
    HUD* m_HudPtr = nullptr;


    // entity world
    std::shared_ptr<framework::World> m_World;

    // Point to the owner game, this is used by the game state system
    ElectronicJonaJoy* _owner;
};

 
