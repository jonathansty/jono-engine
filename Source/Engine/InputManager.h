#pragma once

// The values of keycode match up with the codes coming in from the Win32 event api
enum class KeyCode : u32
{
	Back = 0x08,
	Tab = 0x09,
	LeftAlt = 18,
	Clear = 0x0C,
	Return = 0x0D,
	// 0x0E-0x0F undefined
	Shift = 0x10,
	Control = 0x11,
	Menu = 0x12,
	Pause = 0x13,
	Capslock = 0x14,
	// 0x15 - 0x1A: IME key codes
	Escape = 0x1B,
	Space = 0x20,
	PageUp = 0x21,
	PageDown = 0x22,
	End = 0x23,
	Home = 0x24,
	Left = 0x25,
	Up = 0x26,
	Right = 0x27,
	Down = 0x28,
	Select = 0x29,
	Execute = 0x2B,
	Snapshot = 0x2C,
	Insert = 0x2D,
	Delete = 0x2E,
	Help = 0x2F,
	Num0 = 0x30,
	Num1 = 0x31,
	Num2 = 0x32,
	Num3 = 0x33,
	Num4 = 0x34,
	Num5 = 0x35,
	Num6 = 0x36,
	Num7 = 0x37,
	Num8 = 0x38,
	Num9 = 0x39,
	// 0x3a - 0x40 undefined
	A = 0x41,
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
	Z = 0x5A
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

class InputManager
{
public:
	InputManager(void);
	~InputManager(void);

	InputManager(const InputManager&) = delete;
	InputManager& operator=(const InputManager&) = delete;

	void init();
	void update();

	int2 get_mouse_position(bool previousFrame = false) const;
	int2 get_mouse_delta() const { return _mouse_delta; }
	f32 get_scroll_delta() const { return _mouse_wheel; }

	bool is_key_down(KeyCode key) const;
	bool is_key_pressed(KeyCode key) const;
	bool is_key_released(KeyCode key) const;

	bool is_mouse_button_down(int button) const;
	bool is_mouse_button_pressed(int button) const;
	bool is_mouse_button_released(int button) const;

	// #TODO: Remove this from the input manager
	void set_cursor_visible(bool visible) { ShowCursor(visible); }

private:
	using KeyState = bool[2];
	using KeyHandler = std::function<void(WPARAM, LPARAM)>;
	using MouseHandler = std::function<void(WPARAM, LPARAM)>;

	// Input handling is done by registering a bunch of handlers into fixed size arrays
	void register_key_handler(UINT msg, KeyHandler handler);
	void register_key_handler(std::vector<UINT> msgs, KeyHandler handler);

	void register_mouse_handler(UINT msg, MouseHandler handler);
	void register_mouse_handler(std::vector<UINT> msgs, MouseHandler handler);

	bool handle_events(UINT msg, WPARAM wParam, LPARAM lParam);

	// IDs to index into our state arrays
	// Mainly used for convenience and to avoid hardcoding numbers
	static constexpr u32 s_curr_frame = 0;
	static constexpr u32 s_prev_frame = 1;
	static constexpr u32 s_frame_count = 2;

	std::array<int2, 2> _mouse_pos;
	f32 _mouse_wheel;

	std::array<KeyState, 5> _mouse_buttons;

	std::array<KeyHandler, WM_KEYLAST - WM_KEYFIRST> _key_handlers;
	std::array<MouseHandler, WM_MOUSELAST - WM_MOUSEFIRST> _mouse_handlers;

	std::unordered_map<u32, KeyState> _keys;
	std::unordered_map<KeyCode, u32> _vk_to_scan;
	int2 _mouse_delta;

	friend class GameEngine;
};