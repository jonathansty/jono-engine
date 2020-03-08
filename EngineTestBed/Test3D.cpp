#include "stdafx.h"
#include "Test3D.h"

#include "Framework/framework.h"
#include "Components.h"

#include <rttr/registration>
#include <rttr/type>

#include "EngineFiles/Core/ModelResource.h"
#include "EngineFiles/Core/TextureResource.h"


using framework::Entity;
using framework::Component;

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


void Hello3D::GameInitialize(GameSettings& gameSettings)
{
	gameSettings.EnableConsole(true);
}

void Hello3D::GameStart()
{
	auto device = GameEngine::Instance()->GetD3DDevice();
	auto ctx = GameEngine::Instance()->GetD3DDeviceContext();

	using namespace framework;
	_world = std::make_unique<framework::World>();

	// Create the world camera
	{
		World::EntityId cam_id = _world->create_entity();
		Entity* ent = _world->get_entity(cam_id);
		auto comp = ent->create_component<CameraComponent>();
		ent->set_local_position(XMFLOAT3(0.0f, 0.0f, -2.0f));

	}

	//TODO: Implement ModelLoader with a resource cache 
	//{
	//	World::EntityId model = _world->create_entity();
	//	Entity* ent = _world->get_entity(model);
	//	ent->set_local_position({ 4.0f,0.0f,0.0f });
	//	auto comp = ent->create_component<SimpleMeshComponent>();
	//	comp->load("Resources/Models/Suzanne.fbx");
	//}

	{
		World::EntityId model = _world->create_entity();
		Entity* ent = _world->get_entity(model);
		auto comp = ent->create_component<SimpleMeshComponent>();
		comp->load("Resources/Models/ball.fbx");
		auto mov = ent->create_component<SimpleMovement>();
		mov->set_speed(1.0f);
	}

	//{
	//	World::EntityId model = _world->create_entity();
	//	Entity* ent = _world->get_entity(model);
	//	ent->set_local_position({ 0.0f,0.0f,0.0f });
	//	auto comp = ent->create_component<SimpleMeshComponent>();
	//	comp->load("Resources/Models/axes.fbx");
	//}


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
	}

	CD3D11_DEPTH_STENCIL_DESC ds_desc{ CD3D11_DEFAULT() };
	ds_desc.DepthFunc = D3D11_COMPARISON_GREATER_EQUAL;
	SUCCEEDED(device->CreateDepthStencilState(&ds_desc, _depth_state.GetAddressOf()));
	DirectX::SetDebugObjectName(_depth_state.Get(), L"Default DepthStencilState");

	CD3D11_BLEND_DESC bs_desc{ CD3D11_DEFAULT() };
	SUCCEEDED(device->CreateBlendState(&bs_desc, _blend_state.GetAddressOf()));
	DirectX::SetDebugObjectName(_blend_state.Get(), L"Default BlendState");

	CD3D11_RASTERIZER_DESC rs_desc{ CD3D11_DEFAULT() };
	rs_desc.FrontCounterClockwise = true;
	SUCCEEDED(device->CreateRasterizerState(&rs_desc, _raster_state.GetAddressOf()));
	DirectX::SetDebugObjectName(_raster_state.Get(), L"Default RasterizerState");

	// Load our textures
	g_Materials.albedo = ResourceLoader::Instance()->load<TextureResource>({ "Resources/Textures/pitted-metal-bl/pitted-metal_albedo.png" });
	g_Materials.roughness = ResourceLoader::Instance()->load<TextureResource>({ "Resources/Textures/pitted-metal-bl/pitted-metal_roughness.png" });
	g_Materials.metalness = ResourceLoader::Instance()->load<TextureResource>({ "Resources/Textures/pitted-metal-bl/pitted-metal_metallic.png" });
	g_Materials.normal = ResourceLoader::Instance()->load<TextureResource>({ "Resources/Textures/pitted-metal-bl/pitted-metal_normal-ogl.png" });

	CD3D11_SAMPLER_DESC sampler{ CD3D11_DEFAULT() };
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

	ID3D11ShaderResourceView const* views[4] = {
		g_Materials.albedo->get_srv(),
		g_Materials.normal->get_srv(),
		g_Materials.metalness->get_srv(),
		g_Materials.roughness->get_srv()
	};
	ctx->PSSetShaderResources(0, 4, (ID3D11ShaderResourceView**)views);

	ID3D11SamplerState const* samplers[1] = {
		m_Samplers[uint32_t(Samplers::AllLinear)].Get()
	};
	ctx->PSSetSamplers(0, 1, (ID3D11SamplerState**)samplers);

	CameraComponent* camera = _world->find_first_component<CameraComponent>();
	View = XMMatrixLookAtLH(view_direction, XMVector3Create(0.0f,0.0f,0.0f), XMVector3Create(0.0f,1.0f,0.0f));
	if (camera)
	{
		View = XMMatrixInverse(nullptr,camera->get_entity()->get_local_transform());
		Projection = XMMatrixPerspectiveFovLH(XMConvertToRadians(camera->get_fov()), aspect, camera->get_far_plane(), camera->get_near_plane());

		// +Y is forward
		XMFLOAT4 forward = { 0.0f,0.0f,-1.0f, 0.0f };
		XMVECTOR world_fwd = XMLoadFloat4(&forward);
		view_direction = XMVector3Transform(world_fwd, View);
	}
	XMMATRIX invView = XMMatrixInverse(nullptr, View);

	for (auto& ent : _world->get_entities())
	{
		// If the entity has a mesh component
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
			XMStoreFloat4(&buffer->ViewDirection, (-1 * view_direction));
			XMStoreFloat4(&buffer->LightDirection, (light_direction));
			ctx->Unmap(_cb_MVP.Get(), 0);

			// TODO: Implement materials supplying buffers?
			ctx->VSSetConstantBuffers(0, 1, _cb_MVP.GetAddressOf());
			ctx->PSSetConstantBuffers(0, 1, _cb_MVP.GetAddressOf());

			comp->render();

		}
	}
}

