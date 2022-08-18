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

		}
		break;
		case ShaderType::Pixel:
			SUCCEEDED(device->CreatePixelShader(byte_code, size, nullptr, (ID3D11PixelShader**)_shader.GetAddressOf()));
			break;
		default:
			throw new std::exception("ShaderType not supported!");
	}

	D3DReflect(byte_code, size, IID_ID3D11ShaderReflection, &_reflection);

	if(type == ShaderType::Vertex)
	{
		D3D11_SHADER_DESC desc{};
		_reflection->GetDesc(&desc);

		UINT params = desc.InputParameters;

		std::vector<D3D11_INPUT_ELEMENT_DESC> inputs{};
		inputs.resize(params);
		for(u32 i = 0; i < params; ++i)
		{
			D3D11_SIGNATURE_PARAMETER_DESC paramDesc{};
			_reflection->GetInputParameterDesc(i, &paramDesc);
			
			inputs[i].SemanticName = paramDesc.SemanticName;
			inputs[i].SemanticIndex = paramDesc.SemanticIndex;
			inputs[i].InputSlot = D3D11_INPUT_PER_VERTEX_DATA;
			inputs[i].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
			inputs[i].InstanceDataStepRate = 0;

			if(paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32)
			{
				if(paramDesc.Mask == 0b1111)
				{
					inputs[i].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
				}
				if(paramDesc.Mask == 0b111)
				{
					inputs[i].Format = DXGI_FORMAT_R32G32B32_FLOAT;
				}

				if (paramDesc.Mask == 0b0011)
				{
					inputs[i].Format = DXGI_FORMAT_R32G32_FLOAT;
				
				}
			}
			else if(paramDesc.ComponentType == D3D10_REGISTER_COMPONENT_UINT32)
			{
				if(paramDesc.Mask == 0b1)
				{
					inputs[i].Format = DXGI_FORMAT_R32_UINT;
				}
			
			}
		
		}
	
		SUCCEEDED(device->CreateInputLayout(inputs.data(), (UINT)inputs.size(), byte_code, size, _input_layout.GetAddressOf()));
	}

	//D3D11_SHADER_INPUT_BIND_DESC bindDesc;
	//_reflection->GetResourceBindingDescByName("g_Albedo", &bindDesc);
}

Shader::~Shader()
{
}

std::unique_ptr<Shader> Shader::create(ShaderType type, const u8* byte_code, uint32_t size)
{
	return std::make_unique<Shader>(type, byte_code, size);
}

}