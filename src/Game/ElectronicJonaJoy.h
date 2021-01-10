#pragma once

#include "Resource.h"	
#include "AbstractGame.h"

class Game;
class HUD;
class StartMenu;
class LevelList;
class DataManager;

// Interface for defining a game state. This should *enable* easy state switching
interface IGameState
{
	virtual ~IGameState() = default;

    virtual void on_activate() { }
    virtual void on_deactivate() {}

    virtual void update(double dt) {}
    virtual void render_2d() {}
};

class ElectronicJonaJoy final : public AbstractGame
{
public:				
	ElectronicJonaJoy();
	virtual ~ElectronicJonaJoy();

	ElectronicJonaJoy(const ElectronicJonaJoy&) = delete;
	ElectronicJonaJoy& operator=(const ElectronicJonaJoy&) = delete;

	virtual void initialize(GameSettings &gameSettings) override;
	virtual void start() override;				
	virtual void end() override;
	virtual void tick(double deltaTime) override;
	virtual void paint(RECT rect) override;
    virtual void debug_ui() override;

    void HandleGameState();
    void LoadNextLevel(Game* game);
    void ReloadCurrentLevel();

    // Transition to a new state
    void TransitionToState(IGameState* state);

    void SaveGameResults();

    DataManager* GetFileManager() const { return m_FileManagerPtr; }

    static const std::string CONFIGPATH;

    int get_curr_level() const { return m_CurrentLevel; }
    LevelList* get_level_names() const { return m_LevelListPtr; }
private:
    IGameState* _current_state = nullptr;
    DataManager* m_FileManagerPtr = nullptr;
    Bitmap* m_BmpLoadingPtr = nullptr;

    tm m_BeginTime;
    tm m_EndTime;
    double m_LifeTime = 0;
    double m_AccuTime = 0;
    int m_CurrentLevel = 0;
    String m_Level;

    LevelList* m_LevelListPtr = nullptr;
    std::unique_ptr<Game> m_Game;
    HUD* m_HUDPtr = nullptr;
    StartMenu* m_Menu = nullptr;

    int m_FramesPlayed = 0;
    enum class GameState
    {
        RUNNING,
        MENU,
        PAUSE,
        LOADING,
        QUIT
    }m_GameState = GameState::LOADING;

    friend class MainMenuState;
};


// Create main menu state
class MainMenuState final : public IGameState
{
public:
	MainMenuState(ElectronicJonaJoy* owner);
    ~MainMenuState();

	void on_activate() override;

	void on_deactivate() override;

    void update(double dt) override;

    void render_2d() override;
private:
    StartMenu* _menu;
	ElectronicJonaJoy* _owner;
};

class LoadingScreenState final : public IGameState
{
public:
    LoadingScreenState(ElectronicJonaJoy* owner);
    ~LoadingScreenState() = default;

    virtual void on_activate() override;

    virtual void update(double dt) override;

    virtual void render_2d() override;

private:
    Bitmap* _loading_bitmap;
    float _load_timer;
	ElectronicJonaJoy * _owner;

};

