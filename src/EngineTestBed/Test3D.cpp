#include "testbed.stdafx.h"
#include <hlsl++.h>
#include "Test3D.h"

#include "Engine/Core/ModelResource.h"
#include "Engine/Core/TextureResource.h"
#include "Engine/Core/MaterialResource.h"

#include "Framework/framework.h"
#include "Components.h"
#include "Overlays.h"

using framework::Entity;
using framework::Component;

namespace Shaders {
#include "shaders/simple_px.h"
#include "shaders/simple_vx.h"
#include "shaders/debug_px.h"
}

using hlslpp::float4x4;
using hlslpp::float4;

__declspec(align(16))
struct MVPConstantBuffer
{
	float4x4 World;
	float4x4 WorldView;
	float4x4 Projection;
	float4x4 WorldViewProjection;

	float4x4 View;
	float4x4 InvView;

	float4 ViewDirection;
	float4 LightDirection;
};

struct DebugVisualizeMode
{
	enum Enum {
		Default = 0,
		Albedo = 1,
		Roughness = 2,
		Metalness = 3,
		TangentNormals = 4,
		AO = 5,
		WorldNormals = 6
	};
};
__declspec(align(16))
struct DebugCB
{
	unsigned int m_VisualizeMode;
	uint8_t pad[12];
};

int g_DebugMode = 0;

void Hello3D::configure_engine(EngineSettings &engineSettings) {
	engineSettings.d2d_use = false;

	engineSettings.d3d_use = true;
	engineSettings.d3d_msaa_mode = MSAAMode::MSAA_4x;
}

void Hello3D::initialize(GameSettings& gameSettings)
{
	gameSettings.m_FullscreenMode = GameSettings::FullScreenMode::Windowed;
	gameSettings.m_WindowWidth = 1800;
	gameSettings.m_WindowHeight = 900;
}


void Hello3D::start()
{
	auto device = GameEngine::instance()->GetD3DDevice();
	auto ctx = GameEngine::instance()->GetD3DDeviceContext();

	::SetCapture(GameEngine::instance()->get_window());

	using namespace framework;
	_world = std::make_shared<framework::World>();

	EntityDebugOverlay *overlay = new EntityDebugOverlay(_world.get());
	GameEngine::instance()->get_overlay_manager()->register_overlay(overlay);

	// Create the world camera
	{
		World::EntityId cam_id = _world->create_entity();
		framework::Entity* ent = _world->get_entity(cam_id);
		ent->set_name("MainCamera");
		auto comp = ent->create_component<CameraComponent>();
		ent->set_local_position(float3(0.0f, 0.0f, -2.0f));
	}

	{
		for (int i = 0; i < 1; ++i)
		{
			for (int j = 0; j < 1; ++j)
			{
				World::EntityId model = _world->create_entity();
				Entity* ent = _world->get_entity(model);
				ent->set_name("boxes_3");
				ent->set_local_position(float3( 2.0f * (float)i, 0.0f, 2.0f * (float)j));
				ent->set_local_scale({ 10.0f, 10.0f,10.0f });
				auto comp = ent->create_component<SimpleMeshComponent>();
				comp->set_model("Resources/Models/m-96_mattock/scene.gltf");

				auto c = ent->create_component<SimpleMovement3D>();
				c->set_speed(10.0f);
			}
		}


		World::EntityId model = _world->create_entity();
		Entity* ent = _world->get_entity(model);
		ent->set_name("Sun");
		ent->set_rotation(hlslpp::euler(float3(-0.33f, -0.33f, 0.0f)));
		auto comp = ent->create_component<LightComponent>();
	}


	// Initialize our rendering buffer
	{
		D3D11_BUFFER_DESC buff{};
		buff.ByteWidth = sizeof(MVPConstantBuffer);
		buff.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		buff.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		buff.Usage = D3D11_USAGE_DYNAMIC;
		buff.StructureByteStride = 0;
		buff.MiscFlags = 0;

		MVPConstantBuffer mvp_data{};
		D3D11_SUBRESOURCE_DATA data{};
		data.pSysMem = &mvp_data;
		SUCCEEDED(device->CreateBuffer(&buff, &data, _cb_MVP.GetAddressOf()));

		DebugCB debugData{};
		data.pSysMem = &debugData;
		buff.ByteWidth = sizeof(DebugCB);
		SUCCEEDED(device->CreateBuffer(&buff, &data, _cb_Debug.GetAddressOf()));

	}

	CD3D11_DEPTH_STENCIL_DESC ds_desc{ CD3D11_DEFAULT() };
	ds_desc.DepthFunc = D3D11_COMPARISON_GREATER_EQUAL;
	SUCCEEDED(device->CreateDepthStencilState(&ds_desc, _depth_state.GetAddressOf()));
	DirectX::SetDebugObjectName(_depth_state.Get(), L"Default DepthStencilState");

	CD3D11_BLEND_DESC bs_desc{ CD3D11_DEFAULT() };
	SUCCEEDED(device->CreateBlendState(&bs_desc, _blend_state.GetAddressOf()));
	DirectX::SetDebugObjectName(_blend_state.Get(), L"Default BlendState");

	CD3D11_RASTERIZER_DESC rs_desc{ CD3D11_DEFAULT() };
	SUCCEEDED(device->CreateRasterizerState(&rs_desc, _raster_state.GetAddressOf()));
	DirectX::SetDebugObjectName(_raster_state.Get(), L"Default RasterizerState");




	CD3D11_SAMPLER_DESC sampler{ CD3D11_DEFAULT() };
	sampler.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	SUCCEEDED(device->CreateSamplerState(&sampler, m_Samplers[uint32_t(Samplers::AllLinear)].GetAddressOf()));

}

