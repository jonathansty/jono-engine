#include "engine.pch.h"

#include "Graphics.h"
#include "Shader.h"
#include "Core/ModelResource.h"

#include "Graphics/RenderInterface.h"

namespace Graphics
{

	
Shader::Shader(ShaderType type, const u8* byte_code, uint32_t size, const char* debug_name)
		: m_Type(type)
{
	ComPtr<ID3D11Device> device = Graphics::get_device();
	switch (type)
	{
		case ShaderType::Vertex:
			ENSURE_HR(device->CreateVertexShader(byte_code, size, nullptr, (ID3D11VertexShader**)m_Shader.GetAddressOf()));
			break;
		case ShaderType::Pixel:
			ENSURE_HR(device->CreatePixelShader(byte_code, size, nullptr, (ID3D11PixelShader**)m_Shader.GetAddressOf()));
			break;
		case ShaderType::Compute:
			ENSURE_HR(device->CreateComputeShader(byte_code, size, nullptr, (ID3D11ComputeShader**)m_Shader.GetAddressOf()));
			break;
		default:
			throw new std::exception("ShaderType not supported!");
	}

	if(m_Shader)
	{
		Helpers::SetDebugObjectName(m_Shader.Get(), debug_name ? debug_name : (__FILE__));
	}

	D3DReflect(byte_code, size, IID_ID3D11ShaderReflection, &m_Reflection);

	if(type == ShaderType::Vertex)
	{
		D3D11_SHADER_DESC desc{};
		m_Reflection->GetDesc(&desc);

		UINT params = desc.InputParameters;

		std::vector<D3D11_INPUT_ELEMENT_DESC> inputs{};
		inputs.resize(params);
		for(u32 i = 0; i < params; ++i)
		{
			D3D11_SIGNATURE_PARAMETER_DESC paramDesc{};
			m_Reflection->GetInputParameterDesc(i, &paramDesc);
			
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
		m_InputLayout = GetRI()->CreateInputLayout(inputs, (void*)byte_code, size);
	}

	//D3D11_SHADER_INPUT_BIND_DESC bindDesc;
	//_reflection->GetResourceBindingDescByName("g_Albedo", &bindDesc);
}

Shader::~Shader()
{
}

std::unique_ptr<Shader> Shader::Create(ShaderType type, const u8* byte_code, uint32_t size, const char* debug_name)
{
	return std::make_unique<Shader>(type, byte_code, size, debug_name);
}

}