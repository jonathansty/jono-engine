#pragma once

#include "singleton.h"

#include "EnkiTS/TaskScheduler.h"

#include "Box2DDebugRenderer.h"

#include "debug_overlays/MetricsOverlay.h"
#include "debug_overlays/OverlayManager.h"

#include "CommandLine.h"

#include "graphics/2DRenderContext.h"

#include "GameSettings.h"
#include "PlatformIO.h"
#include "framework/World.h"
#include "Graphics/RenderWorld.h"

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

#if FEATURE_D2D
using graphics::bitmap_interpolation_mode;
#endif

enum class MSAAMode {
	Off,
	MSAA_2x,
	MSAA_4x,
};

	struct RenderPass {
	enum Value {
		ZPrePass,
		Opaque,
		Shadow,
		Post

	};

	static std::string ToString(RenderPass::Value pass) {
		switch (pass) {
			case Value::ZPrePass:
				return "ZPrePass";
			case Value::Opaque:
				return "Opaque";
			case Value::Post:
				return "Post";
			case Value::Shadow:
				return "Shadow";
			default:
				return "Invalid";
				break;
		}
	}
};

struct ViewParams {
	float4x4 view;
	float4x4 proj;
	float3 view_direction;
	D3D11_VIEWPORT viewport;
	RenderPass::Value pass;
	bool reverse_z = true;
};


struct EngineSettings {
	// Allow 2D rendering
	bool d2d_use = false;
	bool d2d_use_aa = false;

	// Allow 3D rendering
	bool d3d_use = true;
	MSAAMode d3d_msaa_mode = MSAAMode::Off;

	f64 max_frame_time = 0.0;
	
};

template <typename T>
HRESULT set_debug_name(T *obj, std::string const &n) {
	return obj->SetPrivateData(WKPDID_D3DDebugObjectName, UINT(n.size()), n.data());
}


namespace framework {
class Entity;
}

class GameEngine : public TSingleton<GameEngine>, public b2ContactListener {

private:
	GameEngine();
	friend class TSingleton<GameEngine>;

	// #TODO: Generalise render interface into 2D and 3D
	friend class BitmapComponent;
	friend class framework::Entity;

public:
	virtual ~GameEngine();

	GameEngine(const GameEngine &) = delete;
	GameEngine &operator=(const GameEngine &) = delete;

	// entry point to run a specific game
	static int run_game(HINSTANCE hInstance, cli::CommandLine const &cmdLine, int iCmdShow, unique_ptr<class AbstractGame>&& game);

public:
	std::shared_ptr<IO::IPlatformIO> io() const { return _platform_io; };

	// Quits the game
	void quit_game();

	// Box2D virtual overloads
	virtual void BeginContact(b2Contact *contactPtr);
	virtual void EndContact(b2Contact *contactPtr);
	virtual void PreSolve(b2Contact *contact, const b2Manifold *oldManifold);
	virtual void PostSolve(b2Contact *contact, const b2ContactImpulse *impulse);

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

	//! ConsolePrintString: Display a String on the current cursor position. A new line is appended automatically
	void print_string(std::string const &msg);

	// Accessor Methods
	HINSTANCE get_instance() const;
	HWND      get_window() const;
	string    get_title() const;
	WORD      get_icon() const;
	WORD      get_small_icon() const;
	ImVec2    get_viewport_size(int id = 0) const;
	ImVec2    get_window_size() const;
	int       get_width() const;
	int       get_height() const;
	bool      get_sleep() const;


	ID3D11Device*           GetD3DDevice() const { return _d3d_device; };
	ID3D11DeviceContext*    GetD3DDeviceContext() const { return _d3d_device_ctx; };
	ID3D11RenderTargetView* GetD3DBackBufferView() const { return _d3d_backbuffer_view; };
	IWICImagingFactory*     GetWICImagingFactory() const { return _wic_factory; };
#if FEATURE_D2D
	ID2D1Factory*           GetD2DFactory() const { return _d2d_factory; };
	ID2D1RenderTarget*      GetHwndRenderTarget() const { return _d2d_rt; };
#endif

