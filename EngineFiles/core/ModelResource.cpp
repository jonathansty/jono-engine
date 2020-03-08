#include "stdafx.h"
#include "ModelResource.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <DirectXTK/DirectXHelpers.h>


// Inline shaders
namespace Shaders
{
#include "shaders/generated/simple_vx.h"
#include "shaders/generated/simple_px.h"
}


const D3D11_INPUT_ELEMENT_DESC ModelVertex::InputElements[InputElementCount] = {
	{"SV_Position", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{"TANGENT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{"TANGENT", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT,  D3D11_INPUT_PER_VERTEX_DATA, 0 }
};
void ModelResource::load()
{
	std::string const& path = _init.path;
	auto device = GameEngine::Instance()->GetD3DDevice();
	auto ctx = GameEngine::Instance()->GetD3DDeviceContext();

	using namespace Assimp;
	Importer importer{};
	aiScene const* scene = importer.ReadFile(path.c_str(), aiProcess_Triangulate | aiProcess_CalcTangentSpace | aiProcess_ConvertToLeftHanded | aiProcess_CalcTangentSpace);
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

		std::vector<VertexType> vertices;
		std::vector<int> indices;
		vertices.reserve(mesh->mNumVertices);
		indices.reserve(int(mesh->mNumFaces) * 3);

		for (unsigned int i = 0; i < mesh->mNumVertices; ++i)
		{
			VertexType v{};
			v.position.x = positions[i].x;
			v.position.y = positions[i].y;
			v.position.z = positions[i].z;


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
				v.uv.x = p.x;
				v.uv.y = p.y;
			}

			if (mesh->HasNormals())
			{
				v.normal.x = mesh->mNormals[i].x;
				v.normal.y = mesh->mNormals[i].y;
				v.normal.z = mesh->mNormals[i].z;
			}

			if (mesh->HasTangentsAndBitangents())
			{
				v.tangent.x = mesh->mTangents[i].x;
				v.tangent.y = mesh->mTangents[i].y;
				v.tangent.z = mesh->mTangents[i].z;
				v.tangent.w = 0.0f;

				v.bitangent.x = mesh->mBitangents[i].x;
				v.bitangent.y = mesh->mBitangents[i].y;
				v.bitangent.z = mesh->mBitangents[i].z;
				v.bitangent.w = 0.0f;
			}

			vertices.push_back(v);

		}

		for (unsigned int i = 0; i < mesh->mNumFaces; ++i)
		{
			aiFace const& f = faces[i];
			for (unsigned int j = 0; j < f.mNumIndices; ++j)
			{
				indices.push_back(f.mIndices[j]);
			}
		}

		// Create our buffers
		D3D11_BUFFER_DESC bufferDesc{};
		D3D11_SUBRESOURCE_DATA data{};

		bufferDesc.ByteWidth = UINT(vertices.size() * sizeof(vertices[0]));
		bufferDesc.StructureByteStride = UINT(sizeof(vertices[0]));
		bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bufferDesc.CPUAccessFlags = 0;

		data.pSysMem = vertices.data();

		SUCCEEDED(device->CreateBuffer(&bufferDesc, &data, _vert_buffer.GetAddressOf()));


		bufferDesc.ByteWidth = UINT(indices.size() * sizeof(indices[0]));
		bufferDesc.StructureByteStride = sizeof(indices[0]);
		bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		data.pSysMem = indices.data();
		SUCCEEDED(device->CreateBuffer(&bufferDesc, &data, _index_buffer.GetAddressOf()));
		_index_count = indices.size();


	}

	// Create shaders
	SUCCEEDED(device->CreateVertexShader(Shaders::cso_simple_vx, std::size(Shaders::cso_simple_vx), nullptr, _vert_shader.GetAddressOf()));
	SUCCEEDED(device->CreatePixelShader(Shaders::cso_simple_px, std::size(Shaders::cso_simple_px), nullptr, _pixel_shader.GetAddressOf()));
	SUCCEEDED(device->CreateInputLayout(VertexType::InputElements, VertexType::InputElementCount, Shaders::cso_simple_vx, std::size(Shaders::cso_simple_vx), _input_layout.GetAddressOf()));

#ifdef _DEBUG
	char name[512];
	sprintf_s(name, "%s - Index Buffer", path.c_str());
	DirectX::SetDebugObjectName(_index_buffer.Get(), name);

	sprintf_s(name, "%s - Vertex Buffer", path.c_str());
	DirectX::SetDebugObjectName(_vert_buffer.Get(), name);
#endif

}

