#include "SceneViewer.pch.h"
#include "OrbitCamera.h"
#include "InputManager.h"

OrbitCamera::OrbitCamera(shared_ptr<RenderWorld> const& world)
		: _timer(0.0f)
		, _zoom(2.0f)
		, _up_timer(0.0f)
		, _center()
		, _world(world)
{
}

OrbitCamera::~OrbitCamera()
{
}

void OrbitCamera::tick(double dt)
{
	f32 f32_dt = (f32)dt;
	unique_ptr<InputManager> const& input_manager = GameEngine::instance()->get_input();

	shared_ptr<RenderWorldCamera> camera = _world->get_view_camera();

	// Handle the input
	{
		if (input_manager->is_key_pressed(KeyCode::Right))
		{
			_timer -= 0.5f;
		}
		if (input_manager->is_key_pressed(KeyCode::Left))
		{
			_timer += 0.5f;
		}
	}

	bool has_viewport_focus = GameEngine::instance()->is_viewport_focused();
	if (has_viewport_focus)
	{
		f32 scroll_delta = input_manager->get_scroll_delta();
		f32 zoom_factor = _zoom * 0.05f;
		if (scroll_delta != 0)
		{
			_zoom -= scroll_delta * (zoom_factor)*100.0f * f32_dt;
			_zoom = std::clamp(_zoom, 0.01f, 1000.0f);
		}

		// Rotate camera
		float2 mouse_delta = float2(input_manager->get_mouse_delta());
		if (input_manager->is_mouse_button_down(0))
		{
			_timer -= mouse_delta.x * f32_dt * 0.5f;
			_up_timer += mouse_delta.y * f32_dt * 0.5f;
		}
		_up_timer = std::clamp<f32>(_up_timer, -static_cast<f32>(M_PI) + std::numeric_limits<f32>::epsilon(), -std::numeric_limits<f32>::epsilon());


		// pan
		float3 pos = camera->get_position();

		float3 localPan = float3(0.0f, 0.0f, 0.0f);
		if (input_manager->is_mouse_button_down(1))
		{
			float4x4 view = camera->get_view();

			float4 right = hlslpp::mul(view, float4(1.0, 0.0, 0.0, 0.0)) * -(f32)mouse_delta.x * f32_dt;
			float4 up = hlslpp::mul(view, float4(0.0, 1.0, 0.0, 0.0)) * (f32)mouse_delta.y * f32_dt;

			localPan = (right + up).xyz * zoom_factor;
		}
		_center += localPan;

		if (input_manager->is_key_pressed(KeyCode::Z))
		{
			LOG_INFO(Input, "Resetting view.");
			_timer = 0.0f;
			_up_timer = 0.0f;
			_center = float3(0.0, 0.0, 0.0);
		}
	}

	constexpr float radius = 10.0f;
	float3 newUnitPos = float3{ cos(_timer) * sin(_up_timer), cos(_up_timer), sin(_timer) * sin(_up_timer) };

	float4x4 look = float4x4::look_at(_center + _zoom * newUnitPos, _center, float3(0.0f, 1.0f, 0.0f));
	auto result = hlslpp::quaternion(float3x3(look.vec0, look.vec1, look.vec2));
	camera->set_view(look);
}
