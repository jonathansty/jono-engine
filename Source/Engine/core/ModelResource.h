#pragma once
#include "ResourceLoader.h"


using MaterialRef = std::shared_ptr<class MaterialResource>;

struct ModelVertex
{
	XMFLOAT3 position;
	XMFLOAT3 normal;
	XMFLOAT4 tangent;
	XMFLOAT4 bitangent;
	XMFLOAT4 color;
	XMFLOAT2 uv;

	static const std::size_t InputElementCount = 6;
	static const D3D11_INPUT_ELEMENT_DESC InputElements[InputElementCount];
};

// Sub-mesh
struct Mesh
{
	uint64_t firstVertex;
	uint64_t firstIndex;
	uint64_t indexCount;
	std::size_t materialID;
};



class Model
{};

// Render Model resource
class ModelResource final : public TCachedResource<Model, FromFileResourceParameters>
{
public:
	using VertexType = ModelVertex;

	ModelResource(FromFileResourceParameters params);

	~ModelResource() {}
	void load() override;

	// Gpu resources
	uint64_t _index_count;

	std::vector<MaterialRef> _materials;
	std::vector<Mesh> _meshes;

	// Vert buffer and index buffer contain all the data for each meshlet.
	ComPtr<ID3D11Buffer> _vert_buffer;
	ComPtr<ID3D11Buffer> _index_buffer;
};