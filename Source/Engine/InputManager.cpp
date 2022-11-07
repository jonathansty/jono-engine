#include "engine.pch.h"

#include "GameEngine.h"
#include "InputManager.h"

// Only enable when debugging the input manager
#define VERBOSE_LOGGING 0

bool is_mouse_event(UINT msg)
{
	return msg >= WM_MOUSEFIRST && msg < WM_MOUSELAST;
}

bool is_key_event(UINT msg)
{
	return msg >= WM_KEYFIRST && msg < WM_KEYLAST;
}

bool is_mouse_event(SDL_Event& msg)
{
	return msg.type == SDL_MOUSEBUTTONDOWN || msg.type == SDL_MOUSEBUTTONUP || msg.type == SDL_MOUSEWHEEL || msg.type == SDL_MOUSEMOTION;
}

bool is_key_event(SDL_Event& msg)
{
	return msg.type == SDL_KEYDOWN || msg.type == SDL_KEYUP;
}


bool InputManager::handle_events(SDL_Event& e)
{
	if (is_key_event(e))
	{
		if (m_KeyHandlers[e.type - SDL_KEYDOWN])
		{
			m_KeyHandlers[e.type - SDL_KEYDOWN](e);
		}

		return false;
	}
	else if (is_mouse_event(e))
	{
		if (m_MouseHandlers[e.type - SDL_MOUSEMOTION])
		{
			m_MouseHandlers[e.type - SDL_MOUSEMOTION](e);
		}

		return false;
	}

	return false;
}

void InputManager::register_key_handler(UINT msg, KeyHandler handler)
{
	register_key_handler(std::vector<UINT>{ msg }, handler);
}

void InputManager::register_key_handler(std::vector<UINT> msgs, KeyHandler handler)
{
	std::for_each(msgs.begin(), msgs.end(), [this, handler](UINT msg)
			{ m_KeyHandlers[msg - SDL_KEYDOWN] = handler; });
}

void InputManager::register_mouse_handler(UINT msg, MouseHandler handler)
{
	register_mouse_handler(std::vector<UINT>{ msg }, handler);
}

void InputManager::register_mouse_handler(std::vector<UINT> msgs, MouseHandler handler)
{
	std::for_each(msgs.begin(), msgs.end(), [this, handler](UINT msg)
			{ m_MouseHandlers[msg - SDL_MOUSEMOTION] = handler; });
}

InputManager::InputManager(void)
		: m_MousePos()
		, m_MouseDelta()
		, m_Keys()
		, m_MouseWheel(0.0f)
		, m_MouseButtons()
{
}

InputManager::~InputManager(void)
{
}

void InputManager::init()
{
	auto handle_base_keys = [this](SDL_Event& e)
	{
#if VERBOSE_LOGGING
		LOG_INFO(Input, "VK: {} | Scan: {} | repeat ({}): {} | up: {}", vk_code, scan_code, repeat_flag ? "Y" : "N", repeat_count, up_flag);
#endif
		m_Keys[e.key.keysym.scancode].pressed = e.key.state == SDL_PRESSED;
	};
	register_key_handler({ SDL_KEYDOWN, SDL_KEYUP}, handle_base_keys);

	auto handle_mbuttons = [this](SDL_Event& e)
	{
		m_MouseButtons[e.button.button].pressed = e.button.state == SDL_PRESSED;
#if VERBOSE_LOGGING
		LOG_INFO(Input, "B0: {} | B1: {} | B2: {} | B3: {} | B4: {}",
				m_MouseButtons[0][s_curr_frame],
				m_MouseButtons[1][s_curr_frame],
				m_MouseButtons[2][s_curr_frame],
				m_MouseButtons[3][s_curr_frame],
				m_MouseButtons[4][s_curr_frame]);
#endif
	};
	register_mouse_handler(
			{
					SDL_MOUSEBUTTONDOWN,
					SDL_MOUSEBUTTONUP,
			},
			handle_mbuttons);

	register_mouse_handler(SDL_MOUSEWHEEL, [this](SDL_Event& e)
			{
				m_MouseWheel = e.wheel.preciseY;
#if VERBOSE_LOGGING
				LOG_INFO(Input, "Wheel: {}", delta);
#endif
			}
	);
}

void InputManager::update()
{
	for (KeyState& it : m_Keys)
	{
		it.prevPressed = it.pressed;
	}

	for (auto& button : m_MouseButtons)
	{
		button.prevPressed = button.pressed;
	}

	// Update the previous mouse position
	m_MousePos[s_prev_frame] = m_MousePos[s_curr_frame];

	int x, y;
	Uint32 buttonState = SDL_GetMouseState(&x, &y);
	m_MousePos[s_curr_frame] = { x, y };
	m_MouseDelta = m_MousePos[s_curr_frame] - m_MousePos[s_prev_frame];

	// Calculate the delta
	m_MouseWheel = 0;
}

int2 InputManager::get_mouse_position(bool previousFrame) const
{
	return m_MousePos[previousFrame ? 1 : 0];
}

bool InputManager::is_key_down(KeyCode key) const
{
	SDL_Scancode code = SDL_GetScancodeFromKey((SDL_KeyCode)key);
	return m_Keys[code].pressed;
}

bool InputManager::is_mouse_button_down(int button) const
{
	return m_MouseButtons[button].pressed;
}

bool InputManager::is_key_pressed(KeyCode key) const
{
	SDL_Scancode code = SDL_GetScancodeFromKey((SDL_KeyCode)key);
	return m_Keys[code].pressed && !m_Keys[code].prevPressed;
}

bool InputManager::is_mouse_button_pressed(int button) const
{
	KeyState const& button_state = m_MouseButtons[button];
	return button_state.pressed && !button_state.prevPressed;
}

bool InputManager::is_key_released(KeyCode key) const
{
	SDL_Scancode code = SDL_GetScancodeFromKey((SDL_KeyCode)key);
	KeyState const& state = m_Keys[code];
	return !state.pressed && state.prevPressed;
}

bool InputManager::is_mouse_button_released(int button) const
{
	KeyState const& button_state = m_MouseButtons[button];
	return !button_state.pressed && button_state.prevPressed;
}

void InputManager::set_cursor_visible(bool visible)
{
	m_CaptureMouse = !visible;
	SDL_ShowCursor(visible);
}
