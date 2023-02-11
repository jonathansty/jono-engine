#pragma once
#include "ResourceLoader.h"

#include "Graphics/GraphicsResourceHandle.h"
#include "Graphics/ShaderTypes.h"
#include "Math.h"

class TextureHandle;

using MaterialRef = std::shared_ptr<class MaterialHandle>;
class MaterialInstance;

struct ModelUberVertex
{
	Shaders::float3 position;
	Shaders::float3 normal;
	Shaders::float4 tangent;
	Shaders::float4 bitangent;
	Shaders::float4 color;

	// Uvs
	Shaders::float2 uv0;
	Shaders::float2 uv1;
	Shaders::float2 uv2;
	Shaders::float2 uv3;

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

class Model
{
public:

	using VertexType = ModelUberVertex;

	Model();

	~Model();

	void Load(enki::ITaskSet* parent, std::string const& path);

	GraphicsResourceHandle GetVertexBuffer() const { return m_VertexBuffer; }
	GraphicsResourceHandle GetIndexBuffer() const { return m_IndexBuffer; }

	std::vector<Mesh> const& GetMeshes() const { return m_Meshes; }

	inline MaterialInstance* GetMaterial(u32 idx) const 
	{ 
		ASSERTMSG(idx < static_cast<u32>(m_Materials.size()), "Index out of range.");
		return m_Materials[idx].get(); 
	}

	u32 get_material_count() const { return (u32)m_Materials.size(); }

	Math::AABB get_bounding_box() const { return m_AABB; }

private:
	u64 _index_count;

	std::vector<std::unique_ptr<MaterialInstance>> m_Materials;
	std::vector<Mesh> m_Meshes;
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

	void load(enki::ITaskSet* parent) override;
};