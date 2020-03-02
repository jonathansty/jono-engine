#include "stdafx.h"
#include "Test3D.h"

#include "Framework/framework.h"

#include <rttr/registration>
#include <rttr/type>

class TestRTTR
{
	float data;
public:
};

RTTR_REGISTRATION
{
	using namespace rttr;
	registration::class_<TestRTTR>("TestRTTR")
		.constructor();

}

// Inline shaders
namespace Shaders
{
	#include "shaders/generated/simple_vx.h"
	#include "shaders/generated/simple_px.h"
}

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
		ctx->VSSetShader(_vert_shader.Get(), nullptr,0);
		ctx->PSSetShader(_pixel_shader.Get(), nullptr, 0);

		ctx->IASetInputLayout(_input_layout.Get());
		ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		ctx->IASetIndexBuffer(_index_buffer.Get(), DXGI_FORMAT_R32_UINT, 0);
		UINT strides = { sizeof(DirectX::VertexPositionNormalColorTexture) };
		UINT offsets = { 0 };
		ctx->IASetVertexBuffers(0, 1, _vert_buffer.GetAddressOf(), &strides, &offsets);
		ctx->DrawIndexed(_index_count, 0, 0);
	}

	void load(std::string const& mesh)
	{
		_mesh_path = mesh;

		auto device = GameEngine::Instance()->GetD3DDevice();
		auto ctx = GameEngine::Instance()->GetD3DDeviceContext();

		using namespace Assimp;
		Importer importer{};
		aiScene const* scene = importer.ReadFile(_mesh_path, aiProcess_Triangulate | aiProcess_CalcTangentSpace | aiProcess_MakeLeftHanded);
		if (!scene)
		{
			//TODO: Load error mesh
			return;
		}

		if (scene->HasMeshes())
		{
			aiMesh const* mesh = scene->mMeshes[0];

			auto positions = mesh->mVertices;
			auto colors = mesh->mColors;
			auto faces = mesh->mFaces;
			auto uv = mesh->mTextureCoords;

			std::vector<VertexPositionNormalColorTexture> vertices;
			std::vector<int> indices;
			vertices.reserve(mesh->mNumVertices);
			indices.reserve(int(mesh->mNumFaces) * 3);

			for (int i = 0; i < mesh->mNumVertices; ++i)
			{
				VertexPositionNormalColorTexture v{};
				v.position.x = positions[i].x;
				v.position.y = positions[i].y;
				v.position.z = positions[i].z;

				v.color = XMFLOAT4{ 1.0f,1.0f,1.0f,1.0f };

				if (mesh->HasVertexColors(0))
				{
					v.color.x = colors[0][i].r;
					v.color.y = colors[0][i].g;
					v.color.z = colors[0][i].b;
					v.color.w = colors[0][i].a;
				}

				if (mesh->GetNumUVChannels() > 0)
				{
					aiVector3D p = uv[0][i];
					v.textureCoordinate.x = p.x;
					v.textureCoordinate.y = p.y;
				}

				if (mesh->HasNormals())
				{
					v.normal.x = mesh->mNormals[i].x;
					v.normal.y = mesh->mNormals[i].y;
					v.normal.z = mesh->mNormals[i].z;
				}

				vertices.push_back(v);

			}

			int n = 0;
			for (int i = 0; i < mesh->mNumFaces; ++i)
			{
				aiFace const& f = faces[i];
				for (int j = 0; j < f.mNumIndices; ++j)
				{
					indices.push_back(f.mIndices[j]);
				}
			}

			// Create our buffers
			D3D11_BUFFER_DESC bufferDesc{};
			D3D11_SUBRESOURCE_DATA data{};

			bufferDesc.ByteWidth = vertices.size() * sizeof(vertices[0]);
			bufferDesc.StructureByteStride = sizeof(vertices[0]);
			bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
			bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			bufferDesc.CPUAccessFlags = 0;

			data.pSysMem = vertices.data();
			device->CreateBuffer(&bufferDesc, &data, _vert_buffer.GetAddressOf());

			bufferDesc.ByteWidth = indices.size() * sizeof(indices[0]);
			bufferDesc.StructureByteStride = sizeof(indices[0]);
			bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
			data.pSysMem = indices.data();
			device->CreateBuffer(&bufferDesc, &data, _index_buffer.GetAddressOf());
			_index_count = indices.size();

		}

			// Create shaders
		SUCCEEDED(device->CreateVertexShader(Shaders::cso_simple_vx, std::size(Shaders::cso_simple_vx), nullptr, _vert_shader.GetAddressOf()));
		SUCCEEDED(device->CreatePixelShader(Shaders::cso_simple_px, std::size(Shaders::cso_simple_px), nullptr, _pixel_shader.GetAddressOf()));
		SUCCEEDED(device->CreateInputLayout(DirectX::VertexPositionNormalColorTexture::InputElements, DirectX::VertexPositionNormalColorTexture::InputElementCount, Shaders::cso_simple_vx, std::size(Shaders::cso_simple_vx), _input_layout.GetAddressOf()));

	}

private:
	std::string _mesh_path;

	// Gpu resources
	size_t _index_count;


	// Shaders
	ComPtr<ID3D11InputLayout> _input_layout;
	ComPtr<ID3D11Buffer> _vert_buffer;
	ComPtr<ID3D11Buffer> _index_buffer;
	ComPtr<ID3D11VertexShader> _vert_shader;
	ComPtr<ID3D11PixelShader> _pixel_shader;


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
		auto comp = ent->create_component<SimpleMeshComponent>();
		comp->load("Resources/Models/Suzanne.fbx");
	}

	{
		World::EntityId model = _world->create_entity();
		Entity* ent = _world->get_entity(model);
		ent->set_local_position({ -1.0f,0.0f,0.0f });
		auto comp = ent->create_component<SimpleMeshComponent>();
		comp->load("Resources/Models/Suzanne.fbx");
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

	// Get camera entity
	XMMATRIX View = XMMatrixLookAtLH(XMVector3Create(0.0f, 10.0f, 10.0f), XMVector3Create(0.0f, 0.0f, 0.0f), XMVector3Create(0.0f, 1.0f, 0.0f));
	XMMATRIX Projection = XMMatrixPerspectiveFovLH( XMConvertToRadians(45.0f), 1.0f, 0.01f, 100.0f);

	XMVECTOR det = XMMatrixDeterminant(View);
	XMMATRIX invView = XMMatrixInverse(&det, View);

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
			ctx->Unmap(_cb_MVP.Get(), 0);

			// TODO: Implement materials supplying buffers?
			ctx->VSSetConstantBuffers(0, 1, _cb_MVP.GetAddressOf());
			ctx->PSSetConstantBuffers(0, 1, _cb_MVP.GetAddressOf());

			comp->render();

		}
	}
}

