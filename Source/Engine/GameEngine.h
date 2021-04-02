#pragma once

#include "singleton.h"

#include "TaskScheduler.h"

#include "Box2DDebugRenderer.h"

#include "debug_overlays/MetricsOverlay.h"
#include "debug_overlays/OverlayManager.h"

#include "CommandLine.h"

#include "graphics/2DRenderContext.h"

#include "GameSettings.h"
#include "PlatformIO.h"

class Bitmap;
class Font;
class InputManager;
class AudioSystem;
class AbstractGame;
class AudioSystem;
class PrecisionTimer;
class b2World;
class ContactListener;


using graphics::bitmap_interpolation_mode;


enum class MSAAMode {
	Off,
	MSAA_2x,
	MSAA_4x,
};

struct EngineSettings {
	// Allow 2D rendering
	bool d2d_use = false;
	bool d2d_use_aa = false;

	// Allow 3D rendering
	bool d3d_use = true;
	MSAAMode d3d_msaa_mode = MSAAMode::Off;
	
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
	static int run_game(HINSTANCE hInstance, cli::CommandLine const &cmdLine, int iCmdShow, class AbstractGame *game);

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

	void set_bitmap_interpolation_mode(graphics::bitmap_interpolation_mode mode);

	//! Set the font that is used to render text
	void set_font(Font *fontPtr);

	//! Returns the font that is used to render text
	Font *get_font() const;

	//! Set the built-in font asthe one to be used to render text
	void set_default_font();

	//! Sets the color of the brush that is used to draw and fill
	//! Example: GameEngine::instance()->SetColor(COLOR(255,127,64));
	void set_color(COLOR colorVal);

	//! Returns the color of the brush used to draw and fill
	//! Example: COLOR c = GameEngine::instance()->GetColor();
	COLOR get_color();

	// Accessor Methods
	HINSTANCE get_instance() const;
	HWND get_window() const;
	string get_title() const;
	WORD get_icon() const;
	WORD get_small_icon() const;
	ImVec2 get_viewport_size(int id = 0) const;
	ImVec2 get_window_size() const;
	int get_width() const;
	int get_height() const;
	bool get_sleep() const;

	ID3D11Device *GetD3DDevice() const;
	ID3D11DeviceContext *GetD3DDeviceContext() const;
	ID3D11RenderTargetView *GetD3DBackBufferView() const;

	ID2D1Factory *GetD2DFactory() const;
	IWICImagingFactory *GetWICImagingFactory() const;
	ID2D1RenderTarget *GetHwndRenderTarget() const;
	IDWriteFactory *GetDWriteFactory() const;

	// Returns a POINT containing the window coordinates of the mouse offset in the viewport
	// Usage example:
	// POINT mousePos = GameEngine::instance()->GetMousePosition();
	float2 get_mouse_pos_in_viewport() const;

	//! returns pointer to the Audio object
	unique_ptr<AudioSystem> const& GetXAudio() const;
	//! returns pointer to the box2D world object
	b2World *GetBox2DWorld() { return _b2d_world; }

	void set_icon(WORD wIcon);
	void set_small_icon(WORD wSmallIcon);
	void apply_settings(GameSettings &gameSettings);

	void set_vsync(bool vsync);
	bool get_vsync();

	// The overlay manager manages all active IMGUI debug overlays
	std::shared_ptr<OverlayManager> const &get_overlay_manager() const {
		return _overlay_manager;
	}

	static enki::TaskScheduler s_TaskScheduler;
	static std::thread::id s_main_thread;

	// Enables/disables physics simulation stepping.
	void set_physics_step(bool bEnabled);

	bool is_viewport_focused() const { return _is_viewport_focused; }

	ID2D1RenderTarget *get_2d_draw_ctx() const { return _d2d_rt; }

private:
	// Internal run function called by GameEngine::run
	int run(HINSTANCE hInstance, int iCmdShow);

	// Sets the current game implementation
	void set_game(AbstractGame *gamePtr);

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
	bool is_paint_allowed() const;

	// Direct2D methods
	void d2d_render();

	void create_factories();
	void d2d_create_factory();
	void WIC_create_factory();
	void write_create_factory();

	void d3d_init();
	void d3d_deinit();

	// Trigger Contacts are stored as pairs in a std::vector.
	// Iterates the vector and calls the ContactListeners
	// This for begin and endcontacts
	void CallListeners();

	void build_ui();

private:
	cli::CommandLine _command_line;

	// Member Variables
	HINSTANCE _hinstance;
	HWND _hwindow;
	string _title;
	WORD _icon, _small_icon;
	int _window_width, _window_height;
	bool _should_sleep;
	AbstractGame *_game;
	HANDLE _console;

	// DirectX resources
	bool _initialized;
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
	ID3D11RenderTargetView *_d3d_output_rtv;
	ID3D11Texture2D *_d3d_output_depth;
	ID3D11DepthStencilView *_d3d_output_dsv;


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
	Font *_default_font; 

	// Box2D
	b2World *_b2d_world = nullptr;
	b2ContactFilter *_b2d_contact_filter = nullptr;
	double _b2d_time = 0;
	Box2DDebugRenderer _b2d_debug_renderer;
	std::vector<ContactData> _b2d_begin_contact_data;
	std::vector<ContactData> _b2d_end_contact_data;
	std::vector<ImpulseData> _b2d_impulse_data;
	double _physics_timestep = 1 / 60.0f;
	float2 _gravity;
	unsigned long _frame_cnt = 0;

	// Systems
	unique_ptr<PrecisionTimer> _game_timer;
	unique_ptr<InputManager> _input_manager;
	unique_ptr<AudioSystem> _xaudio_system = nullptr;

	MetricsOverlay *_metrics_overlay;
	std::shared_ptr<OverlayManager> _overlay_manager;
	GameSettings _game_settings;
	EngineSettings _engine_settings;

	graphics::D2DRenderContext *_d2d_ctx;

	std::shared_ptr<IO::IPlatformIO> _platform_io;

	// State
	bool _can_paint;
	bool _vsync_enabled;
	bool _debug_physics_rendering;
	bool _is_viewport_focused;
	bool _recreate_game_texture;
	bool _recreate_swapchain;
	bool _physics_step_enabled;
	bool _running;
};

// Helper class for scoped gpu debugging
#ifdef _DEBUG
class scoped_gpu_event final {
public:
	scoped_gpu_event(ID3DUserDefinedAnnotation *annotation, std::wstring name) :
			_name(name), _annotation(annotation) {
		annotation->BeginEvent(name.c_str());
	}
	~scoped_gpu_event() {
		_annotation->EndEvent();
	}

private:
	std::wstring _name;
	ID3DUserDefinedAnnotation *_annotation;
};

#define COMBINE1(X, Y) X##Y // helper macro
#define COMBINE(X, Y) COMBINE1(X, Y)
#define GPU_SCOPED_EVENT(ctx, name) scoped_gpu_event COMBINE(perfEvent, __LINE__) = scoped_gpu_event(ctx, name)
#define GPU_MARKER(ctx, name) ctx->SetMarker(name);
#else
#define GPU_SCOPED_EVENT(ctx, name)
#define GPU_MARKER(ctx, name)
#endif
