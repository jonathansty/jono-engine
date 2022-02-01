#include "engine.pch.h"
#include "Material.h"

#include "GameEngine.h"
#include "ModelResource.h"
#include "TextureResource.h"

Shader::Shader(ShaderType type, const char* byte_code, uint32_t size)
	: _type(type)
{
	ID3D11Device* device = GameEngine::instance()->GetD3DDevice();
	switch (type)
	{
	case ShaderType::Vertex:
	{
		SUCCEEDED(device->CreateVertexShader(byte_code, size, nullptr, (ID3D11VertexShader**)_shader.GetAddressOf()));
		using VertexType = ModelVertex;
		SUCCEEDED(device->CreateInputLayout(VertexType::InputElements, VertexType::InputElementCount, byte_code, size, _input_layout.GetAddressOf()));
	}break;
	case ShaderType::Pixel:
		SUCCEEDED(device->CreatePixelShader(byte_code, size, nullptr, (ID3D11PixelShader**)_shader.GetAddressOf()));
		break;
	default:
		throw new std::exception("ShaderType not supported!");
	}

	D3DReflect(byte_code, size, IID_ID3D11ShaderReflection, &_reflection);
	D3D11_SHADER_INPUT_BIND_DESC bindDesc;
	_reflection->GetResourceBindingDescByName("g_Albedo", &bindDesc);

}

Shader::~Shader()
{
}

std::unique_ptr<Shader> Shader::create(ShaderType type, const char* byte_code, uint32_t size)
{
	return std::make_unique<Shader>(type, byte_code, size);
}

std::unique_ptr<Material> Material::create(const char* vx_byte_code, uint32_t vx_byte_code_size, const char* px_byte_code, uint32_t px_byte_code_size)
{
	auto obj = std::make_unique<Material>();
	obj->_vertex_shader = Shader::create(ShaderType::Vertex, vx_byte_code, vx_byte_code_size);
	obj->_pixel_shader = Shader::create(ShaderType::Pixel,  px_byte_code, px_byte_code_size);
	return obj;
}

extern int g_DebugMode;

void Material::apply()
{
	auto ctx = GameEngine::instance()->GetD3DDeviceContext();
	ctx->VSSetShader(_vertex_shader->as<ID3D11VertexShader>().Get(), nullptr, 0);
	ctx->IASetInputLayout(_vertex_shader->get_input_layout().Get());

	ctx->PSSetShader(_pixel_shader->as<ID3D11PixelShader>().Get(), nullptr, 0);

	if (g_DebugMode)
	{
		ctx->PSSetShader(_debug_pixel_shader->as<ID3D11PixelShader>().Get(), nullptr, 0);
	}

	// Bind material parameters
	std::array<ID3D11ShaderResourceView const*, 3> views{};
	for (std::size_t i = 0; i < _textures.size(); ++i)
	{
		if (_textures[i])
		{
			views[i] = _textures[i]->get_srv();
		}
	}
	ctx->PSSetShaderResources(0, (UINT)views.size(), (ID3D11ShaderResourceView**)views.data());
}
