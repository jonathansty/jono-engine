#include "stdafx.h"
#include "Test3D.h"

#include "EngineFiles/Core/ModelResource.h"
#include "EngineFiles/Core/TextureResource.h"
#include "EngineFiles/Core/MaterialResource.h"

#include "Framework/framework.h"
#include "Components.h"
#include "Overlays.h"

using framework::Entity;
using framework::Component;

namespace Shaders {
#include "Shaders/generated/simple_px.h"
#include "Shaders/generated/simple_vx.h"
#include "Shaders/generated/debug_px.h"
}

__declspec(align(16))
struct MVPConstantBuffer
{
	XMFLOAT4X4 World;
	XMFLOAT4X4 WorldView;
	XMFLOAT4X4 Projection;
	XMFLOAT4X4 WorldViewProjection;

	XMFLOAT4X4 View;
	XMFLOAT4X4 InvView;

	XMFLOAT4 ViewDirection;
	XMFLOAT4 LightDirection;
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
};

int g_DebugMode = 0;

void Hello3D::GameInitialize(GameSettings& gameSettings)
{
	gameSettings.EnableConsole(true);
	gameSettings.EnableAntiAliasing(true);
}


void Hello3D::GameStart()
{
	auto device = GameEngine::Instance()->GetD3DDevice();
	auto ctx = GameEngine::Instance()->GetD3DDeviceContext();



	using namespace framework;
	_world = std::make_unique<framework::World>();

	GameEngine::Instance()->get_overlay_manager()->register_overlay(new EntityDebugOverlay(_world.get()));

	// Create the world camera
	{
		World::EntityId cam_id = _world->create_entity();
		Entity* ent = _world->get_entity(cam_id);
		ent->set_name("MainCamera");
		auto comp = ent->create_component<CameraComponent>();
		ent->set_local_position(XMFLOAT3(0.0f, 0.0f, -2.0f));
	}

	//{
	//	World::EntityId model = _world->create_entity();
	//	Entity* ent = _world->get_entity(model);
	//	ent->set_name("Suzanne");
	//	ent->set_local_position({ 4.0f,0.0f,0.0f });
	//	auto comp = ent->create_component<SimpleMeshComponent>();
	//	comp->load("Resources/Models/Suzanne.fbx");
	//}

	//{
	//	World::EntityId model = _world->create_entity();
	//	Entity* ent = _world->get_entity(model);
	//	ent->set_name("Ball");
	//	ent->set_local_position({ -4.0f, 0.0f, 0.0f });
	//	auto comp = ent->create_component<SimpleMeshComponent>();
	//	comp->load("Resources/Models/ball.fbx");
	//	auto mov = ent->create_component<SimpleMovement>();
	//	mov->set_speed(1.0f);
	//}

	//{
	//	World::EntityId model = _world->create_entity();
	//	Entity* ent = _world->get_entity(model);
	//	ent->set_name("Axes");
	//	ent->set_local_position({ 0.0f,0.0f,0.0f });
	//	auto comp = ent->create_component<SimpleMeshComponent>();
	//	comp->load("Resources/Models/axes.fbx");
	//}

	{
		World::EntityId model = _world->create_entity();
		Entity* ent = _world->get_entity(model);
		ent->set_name("boxes_3");
		ent->set_local_position({ 0.0f,0.0f,0.0f });
		ent->set_local_scale({ 10.0f, 10.0f,10.0f });
		auto comp = ent->create_component<SimpleMeshComponent>();
		comp->load("Resources/Models/m-96_mattock/scene.gltf");

		auto c = ent->create_component<SimpleMovement>();
		c->set_speed(10.0f);
		//comp->load("Resources/Models/boxes_3.fbx");

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

void Hello3D::GameEnd()
{

}

void Hello3D::GamePaint(RECT rect)
{
}

void Hello3D::GameTick(double deltaTime)
{
	_world->update((float)deltaTime);

	_timer += (float)deltaTime;
}

void Hello3D::DebugUI()
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

XMVECTOR XMVector4Create(float x, float y, float z, float w)
{
	XMFLOAT4 v = { x,y,z,w };
	return XMLoadFloat4(&v);
}

XMVECTOR XMVector3Create(float x, float y, float z)
{
	return XMVector4Create(x, y, z, 0.0);
}

void Hello3D::Render3D()
{
	auto device = GameEngine::Instance()->GetD3DDevice();
	auto ctx = GameEngine::Instance()->GetD3DDeviceContext();

	// Use RH system so that we can directly export from blender
	//_timer = 0.0f;
	XMVECTOR view_direction = XMVector3Create(0.0f, 0.0f, -2.0f);
	XMVECTOR light_direction = XMVector3Create(0.0, -1.0, -1.0f);
	XMMATRIX View = XMMatrixIdentity();

	float aspect = (float)GameEngine::Instance()->GetWidth() / (float)GameEngine::Instance()->GetHeight();
	float near_plane = 0.01f;
	float far_plane = 100.0f;
	XMMATRIX Projection = XMMatrixPerspectiveFovLH( XMConvertToRadians(45.0f), aspect, far_plane, near_plane);


	ctx->OMSetDepthStencilState(_depth_state.Get(), 0);
	ctx->OMSetBlendState(_blend_state.Get(), NULL, 0xffffffff);
	ctx->RSSetState(_raster_state.Get());

	ID3D11SamplerState const* samplers[1] = {
		m_Samplers[uint32_t(Samplers::AllLinear)].Get()
	};
	ctx->PSSetSamplers(0, 1, (ID3D11SamplerState**)samplers);

	// Find our camera and use the matrices
	CameraComponent* camera = _world->find_first_component<CameraComponent>();
	View = XMMatrixLookAtLH(view_direction, XMVector3Create(0.0f,0.0f,0.0f), XMVector3Create(0.0f,1.0f,0.0f));
	if (camera)
	{
		View = XMMatrixInverse(nullptr,camera->get_entity()->get_world_transform());
		Projection = XMMatrixPerspectiveFovLH(XMConvertToRadians(camera->get_fov()), aspect, camera->get_far_plane(), camera->get_near_plane());

		// +Y is forward
		XMFLOAT4 forward = { 0.0f,0.0f,1.0f, 0.0f };
		XMVECTOR world_fwd = XMLoadFloat4(&forward);
		view_direction = XMVector3Transform(world_fwd, camera->get_entity()->get_world_transform());
	}
	XMMATRIX invView = XMMatrixInverse(nullptr, View);


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
		if (SimpleMeshComponent* comp = ent->get_component<SimpleMeshComponent>(); comp && comp->is_loaded())
		{
			XMMATRIX world = ent->get_world_transform();
			XMMATRIX MVP = XMMatrixMultiply(XMMatrixMultiply(world, View), Projection);

			D3D11_MAPPED_SUBRESOURCE resource{};
			ctx->Map(_cb_MVP.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
			MVPConstantBuffer* buffer = (MVPConstantBuffer*)resource.pData;
			XMStoreFloat4x4(&buffer->World, (world));
			XMStoreFloat4x4(&buffer->WorldViewProjection, (MVP));
			XMStoreFloat4x4(&buffer->WorldView, (XMMatrixMultiply(world, View)));
			XMStoreFloat4x4(&buffer->Projection, (Projection));
			XMStoreFloat4x4(&buffer->InvView, (invView));
			XMStoreFloat4x4(&buffer->View, (View));
			XMStoreFloat4(&buffer->ViewDirection, ( view_direction));
			XMStoreFloat4(&buffer->LightDirection, (light_direction));
			ctx->Unmap(_cb_MVP.Get(), 0);

			// TODO: Implement materials supplying buffers?
			ctx->VSSetConstantBuffers(0, 1, _cb_MVP.GetAddressOf());
			ctx->PSSetConstantBuffers(0, 1, _cb_MVP.GetAddressOf());

			comp->render();

		}
	}
}

