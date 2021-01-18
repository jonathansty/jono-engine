//-----------------------------------------------------------------
// Game Engine Object
// C++ Header - version v2_16 jan 2015
// Copyright DAE Programming Team
// http://www.digitalartsandentertainment.be/
// Heavily modified by Jonathan Steyfkens.
//-----------------------------------------------------------------
#pragma once
#include "singleton.h"

#include "TaskScheduler.h"

#include "debug_overlays/MetricsOverlay.h"
#include "debug_overlays/OverlayManager.h"

#include <Box2D/b2_world_callbacks.h>
#include "cli/CommandLine.h"

class Bitmap;
class String;
class Font;
class GUIBase;
class InputManager;
class AudioSystem;
class AbstractGame;
class AudioSystem;
class PrecisionTimer;
class b2World;
class ContactListener;

#include "graphics/2DRenderContext.h"

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


void OutputDebugString(const String &textRef);

class GameEngine : public TSingleton<GameEngine>, public b2ContactListener {
private:
	GameEngine();
	friend class TSingleton<GameEngine>;

public:
	virtual ~GameEngine();

	GameEngine(const GameEngine &) = delete;
	GameEngine &operator=(const GameEngine &) = delete;

	// entry point to run a specific game
	static int run_game(HINSTANCE hInstance, cli::CommandLine const &cmdLine, int iCmdShow, class AbstractGame *game);

public:
	// Quits the game
	void quit_game();

	//! Create a messagebox
	//! @param text the text to display
	void message_box(const String &text) const;

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

	// Add GUI derived object to the GUI std::vector to be ticked automatically
	// NOT for students
	void register_gui(GUIBase *guiPtr);

	// Remove GUI derived object from the GUI vector
	// NOT for students
	void unregister_gui(GUIBase *guiPtr);

	//Console Methods
	//! ConsoleSetForeColor: Set the color of the text by blending the 3 basic colors to create 8 different color values
	//! @param red:	enable(true) or disable(false) the red component
	//! @param green: enable(true) or disable(false) the green component
	//! @param blue: enable(true) or disable(false) the blue component
	//! @param intensity: enable(true) or disable(false) highlight
	void console_set_fore_color(bool red, bool green, bool blue, bool intensity);

	//! ConsoleSetBackColor: Set the color of the background by blending the 3 basic colors to create 8 different color values
	//! @param red: enable(true) or disable(false) the red component
	//! @param green: enable(true) or disable(false) the green component
	//! @param blue: enable(true) or disable(false) the blue component
	//! @param intensity: enable(true) or disable(false) highlight
	void console_set_back_color(bool red, bool green, bool blue, bool intensity);

	//! ConsoleClear: Remove all characters from the console and set the cursor to the top left corner
	void console_clear() const;

	//! ConsoleSetCursorPosition: Set the cursor on a specific column and row number
	void console_set_cursor_position(int column, int row);

	//! ConsolePrintString: Display a String on the current cursor position. A new line is appended automatically
	void print_string(const String &textRef);
	void print_string(std::string const &msg);
	void print_string(const String &textRef, int column, int row);

	// Draw Methods

	//! Fills the client area of the window using the provided color, alpha is ignored
	bool DrawSolidBackground(COLOR backgroundColor);
	//! Draws a line from p1 to p2 using the strokewidth
	bool DrawLine(DOUBLE2 p1, DOUBLE2 p2, double strokeWidth = 1.0);
	//! Draws a line from the coordinate defined by x1 and y1 to the coordinate define by x2 and y2
	bool DrawLine(int x1, int y1, int x2, int y2);

	//! Draws a polygon defined by the coordinates in ptsArr
	//! count is the number of points that must be drawn
	//! If close is true then it will connect the start and end coordinate
	bool DrawPolygon(const std::vector<DOUBLE2> &ptsArr, unsigned int count, bool close = true, double strokeWidth = 1.0);