	IDWriteFactory*         GetDWriteFactory() const { return _dwrite_factory; };

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
	void apply_settings(GameSettings &gameSettings);

	void set_vsync(bool vsync);
	bool get_vsync();

	// The overlay manager manages all active IMGUI debug overlays
	shared_ptr<OverlayManager> const &get_overlay_manager() const;

	// Task scheduler used during resource loading
	static enki::TaskScheduler* s_TaskScheduler;
	static std::thread::id s_main_thread;

	// Enables/disables physics simulation stepping.
	void set_physics_step(bool bEnabled);

	bool is_viewport_focused() const;

	ID2D1RenderTarget *get_2d_draw_ctx() const { return _d2d_rt; }


	// Returns the platform IO interface
	shared_ptr<IO::IPlatformIO> get_io() const { return _platform_io; }

	// Returns the current render world
	std::shared_ptr<RenderWorld> get_render_world() const { return _render_world; }

	// Returns the current game world 
	std::shared_ptr<framework::World> get_world() const { return _world; }

	enum class BuildMenuOrder {
		First,
		Last
	};
	void set_build_menu_callback(std::function<void(BuildMenuOrder)> fn) { _build_menu = fn; }

private:
	// Internal run function called by GameEngine::run
	int run(HINSTANCE hInstance, int iCmdShow);

	// Sets the current game implementation
	void set_game(unique_ptr<AbstractGame>&& gamePtr);

	// Sets the current command line
	void set_command_line(cli::CommandLine const &cmdLine) { _command_line = cmdLine; }

	// Registers our WND class using the Win32 API
	bool register_wnd_class();

	// Opens the registered window. Should be called after wnd class has been registered
	bool open_window(int iCmdShow);

	// resizes or creates the swapchain and game rtv/dsv outputs
	void resize_swapchain(uint32_t width, uint32_t height);

