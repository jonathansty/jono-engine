#pragma once
#include "ResourceLoader.h"

class ModelResource final : public Resource
{
public:

	ModelResource() {}
	~ModelResource() {}
	void load(std::string const& path) override;

	// Gpu resources
	size_t _index_count;

	// Shaders
	ComPtr<ID3D11InputLayout> _input_layout;
	ComPtr<ID3D11Buffer> _vert_buffer;
	ComPtr<ID3D11Buffer> _index_buffer;
	ComPtr<ID3D11VertexShader> _vert_shader;
	ComPtr<ID3D11PixelShader> _pixel_shader;


};