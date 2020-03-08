#pragma once
#include "ResourceLoader.h"

class ModelResource final : public TCachedResource<FromFileResourceParameters>
{
public:

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