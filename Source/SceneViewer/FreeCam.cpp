#include "SceneViewer.pch.h"
#include "FreeCam.h"
#include "InputManager.h"

FreeCam::FreeCam(shared_ptr<RenderWorld> const& world)
		: _world(world)
		, _position(0.0f,0.0f,0.0f)
		, _x_rotation(0)
		, _y_rotation(0)
{
}

FreeCam::~FreeCam()
{
}

void FreeCam::tick(double deltaTime)
{
	unique_ptr<InputManager> const& manager = GameEngine::instance()->get_input();

	bool has_viewport_focus = GameEngine::instance()->is_viewport_focused();
	if (!has_viewport_focus)
		return;

	if(manager->is_mouse_button_pressed(0))
	{
		manager->set_cursor_visible(false);
	}
	if(manager->is_mouse_button_released(0))
	{
		manager->set_cursor_visible(true);
	}

	if (!manager->is_mouse_button_down(0))
	{
		return;
	}

	int2 mouse_delta = manager->get_mouse_delta();
	shared_ptr<RenderWorldCamera> camera = _world->get_view_camera();
	
	constexpr f32 c_rotation_speed = 1.0f;
	_x_rotation += f32(mouse_delta.y) * f32(deltaTime) * c_rotation_speed;
	_y_rotation += f32(mouse_delta.x) * f32(deltaTime) * c_rotation_speed;

	_rotation = hlslpp::mul( quaternion::rotation_y(_y_rotation), quaternion::rotation_x(_x_rotation));

	float3 pos = _position;
	float3 fwd = hlslpp::mul(Math::c_fwd, _rotation);
	float3 right = hlslpp::mul(Math::c_right, _rotation);

	constexpr f32 c_speed = 10.0f;
	if(manager->is_key_down(KeyCode::W))
	{
		pos += fwd * c_speed * deltaTime;
	}

	if (manager->is_key_down(KeyCode::S))
	{
		pos -= fwd * c_speed * deltaTime;
	}

	if (manager->is_key_down(KeyCode::D))
	{
		pos += right * c_speed * deltaTime;
	}

	if (manager->is_key_down(KeyCode::A))
	{
		pos -= right * c_speed * deltaTime;
	}

	if(manager->is_key_down(KeyCode::Space))
	{
		pos += Math::c_up * c_speed * deltaTime;
	}

	if(manager->is_key_down(KeyCode::Control))
	{
		pos -= Math::c_up * c_speed * deltaTime;
	}
	_position = pos;

	float4x4 world = hlslpp::mul(float4x4(_rotation), float4x4::translation(pos));
	camera->set_world(world);
}