void Hello3D::end()
{

}

void Hello3D::paint(graphics::D2DRenderContext& ctx)
{
}

void Hello3D::tick(double deltaTime)
{
	_world->update((float)deltaTime);

	_timer += (float)deltaTime;
}

void Hello3D::debug_ui()
{
	static bool s_open = true;
	ImGui::Begin("Game", &s_open);
	const char* items[] = {
		"Default",
		"Base Color",
		"Roughness",
		"Metalness",
		"Normals",
		"AO",
		"WorldNormals"
	};

	ImGui::Combo("Debug Mode", &g_DebugMode, items, 7);
	ImGui::End();

}

void Hello3D::render_3d()
{
	auto device = GameEngine::instance()->GetD3DDevice();
	auto ctx = GameEngine::instance()->GetD3DDeviceContext();

	// Use RH system so that we can directly export from blender
	//_timer = 0.0f;
	using namespace hlslpp;
	float4 view_direction = float4(0.0f, 0.0f, -2.0f, 0.0f);
	float4 light_direction = float4(0.0, -1.0, -1.0f,0.0f);
	LightComponent* comp = (LightComponent*)_world->find_first_component(LightComponent::get_static_type());
	if (comp)
	{
		float4x4 worldTransform = comp->get_entity()->get_world_transform();
		float3 fwd = float3(0.0f, 0.0f, 1.0f);
		light_direction = hlslpp::mul(worldTransform, float4(fwd, 0.0));
	}

	float4x4 View = float4x4::identity();

	ImVec2 size = GameEngine::instance()->get_viewport_size();
	const float aspect = (float)size.x / (float)size.y;
	const float near_plane = 0.01f;
	const float far_plane = 100.0f;

	hlslpp::projection proj(frustum::field_of_view_x(XMConvertToRadians(45.0f),aspect, far_plane, near_plane), zclip::zero);
	float4x4 Projection = float4x4::perspective(proj);


	ctx->OMSetDepthStencilState(_depth_state.Get(), 0);
	ctx->OMSetBlendState(_blend_state.Get(), NULL, 0xffffffff);
	ctx->RSSetState(_raster_state.Get());

	ID3D11SamplerState const* samplers[1] = {
		m_Samplers[uint32_t(Samplers::AllLinear)].Get()
	};
	ctx->PSSetSamplers(0, 1, (ID3D11SamplerState**)samplers);

	// Find our camera and use the matrices
	CameraComponent* camera = _world->find_first_component<CameraComponent>();
	View = hlslpp::float4x4::look_at(view_direction.xyz, { 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f });
	if (camera)
	{
		float4x4 cameraTransform = camera->get_entity()->get_world_transform();
		View = inverse(cameraTransform);

		hlslpp::projection proj(frustum::field_of_view_x(camera->get_fov(), aspect, camera->get_far_plane(), camera->get_near_plane()), zclip::zero);
		float4x4 Projection = float4x4::perspective(proj);

		// +Y is forward
		float4 world_fwd{ 0.0f, 0.0f, 1.0f, 0.0f };
		view_direction = mul(cameraTransform, world_fwd);
	}
	float4x4 invView = inverse(View);


	// Update debug mode
	if (g_DebugMode != DebugVisualizeMode::Default)
	{
		D3D11_MAPPED_SUBRESOURCE resource{};
		ctx->Map(_cb_Debug.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);

		DebugCB* buffer = (DebugCB*)resource.pData;
		buffer->m_VisualizeMode = g_DebugMode;
		ctx->Unmap(_cb_Debug.Get(), 0);

		ctx->PSSetConstantBuffers(1, 1, _cb_Debug.GetAddressOf());
	}


	for (auto& ent : _world->get_entities())
	{
		// If the entity has a mesh component and is done loading
		if (SimpleMeshComponent* mesh_comp = ent->get_component<SimpleMeshComponent>(); mesh_comp && mesh_comp->is_loaded())
		{
			float4x4 world = ent->get_world_transform();
			float4x4 MVP = mul(mul(world, View), Projection);

			D3D11_MAPPED_SUBRESOURCE resource{};
			ctx->Map(_cb_MVP.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
			MVPConstantBuffer* buffer = (MVPConstantBuffer*)resource.pData;
			buffer->World = world;
			buffer->WorldViewProjection = MVP;
			buffer->WorldView = mul(world, View);
			buffer->Projection = Projection;
			buffer->InvView = invView;
			buffer->View = View;
			buffer->ViewDirection = view_direction;
			buffer->LightDirection = light_direction;
			ctx->Unmap(_cb_MVP.Get(), 0);

			// TODO: Implement materials supplying buffers?
			ctx->VSSetConstantBuffers(0, 1, _cb_MVP.GetAddressOf());
			ctx->PSSetConstantBuffers(0, 1, _cb_MVP.GetAddressOf());

			mesh_comp->render();
		}
	}
}

