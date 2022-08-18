#include "engine.pch.h"
#include "ModelResource.h"

#include "MaterialResource.h"
#include "TextureResource.h"

#include "GameEngine.h"

const D3D11_INPUT_ELEMENT_DESC ModelUberVertex::InputElements[InputElementCount] = {
	{ "SV_Position", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TANGENT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TANGENT", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
};

void ModelResource::build_load_graph(enki::ITaskSet* parent)
{
}

void ModelResource::load(enki::ITaskSet* parent)
{
	std::string const& path = get_init_parameters().path;
	_resource = std::make_shared<Model>();
	_resource->load(parent, path);
}

ModelResource::ModelResource(FromFileResourceParameters params)
		: TCachedResource(params)
{
}

void Model::load(enki::ITaskSet* parent, std::string const& path)
{
	Timer timer{}; 
	timer.Start();

	std::filesystem::path dir_path = std::filesystem::path(path);
	dir_path = dir_path.parent_path();
	auto device = Graphics::get_device();
	auto ctx = Graphics::get_ctx();

	using namespace Assimp;
	Importer importer{};
	aiScene const* scene = importer.ReadFile(path.c_str(),  aiProcess_ConvertToLeftHanded | aiProcessPreset_TargetRealtime_Fast);
	if (!scene)
	{
		LOG_ERROR(IO, importer.GetErrorString());
		return;
	}

	if (scene->HasMeshes())
	{
		std::vector<VertexType> vertices;
		std::vector<u32> indices;
		_meshes.clear();

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

		SUCCEEDED(device->CreateBuffer(&bufferDesc, &data, _vertex_buffer.GetAddressOf()));

		bufferDesc.ByteWidth = UINT(indices.size() * sizeof(indices[0]));
		bufferDesc.StructureByteStride = sizeof(indices[0]);
		bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		data.pSysMem = indices.data();
		SUCCEEDED(device->CreateBuffer(&bufferDesc, &data, _index_buffer.GetAddressOf()));
		_index_count = indices.size();
	}

	if (scene->HasMaterials())
	{
		// Resize our materials and textures 
		_materials.resize(scene->mNumMaterials);

		struct TextureMappings
		{
			u32 tex_idx;
			u32 mat_idx;

			std::string path;
		};
		std::vector<TextureMappings> mappings;
		for(u32 i = 0; i < scene->mNumMaterials; ++i)
		{
			aiMaterial* material = scene->mMaterials[i];
			aiString texturePath;
			if (material->GetTexture(AI_MATKEY_BASE_COLOR_TEXTURE, &texturePath) == aiReturn_SUCCESS)
			{
				mappings.push_back(TextureMappings{ u32(_textures.size()), i, dir_path.string() + "\\" + std::string(texturePath.C_Str()) });
				_textures.push_back({});
			}
			if (material->GetTexture(AI_MATKEY_ROUGHNESS_TEXTURE, &texturePath) == aiReturn_SUCCESS)
			{
				mappings.push_back(TextureMappings{ u32(_textures.size()), i, dir_path.string() + "\\" + std::string(texturePath.C_Str()) });
				_textures.push_back({});
			}
			if (material->GetTexture(AI_MATKEY_METALLIC_TEXTURE, &texturePath) == aiReturn_SUCCESS)
			{
				mappings.push_back(TextureMappings{ u32(_textures.size()), i, dir_path.string() + "\\" + std::string(texturePath.C_Str()) });
				_textures.push_back({});
			}
			if (material->GetTexture(aiTextureType_NORMALS, 0, &texturePath) == aiReturn_SUCCESS)
			{
				mappings.push_back(TextureMappings{ u32(_textures.size()), i, dir_path.string() + "\\" + std::string(texturePath.C_Str()) });
				_textures.push_back({});
			}
		}



		// Texture load task
		enki::TaskSet textureLoadTask = enki::TaskSet(u32(mappings.size()),[&](enki::TaskSetPartition range, uint32_t threadnum) {

			std::vector<enki::ITaskSet*> tasks{};

			for (u32 i = range.start; i < range.end; ++i)
			{
				TextureMappings const& mapping = mappings[i];

				FromFileResourceParameters params{ mapping.path };
				LoadResourceTask<TextureResource>* texture_resource = ResourceLoader::create_load_task<TextureResource>(std::make_shared<TextureResource>(params));
				texture_resource->_complete = [&](shared_ptr<TextureResource> resource) 
				{
					_textures[mapping.tex_idx] = resource;
				};

				tasks.push_back(texture_resource);
			}

			// Kick off tasks
			for (auto const& t : tasks)
			{
				Tasks::get_scheduler()->AddTaskSetToPipe(t);
			}

			// Wait for tasks
			for (auto const& t : tasks)
			{
				Tasks::get_scheduler()->WaitforTaskSet(t);
				delete t;
			}
		});

		Tasks::get_scheduler()->AddTaskSetToPipe(&textureLoadTask);
		Tasks::get_scheduler()->WaitforTask(&textureLoadTask);

		// Material load tas
		_materials.resize(scene->mNumMaterials);
		enki::TaskSet* materialLoadTask = new enki::TaskSet(scene->mNumMaterials, [&](enki::TaskSetPartition range, uint32_t threadNum) 
		{
				std::vector<enki::ITaskSet*> tasks;
				for (u32 i = range.start; i < range.end; ++i)
				{
					aiMaterial* material = scene->mMaterials[i];
					aiString name = material->GetName();

					MaterialInitParameters parameters{};
					parameters.load_type = MaterialInitParameters::LoadType_FromMemory;
					parameters.name = "[Built-in] Material";
					// Get the name of the material
					aiString materialName;
					material->Get(AI_MATKEY_NAME, materialName);
					parameters.name = name.C_Str();

					// Load the required textures from the assimp imported file
					aiString baseColorTexture;
					aiString roughnessTexture;
					aiString normalTexture;
					aiString metalnessTexture;

					if (material->GetTexture(AI_MATKEY_BASE_COLOR_TEXTURE, &baseColorTexture) == aiReturn_SUCCESS)
					{
						parameters.m_texture_paths[MaterialInitParameters::TextureType_Albedo] = dir_path.string() + "\\" + std::string(baseColorTexture.C_Str());
					}
					if (material->GetTexture(AI_MATKEY_ROUGHNESS_TEXTURE, &roughnessTexture) == aiReturn_SUCCESS)
					{
						parameters.m_texture_paths[MaterialInitParameters::TextureType_Roughness] = dir_path.string() + "\\" + std::string(roughnessTexture.C_Str());
					}
					if (material->GetTexture(AI_MATKEY_METALLIC_TEXTURE, &metalnessTexture) == aiReturn_SUCCESS)
					{
						parameters.m_texture_paths[MaterialInitParameters::TextureType_Metalness] = dir_path.string() + "\\" + std::string(metalnessTexture.C_Str());
					}
					if (material->GetTexture(aiTextureType_NORMALS, 0, &normalTexture) == aiReturn_SUCCESS)
					{
						parameters.m_texture_paths[MaterialInitParameters::TextureType_Normal] = dir_path.string() + "\\" + std::string(normalTexture.C_Str());
					}

					bool double_sided;
					if (material->Get(AI_MATKEY_TWOSIDED, double_sided) == aiReturn_SUCCESS)
					{
						parameters.double_sided = double_sided;
					}

					auto res = std::make_shared<MaterialResource>(parameters);

					// Kick off material load
					LoadResourceTask<MaterialResource>* nextLoadTask = ResourceLoader::create_load_task<MaterialResource>(res);
					nextLoadTask->_complete = [i, this](MaterialRef resource) {
						_materials[i] = resource;
					};
					tasks.push_back(nextLoadTask);
				}
				for(auto const& t : tasks)
				{
					Tasks::get_scheduler()->AddTaskSetToPipe(t);
				}

				for(auto const& t : tasks)
				{
					Tasks::get_scheduler()->WaitforTask(t);
					delete t;
				}
				

		});
		Tasks::get_scheduler()->AddTaskSetToPipe(materialLoadTask);
		Tasks::get_scheduler()->WaitforTask(materialLoadTask);
	}

	char name[512];
	sprintf_s(name, "%s - Index Buffer", path.c_str());
	helpers::SetDebugObjectName(_index_buffer.Get(), name);

	sprintf_s(name, "%s - Vertex Buffer", path.c_str());
	helpers::SetDebugObjectName(_vertex_buffer.Get(), name);

	timer.Stop();
	LOG_VERBOSE(IO, "Loading model \"{}\" took {} ", path.c_str(), timer.GetTime());
}
