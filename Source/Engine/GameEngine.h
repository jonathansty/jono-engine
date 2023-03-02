#pragma once

#include "singleton.h"
#include "Box2DDebugRenderer.h"
#include "CommandLine.h"
#include "GlobalContext.h"
#include "GameSettings.h"
#include "PlatformIO.h"
#include "Framework/World.h"
#include "EngineCfg.h"
#include "EnkiTS/TaskScheduler.h"
#include "Graphics/Perf.h"
#include "Graphics/RenderWorld.h"
#include "Graphics/GraphicsThread.h"
#include "Graphics/2DRenderContext.h"
#include "Graphics/Renderer.h"
#include "Debug/MetricsOverlay.h"
#include "Debug/OverlayManager.h"
#include <semaphore>
#include "Core/SmartPtr.h"


class Bitmap;
class Font;
class InputManager;
class XAudioSystem;
class AbstractGame;
class XAudioSystem;
class PrecisionTimer;
class b2World;
class ContactListener;
struct ImVec2;
struct SDL_Window;

namespace Perf
{
class Timer;
}


#if FEATURE_D2D
using Graphics::bitmap_interpolation_mode;
#endif

namespace framework
{
class Entity;
}

class ENGINE_API GameEngine : public TSingleton<GameEngine>, public b2ContactListener
{
	friend class TSingleton<GameEngine>;
	friend class Graphics::Renderer;
	friend class GraphicsThread;

public:
	static constexpr double c_FixedPhysicsTimestep = 1.0 / 60.0;

	GameEngine();
	virtual ~GameEngine();

	GameEngine(const GameEngine&) = delete;
	GameEngine& operator=(const GameEngine&) = delete;

	// entry point to run a specific game
	static int Run(HINSTANCE hInstance, cli::CommandLine const& cmdLine, int iCmdShow, unique_ptr<class AbstractGame>&& game);

public:
	bool Startup();
	void Update(f64 dt);
	void Shutdown();

	// Quits the game
	void Quit();

	// Box2D virtual overloads
	virtual void BeginContact(b2Contact* contactPtr);
	virtual void EndContact(b2Contact* contactPtr);
	virtual void PreSolve(b2Contact* contact, const b2Manifold* oldManifold);
	virtual void PostSolve(b2Contact* contact, const b2ContactImpulse* impulse);

	//! Darkens the output en displays the physics debug rendering
	//! @param when true it draws the physicsdebug rendering
	void SetPhysicsDebugRendering(bool enable);

	// Accessor Methods
	string GetWindowTitle() const;
	ImVec2 GetViewportSize(int id = 0) const;
	ImVec2 GetViewportPos(int id = 0) const;
	ImVec2 GetWindowSize() const;
	int	   GetWindowWidth() const;
	int    GetWindowHeight() const;

	bool WantCaptureMouse() const;
	bool WantCaptureKeyboard() const;

	// Mouse position relative to the viewport position (Should be used in most cases for game logic)
	float2 GetMousePosInViewport() const;

	// Mouse position relative to the window
	float2 GetMousePosInWindow() const;

	inline unique_ptr<XAudioSystem> const& GetAudioSystem() const { return m_AudioSystem; }
	inline shared_ptr<b2World> const& GetBox2DWorld() const { return m_Box2DWorld; }

	void ApplyGameCfg(GameCfg& gameSettings);

	void SetVSyncEnabled(bool vsync);
	bool GetVSyncEnabled();

	// The overlay manager manages all active IMGUI debug overlays
	shared_ptr<OverlayManager> const& get_overlay_manager() const;

	// Enables/disables physics simulation stepping.
	void SetPhysicsStep(bool bEnabled);

	bool IsViewportFocused() const;
	bool IsInputCaptured() const;


	// Returns the platform IO interface
	shared_ptr<IO::IPlatformIO> get_io() const { return m_PlatformIO; }

	// Returns the current render world
	std::shared_ptr<RenderWorld> get_render_world() const { return m_RenderWorld; }

	// Returns the current game world
	std::shared_ptr<framework::World> get_world() const { return m_World; }

	enum class BuildMenuOrder
	{
		First,
		Last
	};
	void set_build_menu_callback(std::function<void(BuildMenuOrder)> fn) { _build_menu = fn; }

	IDWriteFactory* GetDWriteFactory() const { return m_Renderer->get_raw_dwrite_factory(); }

	SDL_Window* GetWindow() const { return m_Window; }

