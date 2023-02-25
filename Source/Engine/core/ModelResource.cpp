#include "engine.pch.h"
#include "ModelResource.h"

#include "MaterialResource.h"
#include "TextureResource.h"
#include "Material.h"

#include "GameEngine.h"

const D3D11_INPUT_ELEMENT_DESC ModelVertex::InputElements[InputElementCount] = {
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

bool ModelHandle::load(enki::ITaskSet* parent)
{
	std::string const& path = get_init_parameters().path;
	_resource = std::make_shared<Model>();
	if(!_resource->Load(parent, path))
    {
        LOG_ERROR(IO, "Failed to load {}", path);
        return false;
	}

	return true;
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

bool Model::Load(enki::ITaskSet* parent, std::string const& path)
{
	Timer timer{}; 
	timer.Start();

	std::filesystem::path dir_path = std::filesystem::path(path);
	dir_path = dir_path.parent_path();

	std::string final_path = IO::get()->ResolvePath(path);

	// #TODO: Loading vertices breaks due to expecting a specific vertex layout. This is an issue when meshes do not have UVs and can't generate tangents in assimp.
	// The mesh loading pipeline needs to be re-written to allow more flexibility and detecting input layouts per model and pick the most appropriate vertex format (no hardcoded types) 

	using namespace Assimp;
	Importer importer = Importer();
    aiScene const* scene = importer.ReadFile(final_path.c_str(), 0);
    scene = importer.ApplyPostProcessing(aiProcess_Triangulate);
    scene = importer.ApplyPostProcessing(aiProcess_GenNormals);
    scene = importer.ApplyPostProcessing(aiProcess_GenUVCoords);
    scene = importer.ApplyPostProcessing(aiProcess_CalcTangentSpace);
    scene = importer.ApplyPostProcessing(aiProcess_ConvertToLeftHanded | aiProcessPreset_TargetRealtime_Fast);
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		LOG_ERROR(IO, importer.GetErrorString());
		return false;
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
            ASSERT(mesh->HasPositions() && mesh->HasNormals());

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

				for(unsigned int k = 0; k < std::min(mesh->GetNumColorChannels(), 4u); ++k)
                {
                    if (mesh->HasVertexColors(i))
                    {
                        v.color[k].x = colors[k][j].r;
                        v.color[k].y = colors[k][j].g;
                        v.color[k].z = colors[k][j].b;
                        v.color[k].w = colors[k][j].a;
                    }
				}

				// Retrieve the UVs
                u32 n_uv = mesh->GetNumUVChannels();
                for (unsigned int k = 0; k < std::min(mesh->GetNumUVChannels(), 4u); ++k)
                {
                    v.uv[k].x = uv[k][j].x;
                    v.uv[k].y = uv[k][j].y;
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

					v.tangent[0].x = tangent.x;
					v.tangent[0].y = tangent.y;
					v.tangent[0].z = tangent.z;

					v.tangent[1].x = bitangent.x;
					v.tangent[1].y = bitangent.y;
					v.tangent[1].z = bitangent.z;
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
		BufferDesc bufferDesc{};
		SubresourceData data{};

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

		// #TODO: Data-drive from the model meta information or somehow pick the right defines for our material based on what the model provides
        if (strstr(path.c_str(), "Box.gltf") || strstr(path.c_str(), "BoxWithoutIndices.gltf"))
        {
            parameters.name = IO::get()->ResolvePath("res:/Engine/untextured.material");
		}

		std::shared_ptr<MaterialHandle> base_material = ResourceLoader::instance()->load<MaterialHandle>(parameters, false, true);

		if(!base_material)
        {
            LOG_ERROR(IO, "Failed to load mesh material");
            return false;
		}

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

			unsigned int diffuseTexCount	= material->GetTextureCount(aiTextureType_DIFFUSE);
            unsigned int normalTexCount		= material->GetTextureCount(aiTextureType_NORMALS);
			unsigned int metalnessTexCount  = material->GetTextureCount(aiTextureType_METALNESS);
			unsigned int roughnessTexCount  = material->GetTextureCount(aiTextureType_DIFFUSE_ROUGHNESS);

			std::vector<TextureHandle> textures;
			for(unsigned int k = 0; k < diffuseTexCount; ++k)
            {
                aiString p;
                material->GetTexture(aiTextureType_DIFFUSE, k, &p);

				std::string texp = dir_path.string() + "\\" + std::string(p.C_Str());
                FromFileResourceParameters params{ texp };
                auto r = ResourceLoader::instance()->load<TextureHandle>(params, true, true);

                u32 slot = m_Materials[j]->get_slot("A");
                m_Materials[j]->set_texture(slot, r);
			}

			if (material->GetTexture(AI_MATKEY_BASE_COLOR_TEXTURE, &baseColorTexture) == aiReturn_SUCCESS)
			{
				std::string const& tex_path = dir_path.string() + "\\" + std::string(baseColorTexture.C_Str());

				FromFileResourceParameters params{ tex_path };
				auto texture = ResourceLoader::instance()->load<TextureHandle>(params, true, true);
				u32 slot = m_Materials[j]->get_slot("Albedo");
				m_Materials[j]->set_texture(slot, texture);

			}
			if (material->GetTexture(AI_MATKEY_METALLIC_TEXTURE, &roughnessTexture) == aiReturn_SUCCESS)
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

	for(uint32_t i = 0;i< m_Meshes.size(); ++i)
    {
		// #TODO: Create input layouts based on the mesh information
		// #TODO: Store input layout information for the shader (for compatibility)
		aiMesh* m = scene->mMeshes[i];

		uint32_t matIdx = m_Meshes[i].material_index;
        MaterialInstance const* material = m_Materials[matIdx].get();
        Graphics::Shader const* s = material->get_vertex_shader().get();

		// Construct the vertex layout flags to track compatibility with material/shaders 
		VertexLayoutFlags flags = (VertexLayoutFlags)0;
        ASSERT(m->HasPositions() && m->HasNormals());
        flags |= VertexLayoutFlags::Position;
        flags |= VertexLayoutFlags::Normal;

		for(unsigned int j = 0; j < m->GetNumUVChannels(); ++j)
        {
            if(m->HasTextureCoords(j))
            {
                flags |= (VertexLayoutFlags::UV0 << j);
			}

            if(m->HasTangentsAndBitangents())
            {
                flags |= (VertexLayoutFlags::Tangent0 << (i *2));
                flags |= (VertexLayoutFlags::Tangent0 << (i*2+1));
			}
		}

		for(unsigned int j = 0; j < m->GetNumColorChannels(); ++j)
        {
			if(m->HasVertexColors(j))
            {
                flags |= (VertexLayoutFlags::Colour0 << j);
			}
		}

		// Build the input layout based on the ModelVertex and exclude anything that our mesh is not providing. This allows us to render meshes with error shaders.
		std::vector<D3D11_INPUT_ELEMENT_DESC> desc;
        desc.reserve(9);

		UINT offset = 0;
		if(any(flags & VertexLayoutFlags::Position))
        {
            desc.push_back({ "SV_Position", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offset, D3D11_INPUT_PER_VERTEX_DATA, 0 });
		}
        offset += sizeof(Shaders::float3);

		if(any(flags & VertexLayoutFlags::Normal))
        {
            desc.push_back({ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offset, D3D11_INPUT_PER_VERTEX_DATA, 0 });
		}
        offset += sizeof(Shaders::float3);

        for (uint32_t t = 0; t < 4; ++t)
        {
            if (any(flags & (VertexLayoutFlags::Tangent0 << t)))
            {
                desc.push_back({ "TANGENT", t, DXGI_FORMAT_R32G32B32_FLOAT, 0, offset, D3D11_INPUT_PER_VERTEX_DATA, 0 });
            }
            offset += sizeof(Shaders::float3);
        }

        for (uint32_t t = 0; t < 4; ++t)
        {
            if (any(flags & (VertexLayoutFlags::Colour0 << t)))
            {
                desc.push_back({ "COLOR", t, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offset, D3D11_INPUT_PER_VERTEX_DATA, 0 });
            }
            offset += sizeof(Shaders::float4);
        }

		for(uint32_t t = 0; t < 4; ++t)
        {
            if(any(flags & (VertexLayoutFlags::UV0 << t)))
            {
                desc.push_back({ "TEXCOORD", t, DXGI_FORMAT_R32G32_FLOAT, 0, offset, D3D11_INPUT_PER_VERTEX_DATA, 0 });
			}
            offset += sizeof(Shaders::float2);
		}

        m_VertexLayouts.push_back(GetRI()->CreateInputLayout(desc, s->GetByteCode(), (uint32_t)s->GetByteCodeLength()));
        m_VertexLayoutFlags.push_back(flags);
    }


	timer.Stop();
	LOG_VERBOSE(IO, "Loading model \"{}\" took {} ", path.c_str(), timer.GetTime());
    return true;
}
