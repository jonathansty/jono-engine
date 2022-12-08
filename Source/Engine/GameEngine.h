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
	void enable_physics_debug_rendering(bool enable);

	// Input methods

	unique_ptr<InputManager> const& get_input() const { return m_InputManager; };

	//! Returns true when button is down and was down the previous GG
	//! Example values for key are: VK_LEFT, 'A'. ONLY CAPITALS.
	bool is_key_down(int key) const;
	//! Returns true when button is down and was up the previous frame
	//! Example values for key are: VK_LEFT, 'A'. ONLY CAPITALS.
	bool is_key_pressed(int key) const;
	//! Returns true when button is up and was down the previous frame
	//! Example values for key are: VK_LEFT, 'A'. ONLY CAPITALS.
	bool is_key_released(int key) const;

	//! Returns true when button is down and was down the previous frame
	//! Possible values for button are: VK_LBUTTON, VK_RBUTTON and VK_MBUTTON
	bool is_mouse_button_down(int button) const;

	//! Returns true when button is down and was up the previous frame
	//! Possible values for button are: VK_LBUTTON, VK_RBUTTON and VK_MBUTTON
	bool is_mouse_button_pressed(int button) const;

	//! Returns true when button is up and was down the previous frame
	//! Possible values for button are: VK_LBUTTON, VK_RBUTTON and VK_MBUTTON
	bool is_mouse_button_released(int button) const;

	// Accessor Methods
	string get_title() const;
	WORD get_icon() const;
	WORD get_small_icon() const;
	ImVec2 GetViewportSize(int id = 0) const;
	ImVec2 get_viewport_pos(int id = 0) const;
	ImVec2 GetWindowSize() const;
	int get_width() const;
	int get_height() const;
	bool get_sleep() const;

	bool WantCaptureMouse() const;
	bool WantCaptureKeyboard() const;

	// Mouse position relative to the viewport position (Should be used in most cases for game logic)
	float2 get_mouse_pos_in_viewport() const;

	// Mouse position relative to the window
	float2 get_mouse_pos_in_window() const;

	//! returns pointer to the Audio object
	unique_ptr<XAudioSystem> const& get_audio_system() const;
	//! returns pointer to the box2D world object
	shared_ptr<b2World> const& GetBox2DWorld() const { return m_Box2DWorld; }

	void set_icon(WORD wIcon);
	void set_small_icon(WORD wSmallIcon);
	void apply_settings(GameCfg& gameSettings);

	void set_vsync(bool vsync);
	bool get_vsync();

	// The overlay manager manages all active IMGUI debug overlays
	shared_ptr<OverlayManager> const& get_overlay_manager() const;



	// Enables/disables physics simulation stepping.
	void set_physics_step(bool bEnabled);

	bool is_viewport_focused() const;
	bool is_input_captured() const;


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

	struct SDL_Window* GetWindow() const { return m_Window; }

	ImGuiID GetPropertyDockID() const { return m_PropertyDockID; }

private:
	// Internal run function called by GameEngine::run
	int Run(HINSTANCE hInstance, int iCmdShow);

	// Sets the current game implementation
	void set_game(unique_ptr<AbstractGame>&& gamePtr);

	// Sets the current command line
	void set_command_line(cli::CommandLine const& cmdLine) { m_CommandLine = cmdLine; }

	// Opens the registered window. Should be called after wnd class has been registered
	bool InitSubSystems();
	bool InitWindow(int iCmdShow);

	void DeInitSubSystems();

	void ProcessEvent(SDL_Event& e);

	void set_sleep(bool bSleep);

	void set_title(const string& titleRef);

	void set_width(int iWidth);
	void set_height(int iHeight);
	void enable_vsync(bool bEnable = true);

	void enable_aa(bool isEnabled);

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

	string m_Title;
	WORD m_Icon, m_SmallIcon;

	s32     m_WindowWidth;
	s32     m_WindowHeight;
	u32     m_ViewportWidth;
	u32     m_ViewportHeight;
	float2  m_ViewportPos;
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
	bool m_ShowDebugLog;
	bool m_ShowViewport;
	bool m_ShowImguiDemo;
	bool m_ShowImplotDemo;
	bool m_ShowEntityEditor;


	SharedPtr<Graphics::Renderer> m_Renderer;

	friend class MetricsOverlay;
	friend class EngineLoop;
};

