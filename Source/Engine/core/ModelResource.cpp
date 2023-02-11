#include "engine.pch.h"
#include "ModelResource.h"

#include "MaterialResource.h"
#include "TextureResource.h"
#include "Material.h"

#include "GameEngine.h"

const D3D11_INPUT_ELEMENT_DESC ModelUberVertex::InputElements[InputElementCount] = {
	{ "SV_Position", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TANGENT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TANGENT", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD", 2, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD", 3, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
};

void ModelHandle::load(enki::ITaskSet* parent)
{
	std::string const& path = get_init_parameters().path;
	_resource = std::make_shared<Model>();
	_resource->Load(parent, path);
}

ModelHandle::ModelHandle(FromFileResourceParameters params)
		: TCachedResource(params)
{
}

 ModelHandle::~ModelHandle()
{
}

 Model::Model()
		: _index_count(0)
		, m_Materials()
		, m_Meshes()
		, m_VertexBuffer()
		, m_IndexBuffer()
{
}

 Model::~Model()
{
    GetRI()->ReleaseResource(m_IndexBuffer);
    GetRI()->ReleaseResource(m_VertexBuffer);
 }

void Model::Load(enki::ITaskSet* parent, std::string const& path)
{
	Timer timer{}; 
	timer.Start();

	std::filesystem::path dir_path = std::filesystem::path(path);
	dir_path = dir_path.parent_path();

	std::string final_path = IO::get()->ResolvePath(path);
	auto device = Graphics::get_device();
	auto ctx = Graphics::get_ctx();

	using namespace Assimp;
	Importer importer = Importer();
	aiScene const* scene = importer.ReadFile(final_path.c_str(), aiProcess_ConvertToLeftHanded | aiProcessPreset_TargetRealtime_Fast);
	if (!scene)
	{
		LOG_ERROR(IO, importer.GetErrorString());
		return;
	}

	if (scene->HasMeshes())
	{
		std::vector<VertexType> vertices;
		std::vector<u32> indices;
		m_Meshes.clear();

		std::map<int, aiMatrix4x4> transforms;

		// Helper structure to flatten a assimp scene and it's traansforms
		struct Flattener
		{
			void operator()(aiMatrix4x4 parent, aiNode* node, std::map<int, aiMatrix4x4>& result)
			{
				aiMatrix4x4 t = node->mTransformation * parent;
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

		m_AABB.min = (float3)(std::numeric_limits<float>::max());
		m_AABB.max = (float3)(- std::numeric_limits<float>::max());
		for (unsigned int i = 0; i < scene->mNumMeshes; ++i)
		{
			aiMesh const* mesh = scene->mMeshes[i];
			aiMatrix4x4 const& transform = transforms[i];

			aiMatrix3x3 normalTransform = aiMatrix3x3{
				transform.a1, transform.a2, transform.a3,
				transform.b1, transform.b2, transform.b3,
				transform.c1, transform.c2, transform.c3
			};

			Mesh meshlet{};
			meshlet.firstIndex = indices.size();
			meshlet.firstVertex = vertices.size();
			meshlet.indexCount = uint32_t(mesh->mNumFaces) * 3;
			meshlet.material_index = mesh->mMaterialIndex;
			m_Meshes.push_back(meshlet);

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

				// Update local AABB
				m_AABB.min = hlslpp::min(v.position, m_AABB.min);
				m_AABB.max = hlslpp::max(v.position, m_AABB.max);

				if (mesh->HasVertexColors(0))
				{
					v.color.x = colors[0][j].r;
					v.color.y = colors[0][j].g;
					v.color.z = colors[0][j].b;
					v.color.w = colors[0][j].a;
				}

				{
					u32 n_uv = mesh->GetNumUVChannels();
					if(n_uv > 0)
					{
						v.uv0.x = uv[0][j].x;
						v.uv0.y = uv[0][j].y;
					}

					if (n_uv > 1)
					{
						v.uv0.x = uv[1][j].x;
						v.uv0.y = uv[1][j].y;
					}
					if (n_uv > 2)
					{
						v.uv0.x = uv[2][j].x;
						v.uv0.y = uv[2][j].y;
					}
					if (n_uv > 3)
					{
						v.uv0.x = uv[3][j].x;
						v.uv0.y = uv[3][j].y;
					}
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


        char name[512];
        sprintf_s(name, "%s - Index Buffer", path.c_str());

		m_VertexBuffer = GetRI()->CreateBuffer(bufferDesc, &data, name);
        ASSERT(m_VertexBuffer.IsValid());

		bufferDesc.ByteWidth = UINT(indices.size() * sizeof(indices[0]));
		bufferDesc.StructureByteStride = sizeof(indices[0]);
		bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		data.pSysMem = indices.data();

        sprintf_s(name, "%s - Vertex Buffer", path.c_str());
        m_IndexBuffer = GetRI()->CreateBuffer(bufferDesc, &data, name);
        ASSERT(m_IndexBuffer.IsValid());
		_index_count = indices.size();
	}

	if (scene->HasMaterials())
	{
		MaterialInitParameters parameters{};
		parameters.load_type = MaterialInitParameters::LoadType_FromFile;
		parameters.name = IO::get()->ResolvePath("res:/Engine/default.material");

		auto base_material = ResourceLoader::instance()->load<MaterialHandle>(parameters, false, true);

		// Resize our materials and textures 
		m_Materials.resize(scene->mNumMaterials);
		for(u32 j = 0; j < m_Materials.size(); ++j)
		{
			aiMaterial* material = scene->mMaterials[j];
			aiString name = material->GetName();


			bool double_sided;
			if (material->Get(AI_MATKEY_TWOSIDED, double_sided) == aiReturn_SUCCESS)
			{
				parameters.double_sided = double_sided;
			}

			m_Materials[j] = std::make_unique<MaterialInstance>(base_material);

			// Load the required textures from the assimp imported file
			aiString baseColorTexture;
			aiString roughnessTexture;
			aiString normalTexture;
			aiString metalnessTexture;
			aiString aoTexture;
			aiString emissiveTexture;

			if (material->GetTexture(AI_MATKEY_BASE_COLOR_TEXTURE, &baseColorTexture) == aiReturn_SUCCESS)
			{
				std::string const& tex_path = dir_path.string() + "\\" + std::string(baseColorTexture.C_Str());

				FromFileResourceParameters params{ tex_path };
				auto texture = ResourceLoader::instance()->load<TextureHandle>(params, true, true);
				u32 slot = m_Materials[j]->get_slot("Albedo");
				m_Materials[j]->set_texture(slot, texture);

			}
			if (material->GetTexture(AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_METALLICROUGHNESS_TEXTURE, &roughnessTexture) == aiReturn_SUCCESS)
			{
				std::string const& tex_path = dir_path.string() + "\\" + std::string(roughnessTexture.C_Str());

				FromFileResourceParameters params{ tex_path };
				auto texture = ResourceLoader::instance()->load<TextureHandle>(params, true, true);
				u32 slot = m_Materials[j]->get_slot("MetalnessRoughness");
				m_Materials[j]->set_texture(slot, texture);
			}

			if (material->GetTexture(aiTextureType_NORMALS, 0, &normalTexture) == aiReturn_SUCCESS)
			{
				std::string const& tex_path = dir_path.string() + "\\" + std::string(normalTexture.C_Str());

				FromFileResourceParameters params{ tex_path };
				auto texture = ResourceLoader::instance()->load<TextureHandle>(params, true, true);
				u32 slot = m_Materials[j]->get_slot("Normals");
				m_Materials[j]->set_texture(slot, texture);

			}

			if (material->GetTexture(aiTextureType_LIGHTMAP, 0, &aoTexture) == aiReturn_SUCCESS)
			{
				std::string const& tex_path = dir_path.string() + "\\" + std::string(aoTexture.C_Str());

				FromFileResourceParameters params{ tex_path };
				auto texture = ResourceLoader::instance()->load<TextureHandle>(params, true, true);
				u32 slot = m_Materials[j]->get_slot("AO");
				m_Materials[j]->set_texture(slot, texture);
			}

			if (material->GetTexture(aiTextureType_EMISSIVE, 0, &emissiveTexture) == aiReturn_SUCCESS)
			{
				std::string const& tex_path = dir_path.string() + "\\" + std::string(emissiveTexture.C_Str());

				FromFileResourceParameters params{ tex_path };
				auto texture = ResourceLoader::instance()->load<TextureHandle>(params, true, true);
				u32 slot = m_Materials[j]->get_slot("Emissive");
				m_Materials[j]->set_texture(slot, texture);
			}


		}
	}


	timer.Stop();
	LOG_VERBOSE(IO, "Loading model \"{}\" took {} ", path.c_str(), timer.GetTime());
}
