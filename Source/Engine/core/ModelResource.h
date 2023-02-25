#pragma once
#include "ResourceLoader.h"

#include "Graphics/GraphicsResourceHandle.h"
#include "Graphics/ShaderTypes.h"
#include "Graphics/VertexLayout.h"
#include "Math.h"

class TextureHandle;

using MaterialRef = std::shared_ptr<class MaterialHandle>;
class MaterialInstance;

// This needs to match VS_IN for the shaders we are trying to use
struct ModelVertex
{
	Shaders::float3 position;
	Shaders::float3 normal;

	// Tangents
    Shaders::float3 tangent[4];

	// Colours
	Shaders::float4 color[4];

	// Uvs
	Shaders::float2 uv[4];


	static const std::size_t InputElementCount = 9;
	static const D3D11_INPUT_ELEMENT_DESC InputElements[InputElementCount];
};

struct Mesh
{
	// First vertex offset this mesh starts at in the vertex buffer
	u64 firstVertex;

	// First index location offset this mesh starts at in the index buffer
	u64 firstIndex;

	// The amount of indices associated with this mesh
	u64 indexCount;

	// Index of the material in the model owned material array
	u32 material_index;
};

struct InputLayout
{
    VertexLayoutFlags      m_Usages;
	GraphicsResourceHandle m_Resource;
};

class Model
{
public:

	using VertexType = ModelVertex;

	Model();

	~Model();

	bool Load(enki::ITaskSet* parent, std::string const& path);

	GraphicsResourceHandle GetVertexBuffer() const { return m_VertexBuffer; }
	GraphicsResourceHandle GetIndexBuffer() const { return m_IndexBuffer; }

	std::vector<Mesh> const& GetMeshes() const { return m_Meshes; }

	inline MaterialInstance* GetMaterial(u32 idx) const 
	{ 
		ASSERTMSG(idx < static_cast<u32>(m_Materials.size()), "Index out of range.");
		return m_Materials[idx].get(); 
	}

    inline VertexLayoutFlags GetElementUsages(u32 idx) const
    {
        ASSERTMSG(idx < static_cast<u32>(m_VertexLayoutFlags.size()), "Index out of range.");
        return m_VertexLayoutFlags[idx];
    }
	inline GraphicsResourceHandle GetVertexLayout(u32 idx) const
    {
        return m_VertexLayouts[idx];
	}


	u32 get_material_count() const { return (u32)m_Materials.size(); }

	Math::AABB get_bounding_box() const { return m_AABB; }

private:
	u64 _index_count;

	std::vector<std::unique_ptr<MaterialInstance>> m_Materials;
	std::vector<Mesh> m_Meshes;
    std::vector<VertexLayoutFlags> m_VertexLayoutFlags;
    std::vector<GraphicsResourceHandle> m_VertexLayouts;

	Math::AABB m_AABB;
	GraphicsResourceHandle m_VertexBuffer;
	GraphicsResourceHandle m_IndexBuffer;
};

// Render Model resource
class ModelHandle final : public TCachedResource<Model, FromFileResourceParameters>
{
public:

	ModelHandle(FromFileResourceParameters params);

	~ModelHandle();

	bool load(enki::ITaskSet* parent) override;
};