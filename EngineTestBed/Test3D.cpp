#include "stdafx.h"
#include "Test3D.h"

#include "Framework/framework.h"

#include <rttr/registration>
#include <rttr/type>

#include "EngineFiles/Core/ModelResource.h"


using framework::Entity;
using framework::Component;

__declspec(align(16))
struct MVPConstantBuffer
{
	XMFLOAT4X4 World;
	XMFLOAT4X4 WorldView;
	XMFLOAT4X4 Projection;
	XMFLOAT4X4 WorldViewProjection;

	XMFLOAT4X4 InvView;
	XMFLOAT4X4 View;

	XMFLOAT4 ViewDirection;
	XMFLOAT4 LightDirection;
};

class SimpleMeshComponent : public Component
{
public:
	SimpleMeshComponent() {};
	virtual ~SimpleMeshComponent() {};
	virtual void on_attach(Entity* ent) override
	{
		__super::on_attach(ent);
	}


	virtual void on_detach(Entity* ent) override
	{
		__super::on_detach(ent);
	}


	virtual void update(float dt) override
	{
	}


	virtual void render() override
	{
		auto ctx = GameEngine::Instance()->GetD3DDeviceContext();

		// Draw mesh
		ctx->VSSetShader(_resource->_vert_shader.Get(), nullptr,0);
		ctx->PSSetShader(_resource->_pixel_shader.Get(), nullptr, 0);

		ctx->IASetInputLayout(_resource->_input_layout.Get());
		ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		ctx->IASetIndexBuffer(_resource->_index_buffer.Get(), DXGI_FORMAT_R32_UINT, 0);
		UINT strides = { sizeof(DirectX::VertexPositionNormalColorTexture) };
		UINT offsets = { 0 };
		ctx->IASetVertexBuffers(0, 1, _resource->_vert_buffer.GetAddressOf(), &strides, &offsets);
		ctx->DrawIndexed(_resource->_index_count, 0, 0);
	}

	void load(std::string const& mesh)
	{
		_resource = ResourceLoader::Instance()->load<ModelResource>(mesh);
	}

private:
	std::shared_ptr<ModelResource> _resource;

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

	//TODO: Implement ModelLoader with a resource cache 
	{
		World::EntityId model = _world->create_entity();
		Entity* ent = _world->get_entity(model);
		ent->set_local_position({ 4.0f,0.0f,0.0f });
		auto comp = ent->create_component<SimpleMeshComponent>();
		comp->load("Resources/Models/Suzanne.fbx");
	}

	{
		World::EntityId model = _world->create_entity();
		Entity* ent = _world->get_entity(model);
		ent->set_local_position({ -4.0f,0.0f,0.0f });
		auto comp = ent->create_component<SimpleMeshComponent>();
		comp->load("Resources/Models/ball.fbx");
	}

	{
		World::EntityId model = _world->create_entity();
		Entity* ent = _world->get_entity(model);
		ent->set_local_position({ 0.0f,0.0f,0.0f });
		auto comp = ent->create_component<SimpleMeshComponent>();
		comp->load("Resources/Models/axes.fbx");

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
	}

	CD3D11_DEPTH_STENCIL_DESC ds_desc{ CD3D11_DEFAULT() };
	ds_desc.DepthFunc = D3D11_COMPARISON_GREATER_EQUAL;
	SUCCEEDED(device->CreateDepthStencilState(&ds_desc, _depth_state.GetAddressOf()));

	CD3D11_BLEND_DESC bs_desc{ CD3D11_DEFAULT() };
	SUCCEEDED(device->CreateBlendState(&bs_desc, _blend_state.GetAddressOf()));

	CD3D11_RASTERIZER_DESC rs_desc{ CD3D11_DEFAULT() };
	rs_desc.FrontCounterClockwise = true;
	SUCCEEDED(device->CreateRasterizerState(&rs_desc, _raster_state.GetAddressOf()));



}

void Hello3D::GameEnd()
{

}

void Hello3D::GamePaint(RECT rect)
{
}

void Hello3D::GameTick(double deltaTime)
{
	_world->update(deltaTime);

	_timer += deltaTime;
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
	XMVECTOR view_direction = XMVector3Create(10.0f * cos(_timer), 10.0f * sin(_timer), 10.0f);
	XMVECTOR light_direction = XMVector3Create(0.0, 0.0, -1.0);
	XMMATRIX View = XMMatrixLookAtRH(view_direction, XMVector3Create(0.0f, 0.0f, 0.0f), XMVector3Create(0.0f, 0.0f, 1.0f));

	float aspect = (float)GameEngine::Instance()->GetWidth() / (float)GameEngine::Instance()->GetHeight();
	float near_plane = 0.01f;
	float far_plane = 100.0f;
	XMMATRIX Projection = XMMatrixPerspectiveFovRH( XMConvertToRadians(45.0f), aspect, far_plane, near_plane);

	XMVECTOR det = XMMatrixDeterminant(View);
	XMMATRIX invView = XMMatrixInverse(&det, View);

	ctx->OMSetDepthStencilState(_depth_state.Get(), 0);
	ctx->OMSetBlendState(_blend_state.Get(), NULL, 0xffffffff);
	ctx->RSSetState(_raster_state.Get());

	for (auto& ent : _world->get_entities())
	{
		// If the entity has a mesh component
		if (auto comp = ent->get_component<SimpleMeshComponent>(); comp)
		{
			XMMATRIX world = ent->get_world_transform();
			XMMATRIX MVP = XMMatrixMultiply(XMMatrixMultiply(world, View), Projection);

			D3D11_MAPPED_SUBRESOURCE resource{};
			ctx->Map(_cb_MVP.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
			MVPConstantBuffer* buffer = (MVPConstantBuffer*)resource.pData;
			XMStoreFloat4x4(&buffer->World, world);
			XMStoreFloat4x4(&buffer->WorldViewProjection, MVP);
			XMStoreFloat4x4(&buffer->WorldView, XMMatrixMultiply(world, View));
			XMStoreFloat4x4(&buffer->Projection, Projection);
			XMStoreFloat4x4(&buffer->InvView, invView);
			XMStoreFloat4x4(&buffer->View, View);
			XMStoreFloat4(&buffer->ViewDirection, view_direction);
			XMStoreFloat4(&buffer->LightDirection, light_direction);
			ctx->Unmap(_cb_MVP.Get(), 0);

			// TODO: Implement materials supplying buffers?
			ctx->VSSetConstantBuffers(0, 1, _cb_MVP.GetAddressOf());
			ctx->PSSetConstantBuffers(0, 1, _cb_MVP.GetAddressOf());

			comp->render();

		}
	}
}

