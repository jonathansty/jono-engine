#include "engine.pch.h"
#include "ModelResource.h"

#include "TextureResource.h"
#include "MaterialResource.h"

#include "GameEngine.h"


// Inline shaders
namespace Shaders
{
#include "simple_vx.h"
#include "simple_px.h"
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
	std::string const& path = get_init_parameters().path;

	std::string dir_path = path;
	dir_path = dir_path.substr(0, dir_path.rfind('/') + 1);
	auto device = GameEngine::instance()->GetD3DDevice();
	auto ctx = GameEngine::instance()->GetD3DDeviceContext();

	using namespace Assimp;
	Importer importer{};
	aiScene const* scene = importer.ReadFile(path.c_str(), aiProcess_Triangulate | aiProcess_CalcTangentSpace | aiProcess_ConvertToLeftHanded | aiProcess_CalcTangentSpace);
	if (!scene)
	{
		GameEngine::instance()->print_string(importer.GetErrorString());
		return;
	}

	if (scene->HasMeshes())
	{
		std::vector<VertexType> vertices;
		std::vector<uint32_t> indices;
		_meshes.clear();

		std::map<int, aiMatrix4x4> transforms;

		struct Flattener {
			void operator()(aiMatrix4x4 parent, aiNode* node, std::map<int, aiMatrix4x4>& result)
			{
				aiMatrix4x4 t = node->mTransformation* parent;
				for (unsigned int i = 0; i < node->mNumMeshes; ++i)
				{
					result[node->mMeshes[i]] = t;
				}

				for (unsigned int i = 0; i < node->mNumChildren; ++i)
				{
					operator()(t, node->mChildren[i], result);
				}

			}
		};
		aiNode* root = scene->mRootNode;
		Flattener fn{};
		fn(aiMatrix4x4(), root, transforms);

		//TODO: Implement transforms from assimp
		for (unsigned int i = 0; i < scene->mNumMeshes; ++i)
		{
			aiMesh const* mesh = scene->mMeshes[i];
			aiMatrix4x4 const& transform = transforms[i];


			aiMatrix3x3 normalTransform = aiMatrix3x3{
				transform.a1, transform.a2,transform.a3,
				transform.b1, transform.b2,transform.b3,
				transform.c1, transform.c2,transform.c3
			};


			Mesh meshlet{};
			meshlet.firstIndex = indices.size();
			meshlet.firstVertex = vertices.size();
			meshlet.indexCount = uint32_t(mesh->mNumFaces) * 3;
			meshlet.materialID = mesh->mMaterialIndex;
			_meshes.push_back(meshlet);

			auto positions = mesh->mVertices;
			auto colors = mesh->mColors;
			auto faces = mesh->mFaces;
			auto uv = mesh->mTextureCoords;

			vertices.reserve(mesh->mNumVertices);
			indices.reserve(int(mesh->mNumFaces) * 3);

			for (unsigned int j = 0; j < mesh->mNumVertices; ++j)
			{
				aiVector3D pos = positions[j];
				pos *= transform;

				VertexType v{};
				v.position.x = pos.x;
				v.position.y = pos.y;
				v.position.z = pos.z;


				if (mesh->HasVertexColors(0))
				{
					v.color.x = colors[0][j].r;
					v.color.y = colors[0][j].g;
					v.color.z = colors[0][j].b;
					v.color.w = colors[0][j].a;
				}

				if (mesh->GetNumUVChannels() > 0)
				{
					aiVector3D p = uv[0][j];
					v.uv.x = p.x;
					v.uv.y = p.y;
				}

				if (mesh->HasNormals())
				{
					aiVector3D normal = mesh->mNormals[j];
					normal *= normalTransform;

					v.normal.x = normal.x;
					v.normal.y = normal.y;
					v.normal.z = normal.z;
				}

				if (mesh->HasTangentsAndBitangents())
				{
					aiVector3D tangent = mesh->mTangents[j];
					tangent *= normalTransform;

					aiVector3D bitangent = mesh->mBitangents[j];
					bitangent *= normalTransform;

					v.tangent.x = tangent.x;
					v.tangent.y = tangent.y;
					v.tangent.z = tangent.z;
					v.tangent.w = 0.0f;

					v.bitangent.x = bitangent.x;
					v.bitangent.y = bitangent.y;
					v.bitangent.z = bitangent.z;
					v.bitangent.w = 0.0f;
				}

				vertices.push_back(v);

			}

			for (unsigned int k = 0; k < mesh->mNumFaces; ++k)
			{
				aiFace const& f = faces[k];
				for (unsigned int idx = 0; idx < f.mNumIndices; ++idx)
				{
					indices.push_back(f.mIndices[idx]);
				}
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


	if (scene->HasMaterials())
	{
		for (unsigned int i = 0; i < scene->mNumMaterials; ++i)
		{
			aiMaterial* material = scene->mMaterials[i];
			aiString name = material->GetName();

			aiString materialPath{};
			MaterialInitParameters parameters{};
			parameters.load_type = MaterialInitParameters::LoadType_FromMemory;
			parameters.name = "[Built-in] Simple Shader";

			parameters.vs_shader_bytecode = (const char*)Shaders::cso_simple_vx;
			parameters.vs_shader_bytecode_size = uint32_t(std::size(Shaders::cso_simple_vx));

			parameters.ps_shader_bytecode = (const char*)Shaders::cso_simple_px;
			parameters.ps_shader_bytecode_size = uint32_t(std::size(Shaders::cso_simple_px));

			if (material->GetTexture(AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_BASE_COLOR_TEXTURE, &materialPath) == aiReturn_SUCCESS)
			{
				parameters.m_texture_paths[MaterialInitParameters::TextureType_Albedo] = dir_path + std::string(materialPath.C_Str());
			}
			if (material->GetTexture(AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_METALLICROUGHNESS_TEXTURE, &materialPath) == aiReturn_SUCCESS)
			{
				parameters.m_texture_paths[MaterialInitParameters::TextureType_MetalnessRoughness] = dir_path + std::string(materialPath.C_Str());
			}
			if (material->Get(AI_MATKEY_TEXTURE_NORMALS(0), materialPath) == aiReturn_SUCCESS)
			{
				parameters.m_texture_paths[MaterialInitParameters::TextureType_Normal] = dir_path + std::string(materialPath.C_Str());
			}

			bool double_sided;
			if (material->Get(AI_MATKEY_TWOSIDED, double_sided) == aiReturn_SUCCESS) {
				parameters.double_sided = double_sided;
			}


			auto handle = ResourceLoader::instance()->load<MaterialResource>(parameters, true);
			_materials.push_back(handle);
		}
	}


#ifdef _DEBUG
	char name[512];
	sprintf_s(name, "%s - Index Buffer", path.c_str());
	helpers::SetDebugObjectName(_index_buffer.Get(), name);

	sprintf_s(name, "%s - Vertex Buffer", path.c_str());
	helpers::SetDebugObjectName(_vert_buffer.Get(), name);
#endif

}



ModelResource::ModelResource(FromFileResourceParameters params) : TCachedResource(params)
, _index_count(0)
{

}

