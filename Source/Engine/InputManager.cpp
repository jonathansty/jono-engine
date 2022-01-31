#include "engine.pch.h"

#include "GameEngine.h"
#include "InputManager.h"

// Only enable when debugging the input manager
#define VERBOSE_LOGGING 0

bool is_mouse_event(UINT msg) {
	return msg >= WM_MOUSEFIRST && msg < WM_MOUSELAST;

}

bool is_key_event(UINT msg) {
	return msg >= WM_KEYFIRST && msg < WM_KEYLAST;
}

bool InputManager::handle_events(UINT msg, WPARAM wParam, LPARAM lParam) {

	if(is_key_event(msg)) {
		if (_key_handlers[msg - WM_KEYFIRST])
			_key_handlers[msg - WM_KEYFIRST](wParam, lParam);

		return false;
	} else if (is_mouse_event(msg)) {
		if (_mouse_handlers[msg - WM_MOUSEFIRST])
			_mouse_handlers[msg - WM_MOUSEFIRST](wParam, lParam);

		return false;
	}

	return false;
}

void InputManager::register_key_handler(UINT msg, KeyHandler handler) {
	register_key_handler(std::vector<UINT>{ msg }, handler);
}

void InputManager::register_key_handler(std::vector<UINT> msgs, KeyHandler handler) {
	std::for_each(msgs.begin(), msgs.end(), [this, handler](UINT msg) {
		_key_handlers[msg - WM_KEYFIRST] = handler;
	});
}

void InputManager::register_mouse_handler(UINT msg, MouseHandler handler) {
	register_mouse_handler(std::vector<UINT>{ msg }, handler);
}

void InputManager::register_mouse_handler(std::vector<UINT> msgs, MouseHandler handler) {
	std::for_each(msgs.begin(), msgs.end(), [this, handler](UINT msg) {
		_mouse_handlers[msg - WM_MOUSEFIRST] = handler;
	});
}

InputManager::InputManager(void)
: _mouse_pos()
, _mouse_delta()
, _keys()
, _mouse_wheel(0.0f)
, _mouse_buttons()
{
}

InputManager::~InputManager(void)
{
}

void InputManager::init()
{
	_keys.reserve(255);

	auto handle_base_keys = [this](WPARAM wParam, LPARAM lParam) {
		WORD vk_code = LOWORD(wParam); // virtual-key code
		BYTE scan_code = LOBYTE(HIWORD(lParam)); // scan code
		BOOL scan_code_e0 = (HIWORD(lParam) & KF_EXTENDED) == KF_EXTENDED; // extended-key flag, 1 if scancode has 0xE0 prefix
		BOOL up_flag = (HIWORD(lParam) & KF_UP) == KF_UP; // transition-state flag, 1 on keyup
		BOOL repeat_flag = (HIWORD(lParam) & KF_REPEAT) == KF_REPEAT; // previous key-state flag, 1 on autorepeat
		WORD repeat_count = LOWORD(lParam); // repeat count, > 0 if several keydown messages was combined into one message
		BOOL alt_down_flag = (HIWORD(lParam) & KF_ALTDOWN) == KF_ALTDOWN; // ALT key was pressed
		BOOL dlg_mode_flag = (HIWORD(lParam) & KF_DLGMODE) == KF_DLGMODE; // dialog box is active
		BOOL menu_mode_flag = (HIWORD(lParam) & KF_MENUMODE) == KF_MENUMODE; // menu is active
		#if VERBOSE_LOGGING
		LOG_INFO(Input, "VK: {} | Scan: {} | repeat ({}): {} | up: {}", vk_code, scan_code, repeat_flag ? "Y" : "N", repeat_count, up_flag);
		#endif
		_keys[scan_code][s_curr_frame] = !up_flag;

		_vk_to_scan[(KeyCode)vk_code] = scan_code;
	};
	register_key_handler({ WM_SYSKEYUP, WM_SYSKEYUP, WM_KEYUP, WM_KEYDOWN }, handle_base_keys);

	auto handle_mbuttons = [this](WPARAM wParam, LPARAM lParam) {
		bool ctrl = wParam & MK_CONTROL;
		bool shift = wParam & MK_SHIFT;
		bool lbutton = wParam & MK_LBUTTON;
		_mouse_buttons[0][s_curr_frame] = lbutton;

		bool mbutton = wParam & MK_MBUTTON;
		_mouse_buttons[1][s_curr_frame] = mbutton;

		bool rbutton = wParam & MK_RBUTTON;
		_mouse_buttons[2][s_curr_frame] = rbutton;

		bool xbutton1 = wParam & MK_XBUTTON1;
		_mouse_buttons[3][s_curr_frame] = xbutton1;

		bool xbutton2 = wParam & MK_XBUTTON2;
		_mouse_buttons[4][s_curr_frame] = xbutton2;

		#if VERBOSE_LOGGING
		LOG_INFO(Input, "B0: {} | B1: {} | B2: {} | B3: {} | B4: {}", 
			_mouse_buttons[0][s_curr_frame],
			_mouse_buttons[1][s_curr_frame],
			_mouse_buttons[2][s_curr_frame],
			_mouse_buttons[3][s_curr_frame],
			_mouse_buttons[4][s_curr_frame]
		);
		#endif

	};
	register_mouse_handler(
		{ 
		   WM_MBUTTONDOWN,
		   WM_MBUTTONUP,
		   WM_LBUTTONDOWN,
		   WM_LBUTTONUP,
		   WM_RBUTTONDOWN,
		   WM_RBUTTONUP,
		   WM_XBUTTONDOWN,
		   WM_XBUTTONUP 
		},
        handle_mbuttons);

	register_mouse_handler(WM_MOUSEWHEEL, [this](WPARAM wParam, LPARAM lParam) {
		f32 delta = GET_WHEEL_DELTA_WPARAM(wParam) / (f32)WHEEL_DELTA;
		_mouse_wheel = delta;
		#if VERBOSE_LOGGING		
		LOG_INFO(Input, "Wheel: {}", delta);
		#endif

	});
}

