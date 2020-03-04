#include "stdafx.h"
#include "ModelResource.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

// Inline shaders
namespace Shaders
{
#include "shaders/generated/simple_vx.h"
#include "shaders/generated/simple_px.h"
}



void ModelResource::load(std::string const& path)
{
	auto device = GameEngine::Instance()->GetD3DDevice();
	auto ctx = GameEngine::Instance()->GetD3DDeviceContext();

	using namespace Assimp;
	Importer importer{};
	aiScene const* scene = importer.ReadFile(path.c_str(), aiProcess_Triangulate | aiProcess_CalcTangentSpace);
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

