#include "stdafx.h"	
#include "config.h"

#include "ElectronicJonaJoy.h"																				
#include "Game.h"
#include "StartMenu.h"
#include "LevelList.h"
#include "BitmapManager.h"
#include "SoundManager.h"
#include "HUD.h"
#include <time.h>
#include "Avatar.h"
#include "ejj_data_manager.h"

const std::string ElectronicJonaJoy::CONFIGPATH = std::string("Resources/cfg/config.xml");

ElectronicJonaJoy::ElectronicJonaJoy()
	: m_BeginTime()
	, m_EndTime()
{
}

ElectronicJonaJoy::~ElectronicJonaJoy()
{
	if (_current_state)
	{
		delete _current_state;
		_current_state = nullptr;
	}
}

void ElectronicJonaJoy::GameInitialize(GameSettings &gameSettings)
{
	gameSettings.m_WindowTitle = String("ElectronicJonaJoy - Steyfkens, Jonathan - 1DAE01");
	gameSettings.m_WindowWidth = 1280;
	gameSettings.m_WindowHeight = 720;
	gameSettings.m_WindowFlags |= GameSettings::WindowFlags::EnableVSync;
#ifdef _DEBUG
	gameSettings.m_WindowFlags |= GameSettings::WindowFlags::EnableConsole;
#endif
	gameSettings.m_WindowFlags |= GameSettings::WindowFlags::EnableAA;

}

void ElectronicJonaJoy::GameStart()
{
	char msg[256];
	sprintf_s(msg, "Electronic Jona Joy Version %d.%d.%d", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);
	game_engine::instance()->ConsolePrintString(String(msg));

	time_t beginTime;
	time(&beginTime);
	localtime_s(&m_BeginTime, &beginTime);
	m_CurrentLevel = 0;


	m_FileManagerPtr = new ejj_data_manager();
	m_LevelListPtr = m_FileManagerPtr->LoadLevels(CONFIGPATH);
	m_FileManagerPtr->LoadAvatarKeybinds(CONFIGPATH);

	// Start with the loading screen
	TransitionToState(new LoadingScreenState(this));
}

void ElectronicJonaJoy::GameEnd()
{
	_current_state->on_deactivate();
	safe_delete(_current_state);

	safe_delete(m_LevelListPtr);
	safe_delete(m_HUDPtr);
	safe_delete(m_Menu);
	safe_delete(m_FileManagerPtr);

	bitmap_manager::Shutdown();
	sound_manager::Shutdown();
}

