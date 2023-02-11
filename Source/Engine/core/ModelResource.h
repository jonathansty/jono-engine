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

	void load(enki::ITaskSet* parent, std::string const& path);

	GraphicsResourceHandle get_vertex_buffer() const { return _vertex_buffer; }
	GraphicsResourceHandle get_index_buffer() const { return _index_buffer; }

	std::vector<Mesh> const& get_meshes() const { return _meshes; }

	MaterialInstance* get_material(u32 idx) const 
	{ 
		ASSERTMSG(idx < static_cast<u32>(_materials.size()), "Index out of range.");
		return _materials[idx].get(); 
	}

	u32 get_material_count() const { return (u32)_materials.size(); }

	Math::AABB get_bounding_box() const { return _aabb; }

private:
	u64 _index_count;

	std::vector<std::unique_ptr<MaterialInstance>> _materials;
	std::vector<Mesh> _meshes;
	Math::AABB _aabb;

	GraphicsResourceHandle _vertex_buffer;
	GraphicsResourceHandle _index_buffer;
};

// Render Model resource
class ModelHandle final : public TCachedResource<Model, FromFileResourceParameters>
{
public:

	ModelHandle(FromFileResourceParameters params);

	~ModelHandle();

	void load(enki::ITaskSet* parent) override;
};