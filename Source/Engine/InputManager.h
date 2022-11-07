#pragma once

// The values of keycode match up with the codes coming in from the Win32 event api
enum class KeyCode : SDL_Keycode
{
	Back = SDLK_AC_BACK,
	Tab = SDLK_TAB,
	LAlt = SDLK_LALT,
	Clear = SDLK_CLEAR,
	Return = SDLK_RETURN,
	LShift = SDLK_LSHIFT,
	LControl = SDLK_LCTRL,
	Menu = SDLK_MENU,
	Pause = SDLK_PAUSE,
	Capslock = SDLK_CAPSLOCK,
	Escape = SDLK_ESCAPE,
	Space = SDLK_SPACE,
	PageUp = SDLK_PAGEUP,
	PageDown = SDLK_PAGEDOWN,
	End = SDLK_END,
	Home = SDLK_HOME,
	Left = SDLK_LEFT,
	Up = SDLK_UP,
	Right = SDLK_RIGHT,
	Down = SDLK_DOWN,
	Select = SDLK_SELECT,
	Execute = SDLK_EXECUTE,
	PrintScreen = SDLK_PRINTSCREEN,
	Insert = SDLK_INSERT,
	Delete = SDLK_DELETE,
	Help = SDLK_HELP,
	Num0 = SDLK_KP_0,
	Num1 = SDLK_KP_1,
	Num2 = SDLK_KP_2,
	Num3 = SDLK_KP_3,
	Num4 = SDLK_KP_4,
	Num5 = SDLK_KP_5,
	Num6 = SDLK_KP_6,
	Num7 = SDLK_KP_7,
	Num8 = SDLK_KP_8,
	Num9 = SDLK_KP_9,
	// 0x3a - 0x40 undefined
	A = SDLK_a,
	B,
	C,
	D,
	E,
	F,
	G,
	H,
	I,
	J,
	K,
	L,
	M,
	N,
	O,
	P,
	Q,
	R,
	S,
	T,
	U,
	V,
	W,
	X,
	Y,
	Z = SDLK_z
};

inline u32 get_raw(KeyCode code)
{
	return static_cast<u32>(code);
}

enum class MouseKeyCode
{
	Left = 0x1,
	Right = 0x2,
	Middle = 0x4,
	Mouse4 = 0x5,
	Mouse5 = 0x6
};

class ENGINE_API InputManager
{
public:
	InputManager(void);
	~InputManager(void);

	InputManager(const InputManager&) = delete;
	InputManager& operator=(const InputManager&) = delete;

	void init();
	void update();

	int2 get_mouse_position(bool previousFrame = false) const;
	int2 get_mouse_delta() const { return m_MouseDelta; }
	f32  get_scroll_delta() const { return m_MouseWheel; }

	bool is_key_down(KeyCode key) const;
	bool is_key_pressed(KeyCode key) const;
	bool is_key_released(KeyCode key) const;

	bool is_mouse_button_down(int button) const;
	bool is_mouse_button_pressed(int button) const;
	bool is_mouse_button_released(int button) const;

	// #TODO: Remove this from the input manager
	void set_cursor_visible(bool visible);

private:
	struct KeyState
	{
		bool pressed;
		bool prevPressed;
	};

	using KeyHandler = std::function<void(SDL_Event& e)>;
	using MouseHandler = std::function<void(SDL_Event& e)>;

	// Input handling is done by registering a bunch of handlers into fixed size arrays
	void register_key_handler(UINT msg, KeyHandler handler);
	void register_key_handler(std::vector<UINT> msgs, KeyHandler handler);

	void register_mouse_handler(UINT msg, MouseHandler handler);
	void register_mouse_handler(std::vector<UINT> msgs, MouseHandler handler);

	bool handle_events(SDL_Event& e);


	// IDs to index into our state arrays
	// Mainly used for convenience and to avoid hardcoding numbers
	static constexpr u32 s_curr_frame = 0;
	static constexpr u32 s_prev_frame = 1;
	static constexpr u32 s_frame_count = 2;

	std::array<int2, 2> m_MousePos;
	f32 m_MouseWheel;

	static constexpr u32 s_MaxMouseButtons = 5;
	static constexpr u32 s_MaxKeys = SDL_NUM_SCANCODES;
	static constexpr u32 s_MaxMouseHandlers = SDL_MOUSEWHEEL - SDL_MOUSEMOTION + 1;
	static constexpr u32 s_MaxKeyHandlers = SDL_TEXTEDITING_EXT - SDL_KEYDOWN + 1;

	std::array<KeyState,     s_MaxMouseButtons>  m_MouseButtons;
	std::array<KeyHandler,   s_MaxKeyHandlers>   m_KeyHandlers;
	std::array<MouseHandler, s_MaxMouseHandlers> m_MouseHandlers;
	std::array<KeyState,     s_MaxKeys>          m_Keys;

	int2 m_MouseDelta;

	bool m_CaptureMouse;

	friend class GameEngine;
};