	//! Draws a polygon defined by the coordinates in ptsArr
	//! count is the number of points that must be drawn
	//! If close is true then it will connect the start and end coordinate
	bool DrawPolygon(const std::vector<POINT> &ptsArr, unsigned int count, bool close = true);

	//! Fills the interior of a polygon defined by the coordinates in ptsArr
	//! count is the number of points that must be drawn
	//! If close is true then it will connect the start and end coordinate
	bool FillPolygon(const std::vector<DOUBLE2> &ptsArr, unsigned int count);

	//! Fills the interior of a polygon defined by the coordinates in ptsArr
	//! count is the number of points that must be drawn
	//! If close is true then it will connect the start and end coordinate
	bool FillPolygon(const std::vector<POINT> &ptsArr, unsigned int count);

	//! Draws a rectangle defined by a RECT2 struct
	bool DrawRect(RECT2 rect, double strokeWidth = 1);
	//! Draws a rectangle defined by two coordinates: topleft and rightbottom
	bool DrawRect(DOUBLE2 topLeft, DOUBLE2 rightbottom, double strokeWidth = 1.0);
	//! Draws a rectangle defined by a RECT struct
	bool DrawRect(RECT rect);
	//! Draws a rectangle defined by 4 numbers representing the left side, top side, the right side and the bottom side
	bool DrawRect(int left, int top, int right, int bottom);

	//! Fills the interior of a rectangle defined by a RECT2 struct
	bool FillRect(RECT2 rect);
	//! Fills the interior of a rectangle defined by two coordinates: topleft and rightbottom
	bool FillRect(DOUBLE2 topLeft, DOUBLE2 rightbottom);
	//! Fills the interior of a rectangle defined by a RECT struct
	bool FillRect(RECT rect);
	//! Fills the interior of a rectangle defined by 4 numbers representing the left side, top side, the right side and the bottom side
	bool FillRect(int left, int top, int right, int bottom);

	//! Draws a rounded rectangle defined by a RECT2 struct,
	//!   the x-radius for the quarter ellipse that is drawn to replace every corner of the rectangle.
	//!   the y-radius for the quarter ellipse that is drawn to replace every corner of the rectangle.
	bool DrawRoundedRect(RECT2 rect, int radiusX, int radiusY, double strokeWidth = 1.0);

	//! Draws a rounded rectangle defined by two coordinates: topleft and rightbottom,
	//!   the x-radius for the quarter ellipse that is drawn to replace every corner of the rectangle.
	//!   the y-radius for the quarter ellipse that is drawn to replace every corner of the rectangle.
	bool DrawRoundedRect(DOUBLE2 topLeft, DOUBLE2 rightbottom, int radiusX, int radiusY, double strokeWidth = 1.0);

	//! Draws a rounded rectangle defined by a RECT struct
	//!   the x-radius for the quarter ellipse that is drawn to replace every corner of the rectangle.
	//!   the y-radius for the quarter ellipse that is drawn to replace every corner of the rectangle.
	bool DrawRoundedRect(RECT rect, int radiusX, int radiusY);

	//! Draws a rounded rectangle defined by 4 numbers representing the left side, top side, the right side and the bottom side
	//!   the x-radius for the quarter ellipse that is drawn to replace every corner of the rectangle.
	//!   the y-radius for the quarter ellipse that is drawn to replace every corner of the rectangle.
	bool DrawRoundedRect(int left, int top, int right, int bottom, int radiusX, int radiusY);

	//! Fills the interior of a rounded rectangle defined by a RECT2 struct,
	//!   the x-radius for the quarter ellipse that is drawn to replace every corner of the rectangle.
	//!   the y-radius for the quarter ellipse that is drawn to replace every corner of the rectangle.
	bool FillRoundedRect(RECT2 rect, int radiusX, int radiusY);