void InputManager::update() {
	for (std::pair<const u32, bool[2]>& it : _keys) {
		it.second[s_prev_frame] = it.second[s_curr_frame];
	}
	
	for (auto& button : _mouse_buttons) {
		button[s_prev_frame] = button[s_curr_frame];
	}


	// Update the previous mouse position
	_mouse_pos[s_prev_frame] = _mouse_pos[s_curr_frame];

	// Update the current mouse position
	POINT mouse_pos = {};
	GetCursorPos(&mouse_pos);
	_mouse_pos[s_curr_frame] = { mouse_pos.x, mouse_pos.y };
	_mouse_delta = _mouse_pos[s_curr_frame] - _mouse_pos[s_prev_frame];

	// Calculate the delta
	_mouse_wheel = 0;
}

int2 InputManager::get_mouse_position(bool previousFrame) const {
	return _mouse_pos[previousFrame ? 1 : 0];
}

bool InputManager::is_key_down(KeyCode key) const
{
	if (auto scan_code = _vk_to_scan.find(key); scan_code != _vk_to_scan.end()) {
		if (auto it = _keys.find(scan_code->second); it != _keys.end()) {
			return it->second[s_curr_frame];
		}
	}

	return false;
}

bool InputManager::is_mouse_button_down(int button) const
{
	return _mouse_buttons[button][s_curr_frame];
}

bool InputManager::is_key_pressed(KeyCode key) const
{
	if (auto scan_code = _vk_to_scan.find(key); scan_code != _vk_to_scan.end()) {
		if (auto it = _keys.find(scan_code->second); it != _keys.end()) {
			return it->second[s_curr_frame] && !it->second[s_prev_frame];
		}
	}
	return false;
}

bool InputManager::is_mouse_button_pressed(int button) const
{
	KeyState const& button_state = _mouse_buttons[button];
	return button_state[s_curr_frame] && !button_state[s_prev_frame];
}

bool InputManager::is_key_released(KeyCode key) const
{
	if (auto scan_code = _vk_to_scan.find(key); scan_code != _vk_to_scan.end()) {
		if (auto it = _keys.find(scan_code->second); it != _keys.end()) {
			return !it->second[s_curr_frame] && it->second[s_prev_frame];
		}
	}

	return false;
}

bool InputManager::is_mouse_button_released(int button) const
{
	KeyState const& button_state = _mouse_buttons[button];
	return !button_state[s_curr_frame] && button_state[s_prev_frame];
}
