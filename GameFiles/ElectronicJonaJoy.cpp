//-----------------------------------------------------------------
// Game File
// C++ Source - ElectronicJonaJoy.cpp - version v2_16 jan 2015
// Copyright DAE Programming Team
// http://www.digitalartsandentertainment.be/
//-----------------------------------------------------------------
#include "stdafx.h"		// this include must be the first include line of every cpp file (due to using precompiled header)
#include "config.h"
//-----------------------------------------------------------------
// Include Files
//-----------------------------------------------------------------
#include "ElectronicJonaJoy.h"																				
#include "Game.h"
#include "StartMenu.h"
#include "LevelList.h"
#include "BitmapManager.h"
#include "SoundManager.h"
#include "HUD.h"
#include <time.h>
#include "Avatar.h"
#include "FileManager.h"
//-----------------------------------------------------------------
// Defines
//-----------------------------------------------------------------
#define GAME_ENGINE (GameEngine::GetSingleton())
#define BITMAP_MANAGER (BitmapManager::GetSingleton())
#define SND_MANAGER (SoundManager::GetSingleton())
//-----------------------------------------------------------------
// ElectronicJonaJoy methods																				
//-----------------------------------------------------------------
const String ElectronicJonaJoy::CONFIGPATH = String("Resources/cfg/config.txt");
ElectronicJonaJoy::ElectronicJonaJoy()
{
	// nothing to create
}

ElectronicJonaJoy::~ElectronicJonaJoy()
{
	// nothing to destroy
}

void ElectronicJonaJoy::GameInitialize(GameSettings &gameSettings)
{
	gameSettings.SetWindowTitle(String("ElectronicJonaJoy - Steyfkens, Jonathan - 1DAE01"));
	gameSettings.SetWindowWidth(1280);
	gameSettings.SetWindowHeight(720);
	gameSettings.EnableConsole(false);
#if DEBUG | _DEBUG
	gameSettings.EnableConsole(true);
#endif
	gameSettings.EnableAntiAliasing(true);

}

void ElectronicJonaJoy::GameStart()
{
	char msg[256];
	sprintf_s(msg, "Electronic Jona Joy Version %d.%d.%d", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);
	GameEngine::Instance()->ConsolePrintString(String(msg));

	time_t beginTime;
	time(&beginTime);
	localtime_s(&m_BeginTime, &beginTime);
	m_BmpLoadingPtr = BITMAP_MANAGER->LoadBitmapFile(String("Resources/Menu/LoadingScreen.png"));
	m_CurrentLevel = 0;

	m_Game = new Game();

	m_FileManagerPtr = new FileManager();
	m_LevelListPtr = m_FileManagerPtr->LoadLevels(CONFIGPATH);
	m_FileManagerPtr->LoadAvatarKeybinds(CONFIGPATH);
	m_Game->SetFileManager(m_FileManagerPtr);
}


void ElectronicJonaJoy::GameEnd()
{
	delete m_Game;
	m_Game = nullptr;

	delete m_LevelListPtr;
	m_LevelListPtr = nullptr;

	delete m_HUDPtr;
	m_HUDPtr = nullptr;

	delete m_Menu;
	m_Menu = nullptr;

	delete m_FileManagerPtr;
	m_FileManagerPtr = nullptr;
	// DELETING SINGLETONS
	delete BitmapManager::GetSingleton();
	delete SoundManager::GetSingleton();

}