	LRESULT handle_event(HWND hWindow, UINT msg, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK WndProc(HWND hWindow, UINT msg, WPARAM wParam, LPARAM lParam);

	void set_sleep(bool bSleep);

	void set_title(const string &titleRef);

	void set_width(int iWidth);
	void set_height(int iHeight);
	void enable_vsync(bool bEnable = true);

	void enable_aa(bool isEnabled);

#if FEATURE_D2D
	// Direct2D methods
	void d2d_render();
	void d2d_create_factory();
#endif

	void create_factories();
	void WIC_create_factory();
	void write_create_factory();

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
	void render_view(RenderPass::Value pass);

	// Allows rendering the world with customizable view parameters
	void render_world(ViewParams const& params);

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

	struct GpuTimingData
	{
		ComPtr<ID3D11Query> m_DisjointQuery;
		ComPtr<ID3D11Query> m_StartQuery;
		ComPtr<ID3D11Query> m_EndQuery;
	};
	std::array<std::array<GpuTimingData,GpuTimer::Count>,2> m_GpuTimings;

	cli::CommandLine _command_line;

	// Member Variables
	HINSTANCE _hinstance;
	HWND _hwindow;
	string _title;
	WORD _icon, _small_icon;
	int _window_width, _window_height;
	bool _should_sleep;

	unique_ptr<AbstractGame> _game;

	// DirectX resources
	bool _initialized;
	DXGI_FORMAT _swapchain_format;
	IDXGIFactory *_dxgi_factory;
	ID3D11Device *_d3d_device;
	ID3D11DeviceContext *_d3d_device_ctx;
	IDXGISwapChain *_dxgi_swapchain;
	ID3D11RenderTargetView *_d3d_backbuffer_view;
	ID3D11ShaderResourceView *_d3d_backbuffer_srv;
	ID3DUserDefinedAnnotation *_d3d_user_defined_annotation;

	// Intermediate MSAA game output.
	// these textures get resolved to the swapchain before presenting
	ID3D11Texture2D *_d3d_output_tex;
	ID3D11ShaderResourceView *_d3d_output_srv;
	ID3D11Texture2D* _d3d_non_msaa_output_tex;
	ID3D11ShaderResourceView* _d3d_non_msaa_output_srv;
	ID3D11RenderTargetView *_d3d_output_rtv;
	ID3D11Texture2D *_d3d_output_depth;
	ID3D11DepthStencilView *_d3d_output_dsv;

	ComPtr<ID3D11Texture2D> _shadow_map;
	ComPtr<ID3D11DepthStencilView> _shadow_map_dsv;
	ComPtr<ID3D11ShaderResourceView> _shadow_map_srv;


	// Game viewport resources
	ID2D1Factory *_d2d_factory;
	IWICImagingFactory *_wic_factory;
	ID2D1RenderTarget *_d2d_rt;
	IDWriteFactory *_dwrite_factory;

	ID2D1SolidColorBrush *_color_brush;
	DXGI_SAMPLE_DESC _aa_desc;
	D2D1_ANTIALIAS_MODE _d2d_aa_mode;

	float3x3 _mat_world;
	float3x3 _mat_view;

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
	unsigned long _frame_cnt = 0;

	// Systems
	unique_ptr<PrecisionTimer> _game_timer;
	unique_ptr<InputManager> _input_manager;
	unique_ptr<XAudioSystem> _xaudio_system = nullptr;

	MetricsOverlay *_metrics_overlay;
	std::shared_ptr<OverlayManager> _overlay_manager;
	GameSettings _game_settings;
	EngineSettings _engine_settings;

	#if FEATURE_D2D
	graphics::D2DRenderContext *_d2d_ctx;
	#endif

	std::shared_ptr<IO::IPlatformIO> _platform_io;

	// Game world and render world should be in sync!
	std::shared_ptr<RenderWorld> _render_world;
	std::shared_ptr<framework::World> _world;

	static constexpr u32 MAX_LIGHTS = 4;
	struct LightInfo {
		float4 colour;
		float4 direction;
		float4x4 light_space;
	};

	struct AmbientInfo {
		float4 ambient;
	};

	struct GlobalDataCB {
		float4x4 view;
		float4x4 inv_view;
		float4x4 proj;

		float4 view_direction;

		AmbientInfo ambient;

		LightInfo lights[MAX_LIGHTS];
		u32 num_lights;
	};
	ConstantBufferRef _cb_global;

	__declspec(align(16)) 
	struct DebugCB {
		unsigned int m_VisualizeMode;
		uint8_t pad[12];
	};
	ConstantBufferRef _cb_debug;


	// State
	bool _can_paint;
	bool _vsync_enabled;
	bool _debug_physics_rendering;
	bool _is_viewport_focused;
	bool _recreate_game_texture;
	bool _recreate_swapchain;
	bool _physics_step_enabled;
	bool _running;

	u32 m_ViewportWidth;
	u32 m_ViewportHeight;
	float2 m_ViewportPos;

	ImGuiID _viewport_id;
	friend class SceneViewer;
};

namespace Perf
{

// https://blat-blatnik.github.io/computerBear/making-accurate-sleep-function/
inline void precise_sleep(f64 seconds)
{
	using namespace std;
	using namespace std::chrono;

	static f64  estimate = 5e-3;
	static f64  mean = 5e-3;
	static f64  m2 = 0;
	static int64_t count = 1;

	while (seconds > estimate)
	{
		auto start = high_resolution_clock::now();
		this_thread::sleep_for(milliseconds(1));
		auto end = high_resolution_clock::now();

		f64 observed = (end - start).count() / 1e9;
		seconds -= observed;

		++count;
		double delta = observed - mean;
		mean += delta / count;
		m2 += delta * (observed - mean);
		f64 stddev = sqrt(m2 / (count - 1));
		estimate = mean + stddev;
	}

	// spin lock
	auto start = high_resolution_clock::now();
	while ((high_resolution_clock::now() - start).count() / 1e9 < seconds);
}

}

// Helper class for scoped gpu debugging