	ImGuiID GetPropertyDockID() const { return m_PropertyDockID; }

private:
	// Sets the current game implementation
	void set_game(unique_ptr<AbstractGame>&& gamePtr);

	// Sets the current command line
	void set_command_line(cli::CommandLine const& cmdLine) { m_CommandLine = cmdLine; }

	// Opens the registered window. Should be called after wnd class has been registered
	bool InitSubSystems();
	bool InitWindow(int iCmdShow);

	void DeInitSubSystems();

	void ProcessEvent(SDL_Event& e);

	void SetWindowTitle(const string& titleRef);

#if FEATURE_D2D
	// Direct2D methods
	void RenderD2D();
#endif

	// Trigger Contacts are stored as pairs in a std::vector.
	// Iterates the vector and calls the ContactListeners
	// This for begin and endcontacts
	void CallListeners();

	void Sync();

	void BuildEditorUI();
	void BuildDebugLogUI(ImGuiID* dockID = nullptr);
	void BuildViewportUI(ImGuiID* dockID = nullptr);
    void BuildContentBrowserUI(ImGuiID* dockID = nullptr);
	void BuildMenuBarUI();


	// Callback for applications to create custom menu
	std::function<void(BuildMenuOrder)> _build_menu;

private:
	static constexpr const char* s_RootImguiID = "RootWindow##Root";

	static enki::TaskScheduler* s_TaskScheduler;
	static std::thread::id s_MainThreadID;

	struct GpuTimer
	{
		enum Enum
		{
			Frame,
			Count
		};
	};

	struct SDL_Window* m_Window;

	std::array<Perf::Timer, 50> m_GpuTimings;
	cli::CommandLine m_CommandLine;

	// Window properties
	string  m_WindowTitle;
	s32     m_WindowWidth;
	s32     m_WindowHeight;

	// Viewport properties
	u32     m_ViewportWidth;
	u32     m_ViewportHeight;
	float2  m_ViewportPos;

	// Imgui Window IDs
	ImGuiID m_ViewportImGuiID;
	ImGuiID m_DockImGuiID;
	ImGuiID m_PropertyDockID;

	// Fonts used for text rendering
	shared_ptr<Font> m_DefaultFont;

	// Box2D
	shared_ptr<b2World> m_Box2DWorld;
	shared_ptr<b2ContactFilter> m_Box2DContactFilter;
	double m_Box2DTime = 0;

#if FEATURE_D2D
	Box2DDebugRenderer m_Box2DDebugRenderer;
#endif

	std::vector<ContactData> m_Box2DBeginContactData;
	std::vector<ContactData> m_Box2DEndContactData;
	std::vector<ImpulseData> m_Box2DImpulseData;
	float2 m_Gravity;

	// Systems
	unique_ptr<PrecisionTimer> m_FrameTimer;
	unique_ptr<InputManager> m_InputManager;
	unique_ptr<XAudioSystem> m_AudioSystem = nullptr;

	MetricsOverlay* m_MetricsOverlay;
	std::shared_ptr<OverlayManager> m_OverlayManager;
	GameCfg   m_GameCfg;
	EngineCfg m_EngineCfg;

	std::shared_ptr<IO::IPlatformIO> m_PlatformIO;

	// Game world and render world should be in sync!
	std::shared_ptr<RenderWorld>      m_RenderWorld;

	std::shared_ptr<framework::World> m_World;
	unique_ptr<AbstractGame>          m_Game;

	SharedPtr<class Graphics::D2DRenderContext> m_D2DRenderContext;

	GraphicsThread m_GraphicsThread;
	std::binary_semaphore m_SignalMainToGraphics;
	std::binary_semaphore m_SignalGraphicsToMain;

	f64 m_TimeAccum;
	f64 m_TimeT;

	bool m_IsRunning : 1; 
	bool m_ShouldSleep : 1;
	bool m_CanPaint2D : 1;
	bool m_ViewportIsFocused : 1;
	bool m_VSyncEnabled : 1;
	bool m_DebugPhysicsRendering : 1;
	bool m_RecreateGameTextureRequested : 1;
	bool m_RecreateSwapchainRequested : 1; 
	bool m_PhysicsStepEnabled : 1; 

	// State for debug tools
	bool m_ShowViewport;
	bool m_ShowImguiDemo;
	bool m_ShowImplotDemo;
	bool m_ShowEntityEditor;


	SharedPtr<Graphics::Renderer> m_Renderer;

	friend class MetricsOverlay;
	friend class EngineLoop;
};

