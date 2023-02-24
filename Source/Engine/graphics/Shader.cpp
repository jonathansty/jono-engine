#include "engine.pch.h"

#include "Graphics.h"
#include "Shader.h"
#include "Core/ModelResource.h"

#include "Graphics/RenderInterface.h"
#include "Graphics/VertexLayout.h"

namespace Graphics
{

	
Shader::Shader(ShaderStage type, const u8* byte_code, uint32_t size, const char* debug_name)
		: m_Type(type)
	, m_ByteCode(nullptr)
{
	// #TODO: Replace with RI
	// Think about dx12 and pipeline based APIs, these require shaders to be specified as part of the PSO
    ComPtr<ID3D11Device> ri = GetRI()->Dx11GetDevice();
	switch (type)
	{
		case ShaderStage::Vertex:
			ENSURE_HR(ri->CreateVertexShader(byte_code, size, nullptr, (ID3D11VertexShader**)m_Shader.GetAddressOf()));
			break;
		case ShaderStage::Pixel:
			ENSURE_HR(ri->CreatePixelShader(byte_code, size, nullptr, (ID3D11PixelShader**)m_Shader.GetAddressOf()));
			break;
		case ShaderStage::Compute:
			ENSURE_HR(ri->CreateComputeShader(byte_code, size, nullptr, (ID3D11ComputeShader**)m_Shader.GetAddressOf()));
			break;
		default:
			throw new std::exception("ShaderType not supported!");
	}

	if(m_Shader)
	{
		Helpers::SetDebugObjectName(m_Shader.Get(), debug_name ? debug_name : (__FILE__));
	}

	D3DReflect(byte_code, size, IID_ID3D11ShaderReflection, &m_Reflection);

	if((type & ShaderStage::Vertex) == ShaderStage::Vertex)
	{
		D3D11_SHADER_DESC desc{};
		m_Reflection->GetDesc(&desc);

		UINT params = desc.InputParameters;

		VertexLayoutFlags flags = VertexLayoutFlags(0);

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

			// Determine the right vertex layout flags for this shader
			if(strstr(paramDesc.SemanticName, "SV_Position"))
            {
                flags |= VertexLayoutFlags::Position;
			}
            else if (strstr(paramDesc.SemanticName, "NORMAL"))
            {
                flags |= VertexLayoutFlags::Normal;
			}
            else if (strstr(paramDesc.SemanticName, "TANGENT"))
            {
                flags |= VertexLayoutFlags::Tangent0 << paramDesc.SemanticIndex;
            
			}
            else if (strstr(paramDesc.SemanticName, "COLOR"))
            {
                flags |= VertexLayoutFlags::Colour0 << paramDesc.SemanticIndex;
            
			}
            else if (strstr(paramDesc.SemanticName, "UV"))
            {
                flags |= VertexLayoutFlags::UV0 << paramDesc.SemanticIndex;
			}

			// Determine the required format 
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
            else
            {
                LOG_WARNING(Graphics, "Shader provides semantics that are not properly supported.");
			}
		
		}
		m_InputLayout = GetRI()->CreateInputLayout(inputs, (void*)byte_code, size);
        m_Flags = flags;
	}
    m_ByteCode = malloc(size);
    memcpy(m_ByteCode, byte_code, size);
    m_ByteCodeLength = size;
}

Shader::~Shader()
{
    GetRI()->ReleaseResource(m_InputLayout);

	if(m_ByteCode)
    {
        free(m_ByteCode);
        m_ByteCode = nullptr;
    }
}

std::unique_ptr<Shader> Shader::Create(ShaderStage type, const u8* byte_code, uint32_t size, const char* debug_name)
{
	return std::make_unique<Shader>(type, byte_code, size, debug_name);
}

}