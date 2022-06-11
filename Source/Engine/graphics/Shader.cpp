#include "engine.pch.h"

#include "Graphics.h"
#include "Shader.h"
#include "Core/ModelResource.h"

namespace Graphics
{

	
Shader::Shader(ShaderType type, const u8* byte_code, uint32_t size)
		: _type(type)
{
	ComPtr<ID3D11Device> device = Graphics::get_device();
	switch (type)
	{
		case ShaderType::Vertex:
		{
			SUCCEEDED(device->CreateVertexShader(byte_code, size, nullptr, (ID3D11VertexShader**)_shader.GetAddressOf()));
			using VertexType = ModelUberVertex;
			SUCCEEDED(device->CreateInputLayout(VertexType::InputElements, VertexType::InputElementCount, byte_code, size, _input_layout.GetAddressOf()));
		}
		break;
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

std::unique_ptr<Shader> Shader::create(ShaderType type, const u8* byte_code, uint32_t size)
{
	return std::make_unique<Shader>(type, byte_code, size);
}

}