void ElectronicJonaJoy::GameTick(double deltaTime)
{
	m_FramesPlayed++;
	m_AccuTime += deltaTime;
	HandleGameState();

	//Checks what gamestate we are currently in and executes it.
	switch (m_GameState)
	{
	case ElectronicJonaJoy::GameState::RUNNING:
		m_Game->Tick(deltaTime);
		m_HUDPtr->SetTime(m_Game->GetAccuTime());
		m_HUDPtr->Tick(deltaTime);
		if (GAME_ENGINE->IsKeyboardKeyPressed(VK_F3))
		{
			LoadNextLevel();
		}
		if (GAME_ENGINE->IsKeyboardKeyPressed(VK_F4))
		{
			ReloadCurrentLevel();
		}
		break;
	case ElectronicJonaJoy::GameState::MENU:
		if (m_Menu != nullptr)
		{
			m_Menu->Tick(deltaTime);
		}
		break;
	case ElectronicJonaJoy::GameState::PAUSE:
		m_HUDPtr->Tick(deltaTime);
		m_Game->Tick(deltaTime);
		break;
	case ElectronicJonaJoy::GameState::QUIT:
		break;
	case ElectronicJonaJoy::GameState::LOADING:
		if (m_FramesPlayed > 2)
		{
			m_FileManagerPtr->LoadGameMusic(CONFIGPATH);
			m_Menu = new StartMenu();
			m_Menu->SetFileManager(m_FileManagerPtr);
			m_Menu->ReadKeyBindsForMenu(m_FileManagerPtr->LoadAvatarKeybinds(CONFIGPATH));
			m_GameState = GameState::MENU;
		}
	default:
		break;
	}

	//Create pause menu on escape press.
	if (GAME_ENGINE->IsKeyboardKeyPressed(VK_ESCAPE))
	{
		switch (m_GameState)
		{
		case ElectronicJonaJoy::GameState::RUNNING:
			m_HUDPtr->CreatePauseMenu();
			m_GameState = GameState::PAUSE;
			break;
		case ElectronicJonaJoy::GameState::MENU:
			break;
		case ElectronicJonaJoy::GameState::PAUSE:
			m_HUDPtr->RemovePauseMenu();
			m_GameState = GameState::RUNNING;
			break;
		case ElectronicJonaJoy::GameState::QUIT:
			break;
		default:
			break;
		}
	}

	// Returns us back to the menu when we reached the last level.
	if (m_Game != nullptr && m_Game->GetLevelEnd())
	{
		if (!(m_CurrentLevel + 2 > m_LevelListPtr->GetAmountOfLevels()))
		{
			m_FileManagerPtr->UpdateLastLevel(m_CurrentLevel + 1);
			LoadNextLevel();
		}
		else
		{
			//Check if we are already in the menu or not.
			if (m_GameState != GameState::MENU)
			{
				m_GameState = GameState::MENU;
				m_FileManagerPtr->UpdateLastLevel(m_CurrentLevel + 1);
				time_t endTime;
				time(&endTime);
				localtime_s(&m_EndTime, &endTime);
				m_FileManagerPtr->SaveGameResults(m_BeginTime, m_EndTime, m_Game);
				m_Menu->ReadGameResults();
				m_Game->Unload();
				m_HUDPtr->RemoveSoundMuteBtn();
				m_Menu->EnableButtons();

			}
		}

	}

}
void ElectronicJonaJoy::GamePaint(RECT rect)
{
	switch (m_GameState)
	{
	case ElectronicJonaJoy::GameState::RUNNING:
		m_Game->Paint();
		if (m_AccuTime < 5)
		{
			GAME_ENGINE->SetDefaultFont();
			GAME_ENGINE->DrawString(m_LevelListPtr->GetLevel(m_CurrentLevel), 10, GAME_ENGINE->GetHeight() - 20);
		}
		m_HUDPtr->Paint();
		if (m_Game->GetGameOver())
		{
			m_HUDPtr->PaintGameOverWindow(DOUBLE2(GAME_ENGINE->GetWidth() / 2, GAME_ENGINE->GetHeight() / 2));
		}
		break;
	case ElectronicJonaJoy::GameState::MENU:
		GAME_ENGINE->SetViewMatrix(MATRIX3X2::CreateIdentityMatrix());
		m_Menu->Paint();
		break;
	case ElectronicJonaJoy::GameState::PAUSE:
		m_Game->Paint();
		m_HUDPtr->Paint();
		break;
	case ElectronicJonaJoy::GameState::QUIT:
		break;
	case ElectronicJonaJoy::GameState::LOADING:
		GAME_ENGINE->DrawBitmap(m_BmpLoadingPtr);
		break;
	default:
		break;
	}
}

void ElectronicJonaJoy::DebugUI()
{
	// Do game level debug UI 

}

void ElectronicJonaJoy::ReloadCurrentLevel()
{
	m_AccuTime = 0;
	m_Game->LoadLevel(m_LevelListPtr->GetLevel(m_CurrentLevel));
}

void ElectronicJonaJoy::LoadNextLevel()
{
	m_AccuTime = 0;
	int amountOfLevels = m_LevelListPtr->GetAmountOfLevels();
	m_Game->Restart();
	String toBeRemoved = m_LevelListPtr->GetLevel(m_CurrentLevel);
	m_CurrentLevel++;
	m_CurrentLevel = m_CurrentLevel%amountOfLevels;
	m_Game->LoadLevel(m_LevelListPtr->GetLevel(m_CurrentLevel));
}

void ElectronicJonaJoy::HandleGameState()
{
	if (m_Menu != nullptr && m_Menu->startPressed())
	{
		time_t beginTime;
		time(&beginTime);
		localtime_s(&m_BeginTime, &beginTime);
		m_Menu->Remove();
		m_GameState = GameState::RUNNING;
		m_Game->LoadLevel(m_LevelListPtr->GetLevel(m_CurrentLevel));

		if (m_HUDPtr == nullptr)
		{
			m_HUDPtr = new HUD(m_Game);
			m_Game->LinkHUD(m_HUDPtr);
		}
		m_HUDPtr->ResetIsInMenu();
		m_HUDPtr->CreateSoundMuteBtn();
		//m_Game->Reset(m_LevelListPtr->GetLevel(m_CurrentLevel));
	}
	if (m_Menu != nullptr && m_Menu->quitPressed())
	{
		m_GameState = GameState::QUIT;

		GAME_ENGINE->QuitGame();
	}
	if (m_GameState == GameState::PAUSE && m_HUDPtr->IsGoToStartMenu())
	{
		time_t endTime;
		time(&endTime);
		localtime_s(&m_EndTime, &endTime);
		m_GameState = GameState::MENU;
		m_FileManagerPtr->SaveGameResults(m_BeginTime, m_EndTime, m_Game);
		m_Game->Unload();
		m_HUDPtr->RemoveSoundMuteBtn();
		m_HUDPtr->RemovePauseMenu();
		m_Menu->EnableButtons();
		m_Menu->ReadKeyBindsForMenu(m_FileManagerPtr->GetKeyBinds());
		m_CurrentLevel = 0;
	}
}



