//-----------------------------------------------------------------
// Game Engine Object
// C++ Header - version v2_16 jan 2015 
// Copyright DAE Programming Team
// http://www.digitalartsandentertainment.be/
//-----------------------------------------------------------------

#pragma once

class InputManager
{
public:
	InputManager(void);
	~InputManager(void);

	// C++11 make the class non-copyable
	InputManager(const InputManager&) = delete;
	InputManager& operator=(const InputManager&) = delete;

	// Not intended to be used by students
	void Initialize();
	// Not intended to be used by students
	void Update();

	int2 get_mouse_position(bool previousFrame = false) const;

	// Returns the mouse movement change between this frame and previous frame
	int2 get_mouse_delta() const { return _mouse_delta; }

	bool is_key_down(int key) const;
	bool is_key_pressed(int key) const;
	bool is_key_released(int key) const;

	bool is_mouse_button_down(int button) const;
	bool is_mouse_button_pressed(int button) const;
	bool is_mouse_button_released(int button) const;

	// #TODO: Remove this from the input manager
	void set_cursor_visible(bool visible) { ShowCursor(visible); }



private:
	static constexpr u32 s_curr_frame = 0;
	static constexpr u32 s_prev_frame = 1;
	static constexpr u32 s_frame_count = 2;

	bool handle_events(UINT msg, WPARAM wParam, LPARAM lParam);

	using KeyState = bool[2];

	using KeyHandler = std::function<void(WPARAM, LPARAM)>;
	void register_key_handler(UINT msg, KeyHandler handler);

	using MouseHandler = std::function<void(WPARAM, LPARAM)>;
	void register_mouse_handler(UINT msg, MouseHandler handler);

	std::array<int2,2> _mouse_pos;
	std::array<f32, 2> _mouse_wheel;
	std::array<KeyState, 5> _mouse_buttons;

	std::array<KeyHandler, WM_KEYLAST - WM_KEYFIRST> _key_handlers;
	std::array<MouseHandler, WM_MOUSELAST - WM_MOUSEFIRST> _mouse_handlers;

	std::unordered_map<u32, KeyState> _keys;
	std::unordered_map<int, u32> _vk_to_scan;
	int2 _mouse_delta;

	void update_keyboard();

	friend class GameEngine;
};