void ElectronicJonaJoy::GameTick(double deltaTime)
{
	m_FramesPlayed++;
	m_AccuTime += deltaTime;
	HandleGameState();

	if (_current_state)
	{
		_current_state->update(deltaTime);
	}
	return;

	//Checks what gamestate we are currently in and executes it.
	switch (m_GameState)
	{
	case ElectronicJonaJoy::GameState::RUNNING:
		m_Game->tick(deltaTime);
		if (game_engine::instance()->IsKeyboardKeyPressed(VK_F3))
		{
			LoadNextLevel(m_Game.get());
		}
		if (game_engine::instance()->IsKeyboardKeyPressed(VK_F4))
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
		m_Game->tick(deltaTime);
		break;
	case ElectronicJonaJoy::GameState::QUIT:
		break;
	case ElectronicJonaJoy::GameState::LOADING:
		if (m_FramesPlayed > 2)
		{
			m_FileManagerPtr->LoadGameMusic(CONFIGPATH);
			m_Menu = new StartMenu();
			m_Menu->SetFileManager(m_FileManagerPtr);
			m_Menu->ReadKeyBindsForMenu(m_FileManagerPtr->LoadAvatarKeybinds(CONFIGPATH.c_str()));
			m_GameState = GameState::MENU;
		}
	default:
		break;
	}

	//Create pause menu on escape press.
	if (game_engine::instance()->IsKeyboardKeyPressed(VK_ESCAPE))
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
			LoadNextLevel(m_Game.get());
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
				m_FileManagerPtr->SaveGameResults(m_BeginTime, m_EndTime, m_Game.get());
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
	if (_current_state)
	{
		_current_state->render_2d();
	}
	return;

	switch (m_GameState)
	{
	case ElectronicJonaJoy::GameState::RUNNING:
		m_Game->paint();
		if (m_AccuTime < 5)
		{
			game_engine::instance()->set_default_font();
			game_engine::instance()->DrawString(String(m_LevelListPtr->GetLevel(m_CurrentLevel).c_str()), 10, game_engine::instance()->get_height() - 20);
		}
		m_HUDPtr->Paint();
		if (m_Game->GetGameOver())
		{
			m_HUDPtr->PaintGameOverWindow(DOUBLE2(game_engine::instance()->get_width() / 2, game_engine::instance()->get_height() / 2));
		}
		break;
	case ElectronicJonaJoy::GameState::MENU:
		game_engine::instance()->SetViewMatrix(MATRIX3X2::CreateIdentityMatrix());
		m_Menu->Paint();
		break;
	case ElectronicJonaJoy::GameState::PAUSE:
		m_Game->paint();
		m_HUDPtr->Paint();
		break;
	case ElectronicJonaJoy::GameState::QUIT:
		break;
	case ElectronicJonaJoy::GameState::LOADING:
		game_engine::instance()->DrawBitmap(m_BmpLoadingPtr);
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

void ElectronicJonaJoy::TransitionToState(IGameState* state)
{
	if (_current_state)
	{
		_current_state->on_deactivate();

		delete _current_state;
		_current_state = nullptr;
	}

	_current_state = state;
	_current_state->on_activate();

}

void ElectronicJonaJoy::LoadNextLevel(Game* game)
{
	m_AccuTime = 0;
	int amountOfLevels = m_LevelListPtr->GetAmountOfLevels();
	game->Restart();
	std::string toBeRemoved = m_LevelListPtr->GetLevel(m_CurrentLevel);
	m_CurrentLevel++;
	m_CurrentLevel = m_CurrentLevel%amountOfLevels;
	game->LoadLevel(m_LevelListPtr->GetLevel(m_CurrentLevel));
}

void ElectronicJonaJoy::HandleGameState()
{
	//if (m_Menu != nullptr && m_Menu->startPressed())
	//{
	//	time_t beginTime;
	//	time(&beginTime);
	//	localtime_s(&m_BeginTime, &beginTime);
	//	m_Menu->Remove();
	//	m_GameState = GameState::RUNNING;
	//	m_Game->LoadLevel(m_LevelListPtr->GetLevel(m_CurrentLevel));

	//	if (m_HUDPtr == nullptr)
	//	{
	//		m_HUDPtr = new HUD(m_Game.get());
	//		m_Game->LinkHUD(m_HUDPtr);
	//	}
	//	m_HUDPtr->ResetIsInMenu();
	//	m_HUDPtr->CreateSoundMuteBtn();
	//	//m_Game->Reset(m_LevelListPtr->GetLevel(m_CurrentLevel));
	//}

	if (m_GameState == GameState::PAUSE && m_HUDPtr->IsGoToStartMenu())
	{
		time_t endTime;
		time(&endTime);
		localtime_s(&m_EndTime, &endTime);
		m_GameState = GameState::MENU;
		m_FileManagerPtr->SaveGameResults(m_BeginTime, m_EndTime, m_Game.get());
		m_Game->Unload();
		m_HUDPtr->RemoveSoundMuteBtn();
		m_HUDPtr->RemovePauseMenu();
		m_Menu->EnableButtons();
		m_Menu->ReadKeyBindsForMenu(m_FileManagerPtr->GetKeyBinds());
		m_CurrentLevel = 0;
	}
}



MainMenuState::MainMenuState(ElectronicJonaJoy* owner)
	: _owner(owner)
{

}

void MainMenuState::on_activate()
{
	_owner->m_FileManagerPtr->LoadGameMusic(ElectronicJonaJoy::CONFIGPATH);
	_menu = new StartMenu();
	_menu->SetFileManager(_owner->m_FileManagerPtr);
	_menu->ReadKeyBindsForMenu(_owner->m_FileManagerPtr->LoadAvatarKeybinds(ElectronicJonaJoy::CONFIGPATH.c_str()));

	_menu->_on_start_event = [this]() {
		Game* game = new Game(_owner);
		_owner->TransitionToState(game);
	};
}

void MainMenuState::on_deactivate()
{
	delete _menu;
	_menu = nullptr;
}

void MainMenuState::update(double dt)
{
	_menu->Tick(dt);
}

void MainMenuState::render_2d()
{
	_menu->Paint();
}

MainMenuState::~MainMenuState()
{

}

LoadingScreenState::LoadingScreenState(ElectronicJonaJoy* owner)
	: _load_timer(1.0f)
	, _owner(owner)
{

}

void LoadingScreenState::on_activate()
{
	_loading_bitmap = bitmap_manager::instance()->LoadBitmapFile(String("Resources/Menu/LoadingScreen.png"));
}

void LoadingScreenState::update(double dt)
{
	_load_timer -= static_cast<float>(dt);
	if (_load_timer <= 0.0f)
	{
		_owner->TransitionToState(new MainMenuState(_owner));
	}
}

void LoadingScreenState::render_2d()
{
	game_engine::instance()->DrawBitmap(_loading_bitmap);
}
