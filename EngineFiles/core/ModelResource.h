#pragma once
#include "ResourceLoader.h"

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
class ModelResource final : public TCachedResource<FromFileResourceParameters>
{
public:
	using VertexType = ModelVertex;

	ModelResource(FromFileResourceParameters params) 
		: TCachedResource( params )
		, _index_count(0)
	{
	}

	~ModelResource() {}
	void load() override;

	// Gpu resources
	size_t _index_count;

	// Shaders
	ComPtr<ID3D11InputLayout> _input_layout;
	ComPtr<ID3D11Buffer> _vert_buffer;
	ComPtr<ID3D11Buffer> _index_buffer;
	ComPtr<ID3D11VertexShader> _vert_shader;
	ComPtr<ID3D11PixelShader> _pixel_shader;


};