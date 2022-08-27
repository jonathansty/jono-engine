#pragma once

#include "singleton.h"
#include "EnkiTS/TaskScheduler.h"

#include "Box2DDebugRenderer.h"

#include "debug_overlays/MetricsOverlay.h"
#include "debug_overlays/OverlayManager.h"

#include "CommandLine.h"

#include "Graphics/2DRenderContext.h"
#include "Graphics/Renderer.h"

#include "GameSettings.h"
#include "PlatformIO.h"
#include "Framework/World.h"
#include "Graphics/RenderWorld.h"
#include "EngineSettings.h"
#include "Graphics/Perf.h"


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

class RenderThread : public Threading::Thread
{

public:
	RenderThread() 
		: Thread()
		, _stage(Stage::Initialization)
	{
	}

	~RenderThread() {}

	enum class Stage
	{
		Initialization, // During this stage all D3D objects are being setup
		Running, // During this stage we are processing frames and running the game
		Cleanup, // During this stage we are doing cleanup
		Terminated
	};

	// #TODO: Rather than spin locking we should wait for a specific condition variable for each stage to be fired?
	void wait_for_stage(Stage stage) 
	{
		while(_stage != stage)
		{
			Sleep(10);
		}
	}

	void run() override;

private:
	std::atomic<Stage> _stage;

};


class GameEngine : public TSingleton<GameEngine>, public b2ContactListener
{
private:
	GameEngine();
	friend class TSingleton<GameEngine>;

	// #TODO: Generalise render interface into 2D and 3D
	friend class BitmapComponent;
	friend class framework::Entity;

public:
	virtual ~GameEngine();

	GameEngine(const GameEngine&) = delete;
	GameEngine& operator=(const GameEngine&) = delete;

	// entry point to run a specific game
	static int run_game(HINSTANCE hInstance, cli::CommandLine const& cmdLine, int iCmdShow, unique_ptr<class AbstractGame>&& game);

public:
	std::shared_ptr<IO::IPlatformIO> io() const { return _platform_io; };

	// Quits the game
	void quit_game();

	// Box2D virtual overloads
	virtual void BeginContact(b2Contact* contactPtr);
	virtual void EndContact(b2Contact* contactPtr);
	virtual void PreSolve(b2Contact* contact, const b2Manifold* oldManifold);
	virtual void PostSolve(b2Contact* contact, const b2ContactImpulse* impulse);

	//! Darkens the output en displays the physics debug rendering
	//! @param when true it draws the physicsdebug rendering
	void enable_physics_debug_rendering(bool enable);

	// Input methods

	unique_ptr<InputManager> const& get_input() const { return _input_manager; };

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
	HINSTANCE get_instance() const;
	HWND get_window() const;
	string get_title() const;
	WORD get_icon() const;
	WORD get_small_icon() const;
	ImVec2 get_viewport_size(int id = 0) const;
	ImVec2 get_viewport_pos(int id = 0) const;
	ImVec2 get_window_size() const;
	int get_width() const;
	int get_height() const;
	bool get_sleep() const;

	// Mouse position relative to the viewport position (Should be used in most cases for game logic)
	float2 get_mouse_pos_in_viewport() const;

	// Mouse position relative to the window
	float2 get_mouse_pos_in_window() const;

	//! returns pointer to the Audio object
	unique_ptr<XAudioSystem> const& get_audio_system() const;
	//! returns pointer to the box2D world object
	shared_ptr<b2World> const& GetBox2DWorld() const { return _b2d_world; }

	void set_icon(WORD wIcon);
	void set_small_icon(WORD wSmallIcon);
	void apply_settings(GameSettings& gameSettings);

	void set_vsync(bool vsync);
	bool get_vsync();

	// The overlay manager manages all active IMGUI debug overlays
	shared_ptr<OverlayManager> const& get_overlay_manager() const;

	// Task scheduler used during resource loading
	static enki::TaskScheduler* s_TaskScheduler;
	static std::thread::id s_main_thread;

	// Enables/disables physics simulation stepping.
	void set_physics_step(bool bEnabled);

	bool is_viewport_focused() const;
	bool is_input_captured() const;


	// Returns the platform IO interface
	shared_ptr<IO::IPlatformIO> get_io() const { return _platform_io; }

	// Returns the current render world
	std::shared_ptr<RenderWorld> get_render_world() const { return _render_world; }

	// Returns the current game world
	std::shared_ptr<framework::World> get_world() const { return _world; }