	//! Fills the interior of a rounded rectangle defined by two coordinates: topleft and rightbottom,
	//!   the x-radius for the quarter ellipse that is drawn to replace every corner of the rectangle.
	//!   the y-radius for the quarter ellipse that is drawn to replace every corner of the rectangle.
	bool FillRoundedRect(DOUBLE2 topLeft, DOUBLE2 rightbottom, int radiusX, int radiusY);

	//! Fills the interior of a rounded rectangle defined by a RECT struct
	//!   the x-radius for the quarter ellipse that is drawn to replace every corner of the rectangle.
	//!   the y-radius for the quarter ellipse that is drawn to replace every corner of the rectangle.
	bool FillRoundedRect(RECT rect, int radiusX, int radiusY);

	//! Fills the interior of a rounded rectangle defined by 4 numbers representing the left side, top side, the right side and the bottom side
	//!   the x-radius for the quarter ellipse that is drawn to replace every corner of the rectangle.
	//!   the y-radius for the quarter ellipse that is drawn to replace every corner of the rectangle.
	bool FillRoundedRect(int left, int top, int right, int bottom, int radiusX, int radiusY);

	//! Draws the outline of the specified ellipse using the specified position and radius
	//! strokeWidth: The width of the stroke, in device-independent pixels. The value must be greater than or equal to 0.0f.
	//! If this parameter isn't specified, it defaults to 1.0f. The stroke is centered on the line.
	bool DrawEllipse(DOUBLE2 centerPt, double radiusX, double radiusY, double strokeWidth = 1.0);

	//! Draws the outline of the specified ellipse using the specified position and radius
	bool DrawEllipse(int centerX, int centerY, int radiusX, int radiusY);

	//! Paints the interior of the specified ellipse using the specified position and radius
	bool FillEllipse(DOUBLE2 centerPt, double radiusX, double radiusY);

	//! Paints the interior of the specified ellipse using the specified position and radius
	bool FillEllipse(int centerX, int centerY, int radiusX, int radiusY);

	// Commented to prevent that this would compile: DrawString("blah", 10, 10);
	//bool			DrawString(std::string text, RECT boundingRect);
	//bool			DrawString(std::string text, RECT2 boundingRect);
	bool			DrawString(std::string text, DOUBLE2 topLeft, double right = -1, double bottom = -1);
	bool			DrawString(std::string text, int xPos, int yPos, int right = -1, int bottom = -1);


	//! Draws text in the specified rectangle
	bool DrawString(const String &textRef, RECT boundingRect);

	//! Draws text in the specified rectangle
	bool DrawString(const String &textRef, RECT2 boundingRect);

	//! Draws text in the specified rectangle the topleft corner of the rectange is defined by the param topLeft
	//! The params right and bottom are optional, if left out they are set to the max value of an float type
	bool DrawString(const String &textRef, DOUBLE2 topLeft, double right = -1, double bottom = -1);

	//! Draws text in the specified rectangle; the topleft corner of the rectange is defined by the params xPos and yPos
	//! The params right and bottom are optional, if left out they are set to the max value of an float type
	bool DrawString(const String &textRef, int xPos, int yPos, int right = -1, int bottom = -1);

	//! Draws an image on the position
	//! srcRect: defines the cliprect on the source image. Allows to draw a part of an image
	bool DrawBitmap(Bitmap *imagePtr, DOUBLE2 position, RECT2 srcRect);

	//! Draws an image on the position
	bool DrawBitmap(Bitmap *imagePtr, DOUBLE2 position);

	//! Draws an image on the position defined by x and y
	//! srcRect: defines the cliprect on the source image. Allows to draw a part of an image
	bool DrawBitmap(Bitmap *imagePtr, int x, int y, RECT srcRect);

	//! Draws an image on the position defined by x and y
	bool DrawBitmap(Bitmap *imagePtr, int x, int y);

	//! Draws an image on position x:0, and y:0. Assuming that matrices are used to define the position.
	//! srcRect: defines the cliprect on the source image. Allows to draw a part of an image
	bool DrawBitmap(Bitmap *imagePtr, RECT srcRect);

