#include "SceneViewer.pch.h"
#include "OrbitCamera.h"
#include "InputManager.h"

OrbitCamera::OrbitCamera(shared_ptr<RenderWorld> const& world)
		: m_Timer(0.0f)
		, m_Zoom(2.0f)
		, m_UpTimer(0.0f)
		, m_Center()
		, m_World(world)
{
}

OrbitCamera::~OrbitCamera()
{
}

void OrbitCamera::Tick(double dt)
{
	f32 f32_dt = (f32)dt;
    InputManager* input_manager = GetGlobalContext()->m_InputManager;


	shared_ptr<RenderWorldCamera> camera = m_World->get_view_camera();

	// Handle the input
	{
		if (input_manager->IsKeyPressed(KeyCode::Right))
		{
			m_Timer -= 0.5f;
		}
		if (input_manager->IsKeyPressed(KeyCode::Left))
		{
			m_Timer += 0.5f;
		}
	}

	bool has_viewport_focus = GameEngine::instance()->IsViewportFocused();
	if (has_viewport_focus)
	{
		f32 scroll_delta = input_manager->GetScrollDelta();
		f32 zoom_factor = m_Zoom * 0.05f;
		if (scroll_delta != 0)
		{
			m_Zoom -= scroll_delta * (zoom_factor)*100.0f * f32_dt;
			m_Zoom = std::clamp(m_Zoom, 0.01f, 1000.0f);
		}

		// Rotate camera
		float2 mouse_delta = float2(input_manager->GetMouseDelta());
		if (input_manager->IsMouseButtonDown(SDL_BUTTON_LEFT))
		{
			m_Timer -= mouse_delta.x * f32_dt * 0.5f;
			m_UpTimer += mouse_delta.y * f32_dt * 0.5f;
		}
		m_UpTimer = std::clamp<f32>(m_UpTimer, -static_cast<f32>(M_PI) + std::numeric_limits<f32>::epsilon(), -std::numeric_limits<f32>::epsilon());


		// pan
		float3 pos = camera->get_position();

		float3 localPan = float3(0.0f, 0.0f, 0.0f);
		if (input_manager->IsMouseButtonDown(SDL_BUTTON_RIGHT))
		{
			float4x4 view = camera->get_view();

			float4 right = hlslpp::mul(view, float4(1.0, 0.0, 0.0, 0.0)) * -(f32)mouse_delta.x * f32_dt;
			float4 up = hlslpp::mul(view, float4(0.0, 1.0, 0.0, 0.0)) * (f32)mouse_delta.y * f32_dt;

			localPan = (right + up).xyz * zoom_factor;
		}
		m_Center += localPan;

		if (input_manager->IsKeyPressed(KeyCode::Z))
		{
			LOG_INFO(Input, "Resetting view.");
			m_Timer = 0.0f;
			m_UpTimer = 0.0f;
			m_Center = float3(0.0, 0.0, 0.0);
		}
	}

	constexpr float radius = 10.0f;
	float3 newUnitPos = float3{ cos(m_Timer) * sin(m_UpTimer), cos(m_UpTimer), sin(m_Timer) * sin(m_UpTimer) };

	float4x4 look = float4x4::look_at(m_Center + m_Zoom * newUnitPos, m_Center, float3(0.0f, 1.0f, 0.0f));
	auto result = hlslpp::quaternion(float3x3(look.vec0, look.vec1, look.vec2));
	camera->set_view(look);
}
