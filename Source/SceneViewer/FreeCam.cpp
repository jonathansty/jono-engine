#include "SceneViewer.pch.h"
#include "FreeCam.h"
#include "InputManager.h"

#include <DirectXTK/Effects.h>

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
    InputManager* manager = GetGlobalContext()->m_InputManager;

	bool has_viewport_focus = GameEngine::instance()->IsViewportFocused();
	if (!has_viewport_focus)
		return;

	if(manager->IsMouseButtonPressed(SDL_BUTTON_LEFT))
	{
		manager->SetCursorVisible(false);
	}
	if(manager->IsMouseButtonReleased(SDL_BUTTON_LEFT))
	{
		manager->SetCursorVisible(true);
	}

	if (!manager->IsMouseButtonDown(SDL_BUTTON_LEFT))
	{
		return;
	}

	int2 mouse_delta = manager->GetMouseDelta();
	shared_ptr<RenderWorldCamera> camera = _world->get_view_camera();
	
	constexpr f32 c_rotation_speed = 1.0f;
	_x_rotation += f32(mouse_delta.y) * f32(deltaTime) * c_rotation_speed;
	_y_rotation += f32(mouse_delta.x) * f32(deltaTime) * c_rotation_speed;

	_rotation = hlslpp::mul( quaternion::rotation_y(_y_rotation), quaternion::rotation_x(_x_rotation));

	float3 pos = _position;
	float3 fwd = hlslpp::mul(Math::c_fwd, _rotation);
	float3 right = hlslpp::mul(Math::c_right, _rotation);

	constexpr f32 c_speed = 10.0f;
	if(manager->IsKeyDown(KeyCode::W))
	{
		pos += fwd * c_speed * deltaTime;
	}

	if (manager->IsKeyDown(KeyCode::S))
	{
		pos -= fwd * c_speed * deltaTime;
	}

	if (manager->IsKeyDown(KeyCode::D))
	{
		pos += right * c_speed * deltaTime;
	}

	if (manager->IsKeyDown(KeyCode::A))
	{
		pos -= right * c_speed * deltaTime;
	}

	if(manager->IsKeyDown(KeyCode::Space))
	{
		pos += Math::c_up * c_speed * deltaTime;
	}

	if(manager->IsKeyDown(KeyCode::LControl))
	{
		pos -= Math::c_up * c_speed * deltaTime;
	}
	_position = pos;

	float4x4 world = hlslpp::mul(float4x4(_rotation), float4x4::translation(pos));
	camera->set_world(world);
}
