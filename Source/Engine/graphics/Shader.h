#pragma once

#include "ShaderType.h"
#include "GraphicsResourceHandle.h"

namespace Graphics
{

using ShaderRef = std::shared_ptr<class Shader>;
using ShaderConstRef = std::shared_ptr<class Shader const>;
class Shader
{
public:
	Shader() {}
	Shader(ShaderType type, const u8* byte_code, uint32_t size, const char* debug_name = nullptr);

	~Shader();

	static std::unique_ptr<Shader> Create(ShaderType type, const u8* byte_code, uint32_t size, const char* debug_name = nullptr);

	bool IsValid() const
	{
		return m_Shader != nullptr;
	}

	template <typename T>
	ComPtr<T> as() const
	{
		ComPtr<T> result;
		m_Shader->QueryInterface(__uuidof(T), (void**)result.GetAddressOf());
		assert(result);
		return result;
	}

	ComPtr<ID3D11Resource> GetShader() const { return m_Shader; }

	GraphicsResourceHandle GetInputLayout() const
	{
		assert(m_Type == ShaderType::Vertex);
		return m_InputLayout;
	}

	ShaderType get_type() const { return m_Type; }

private:
	ShaderType m_Type;

	ComPtr<ID3D11ShaderReflection> m_Reflection;
	ComPtr<ID3D11Resource> m_Shader;
	GraphicsResourceHandle m_InputLayout;
};

} // namespace Graphics