	//! Draws an image on position x:0, and y:0. Assuming that matrices are used to define the position.
	bool DrawBitmap(Bitmap *imagePtr);

	void d2d_flush() { _d2d_rt->Flush(); }

	//! Sets the matrix that defines world space
	void set_world_matrix(const MATRIX3X2 &mat);

	//! Returns the matrix that defines world space
	MATRIX3X2 get_world_matrix();

	//! Sets the matrix that defines view space
	void set_view_matrix(const MATRIX3X2 &mat);

	//! Returns the matrix that defines view space
	MATRIX3X2 get_view_matrix();

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
	String get_title() const;
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
	XMFLOAT2 get_mouse_pos_in_viewport() const;

	//! returns pointer to the Audio object
	AudioSystem *GetXAudio() const;
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

	// Binds HINSTANCE from Win32 api
	void set_instance(HINSTANCE hInstance);

	// Binds HWND from Win32 api
	void set_window(HWND hWindow);

	void set_title(const String &titleRef);

	void set_width(int iWidth);
	void set_height(int iHeight);
	void enable_vsync(bool bEnable = true);

	void console_create();
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
	void GUITick(double deltaTime);
	void GUIPaint();
	void GUIConsumeEvents();

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
	String _title;
	WORD _icon, _small_icon;
	int _window_width, _window_height;
	bool _should_sleep;
	AbstractGame *_game;
	HANDLE _console;

	// Draw assistance variables
	bool _can_paint;
	bool _vsync_enabled;

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

	bool _is_viewport_focused;
	bool _recreate_game_texture;
	bool _recreate_swapchain;

	// Game viewport resources
	ID2D1Factory *_d2d_factory;
	IWICImagingFactory *_wic_factory;
	ID2D1RenderTarget *_d2d_rt;
	IDWriteFactory *_dwrite_factory;
	PrecisionTimer *_game_timer;

	ID2D1SolidColorBrush *_color_brush;
	DXGI_SAMPLE_DESC _aa_desc;
	D2D1_ANTIALIAS_MODE _d2d_aa_mode;

	MATRIX3X2 _mat_world;
	MATRIX3X2 _mat_view;


	// Fonts used for text rendering
	Font *_default_font; // Default Font --> deleted in destructor

	// legacy UI is stored as GUI elements and are rendered each frame (not immediate mode)
	std::vector<GUIBase *> _gui_elements;

	// Input manager
	InputManager *_input_manager;

	// Box2D
	b2World *_b2d_world = nullptr;
	b2ContactFilter *_b2d_contact_filter = nullptr;
	double _b2d_time = 0;
	Box2DDebugRenderer _b2d_debug_renderer;
	bool _debug_physics_rendering;
	std::vector<ContactData> _b2d_begin_contact_data;
	std::vector<ContactData> _b2d_end_contact_data;
	std::vector<ImpulseData> _b2d_impulse_data;
	double _physics_timestep = 1 / 60.0f;
	DOUBLE2 _gravity;
	unsigned long _frame_cnt = 0;

	AudioSystem *_xaudio_system = nullptr;

	MetricsOverlay *_metrics_overlay;
	std::shared_ptr<OverlayManager> _overlay_manager;
	GameSettings _game_settings;
	EngineSettings _engine_settings;

	graphics::D2DRenderContext *_d2d_ctx;

	bool _physics_step_enabled;
	bool _should_quit;
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
	ID3DUserDefinedAnnotation *_annotation;
	std::wstring _name;
};

#define COMBINE1(X, Y) X##Y // helper macro
#define COMBINE(X, Y) COMBINE1(X, Y)
#define GPU_SCOPED_EVENT(ctx, name) scoped_gpu_event COMBINE(perfEvent, __LINE__) = scoped_gpu_event(ctx, name)
#define GPU_MARKER(ctx, name) ctx->SetMarker(name);
#else
#define GPU_SCOPED_EVENT(ctx, name)
#define GPU_MARKER(ctx, name)
#endif
