//-----------------------------------------------------------------
// Game File
// C++ Source - ElectronicJonaJoy.h - version v2_16 jan 2015
// Copyright DAE Programming Team
// http://www.digitalartsandentertainment.be/
//-----------------------------------------------------------------

//-----------------------------------------------------------------
// Student data
// Name: Steyfkens, Jonathan
// Group: 1DAE01
//-----------------------------------------------------------------

#pragma once

//-----------------------------------------------------------------
// Include Files
//-----------------------------------------------------------------

#include "Resource.h"	
#include "AbstractGame.h"

//-----------------------------------------------------------------
// ElectronicJonaJoy Class																
//-----------------------------------------------------------------
class Game;
class HUD;
class StartMenu;
class LevelList;
class FileManager;
class ElectronicJonaJoy : public AbstractGame
{
public:				
	//---------------------------
	// Constructor(s)
	//---------------------------
	ElectronicJonaJoy();

	//---------------------------
	// Destructor
	//---------------------------
	virtual ~ElectronicJonaJoy();

	// C++11 make the class non-copyable
	ElectronicJonaJoy(const ElectronicJonaJoy&) = delete;
	ElectronicJonaJoy& operator=(const ElectronicJonaJoy&) = delete;

	//---------------------------
	// General Methods
	//---------------------------

	virtual void GameInitialize(GameSettings &gameSettings);
	virtual void GameStart();				
	virtual void GameEnd();
	virtual void GameTick(double deltaTime);
	virtual void GamePaint(RECT rect);
    virtual void DebugUI();
    void HandleGameState();
    void LoadNextLevel();
    void ReloadCurrentLevel();

    void SaveGameResults();
	// -------------------------
	// Public Member functions
	// -------------------------

private:
    static const std::string CONFIGPATH;
    Bitmap* m_BmpLoadingPtr = nullptr;
    tm m_BeginTime;
    tm m_EndTime;
    double m_LifeTime = 0;
    LevelList* m_LevelListPtr;
    double m_AccuTime = 0;
    int m_CurrentLevel = 0;
    String m_Level;
    Game* m_Game = nullptr;
    HUD* m_HUDPtr = nullptr;
    StartMenu* m_Menu = nullptr;
    FileManager* m_FileManagerPtr = nullptr;
    int m_FramesPlayed = 0;
    enum class GameState
    {
        RUNNING,
        MENU,
        PAUSE,
        LOADING,
        QUIT
    }m_GameState = GameState::LOADING;
};
