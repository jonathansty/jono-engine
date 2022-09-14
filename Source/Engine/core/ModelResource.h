#pragma once
#include "ResourceLoader.h"

#include "Graphics/ShaderTypes.h"

class TextureResource;

using MaterialRef = std::shared_ptr<class MaterialResource>;
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

	ComPtr<ID3D11Buffer> get_vertex_buffer() const { return _vertex_buffer; }
	ComPtr<ID3D11Buffer> get_index_buffer() const { return _index_buffer; }

	std::vector<Mesh> const& get_meshes() const { return _meshes; }

	MaterialInstance* get_material(u32 idx) const 
	{ 
		ASSERTMSG(idx < static_cast<u32>(_materials.size()), "Index out of range.");
		return _materials[idx].get(); 
	}

	u32 get_material_count() const { return (u32)_materials.size(); }

private:
	u64 _index_count;

	std::mutex _textures_cs;
	std::vector<shared_ptr<TextureResource>> _textures;

	std::mutex _material_cs;
	std::vector<std::unique_ptr<MaterialInstance>> _materials;

	std::mutex _meshes_cs;
	std::vector<Mesh> _meshes;

	ComPtr<ID3D11Buffer> _vertex_buffer;
	ComPtr<ID3D11Buffer> _index_buffer;
};

// Render Model resource
class ModelResource final : public TCachedResource<Model, FromFileResourceParameters>
{
public:

	ModelResource(FromFileResourceParameters params);

	~ModelResource();

	void build_load_graph(enki::ITaskSet* parent) override;

	void load(enki::ITaskSet* parent) override;
};