	enum class BuildMenuOrder
	{
		First,
		Last
	};
	void set_build_menu_callback(std::function<void(BuildMenuOrder)> fn) { _build_menu = fn; }

	IDWriteFactory* GetDWriteFactory() const { return _renderer->get_raw_dwrite_factory(); }

private:
	// Internal run function called by GameEngine::run
	int run(HINSTANCE hInstance, int iCmdShow);

	// Sets the current game implementation
	void set_game(unique_ptr<AbstractGame>&& gamePtr);

	// Sets the current command line
	void set_command_line(cli::CommandLine const& cmdLine) { _command_line = cmdLine; }

	// Registers our WND class using the Win32 API
	bool register_wnd_class();

	// Opens the registered window. Should be called after wnd class has been registered
	bool open_window(int iCmdShow);

	LRESULT handle_event(HWND hWindow, UINT msg, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK wndproc(HWND hWindow, UINT msg, WPARAM wParam, LPARAM lParam);

	void set_sleep(bool bSleep);

	void set_title(const string& titleRef);

	void set_width(int iWidth);
	void set_height(int iHeight);
	void enable_vsync(bool bEnable = true);

	void enable_aa(bool isEnabled);

#if FEATURE_D2D
	// Direct2D methods
	void d2d_render();
#endif

	void d3d_init();
	void d3d_deinit();

	// Trigger Contacts are stored as pairs in a std::vector.
	// Iterates the vector and calls the ContactListeners
	// This for begin and endcontacts
	void CallListeners();

	void build_ui();

	void render();
	void present();

	// Renders the main view
	void render_view(Graphics::RenderPass::Value pass);

	void build_debug_log();
	void build_viewport();
	void build_menubar();


	// State for debug tools
	bool _show_debuglog;
	bool _show_viewport;
	bool _show_imgui_demo;
	bool _show_implot_demo;
	bool _show_entity_editor;

	// Callback for applications to create custom menu
	std::function<void(BuildMenuOrder)> _build_menu;

private:

	struct GpuTimer
	{
		enum Enum
		{
			Frame,
			Count
		};
	};

	std::array<Perf::Timer, 50> m_GpuTimings;

	cli::CommandLine _command_line;

	// Member Variables
	HINSTANCE _hinstance;
	HWND      _hwindow;
	string    _title;
	WORD      _icon, _small_icon;
	int       _window_width, _window_height;
	bool      _should_sleep;

	unique_ptr<AbstractGame> _game;

	// DirectX resources
	bool _initialized;

	// Fonts used for text rendering
	shared_ptr<Font> _default_font;

	// Box2D
	shared_ptr<b2World> _b2d_world;
	shared_ptr<b2ContactFilter> _b2d_contact_filter;

	double _b2d_time = 0;
#if FEATURE_D2D
	Box2DDebugRenderer _b2d_debug_renderer;
#endif
	std::vector<ContactData> _b2d_begin_contact_data;
	std::vector<ContactData> _b2d_end_contact_data;
	std::vector<ImpulseData> _b2d_impulse_data;
	double _physics_timestep = 1 / 60.0f;
	float2 _gravity;

	// Systems
	unique_ptr<PrecisionTimer> _game_timer;
	unique_ptr<InputManager> _input_manager;
	unique_ptr<XAudioSystem> _xaudio_system = nullptr;

	MetricsOverlay* _metrics_overlay;
	std::shared_ptr<OverlayManager> _overlay_manager;
	GameSettings _game_settings;
	EngineSettings _engine_settings;

#if FEATURE_D2D
	Graphics::D2DRenderContext* _d2d_ctx;
#endif

	std::shared_ptr<IO::IPlatformIO> _platform_io;

	// Game world and render world should be in sync!
	std::shared_ptr<RenderWorld>      _render_world;
	std::shared_ptr<framework::World> _world;

	// Handle to the render thread. Owned by the game engine
	std::unique_ptr<RenderThread> _render_thread;

	// State
	bool _can_paint;
	bool _vsync_enabled;
	bool _debug_physics_rendering;
	bool _is_viewport_focused;
	bool _recreate_game_texture;
	bool _recreate_swapchain;
	bool _physics_step_enabled;
	bool _running;

	u32    _viewport_width;
	u32    _viewport_height;
	float2 _viewport_pos;

	ImGuiID _viewport_id;

	std::shared_ptr<Graphics::Renderer> _renderer;
	std::unique_ptr<Graphics::D2DRenderContext> _d2d_render_context;

	friend class MetricsOverlay;
};

