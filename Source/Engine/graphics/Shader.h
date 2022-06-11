#pragma once

#include "ShaderType.h"

namespace Graphics
{


using ShaderRef = std::shared_ptr<class Shader>;
class Shader
{
public:
	Shader() {}
	Shader(ShaderType type, const u8* byte_code, uint32_t size);

	~Shader();

	static std::unique_ptr<Shader> create(ShaderType type, const u8* byte_code, uint32_t size);

	bool is_valid() const {
		return _shader != nullptr;
	}

	template <typename T>
	ComPtr<T> as()
	{
		ComPtr<T> result;
		_shader->QueryInterface(__uuidof(T), (void**)result.GetAddressOf());
		assert(result);
		return result;
	}
	ComPtr<ID3D11Resource> get_shader() const { return _shader; }
	ComPtr<ID3D11InputLayout> get_input_layout() const
	{
		assert(_type == ShaderType::Vertex);
		return _input_layout;
	}


	ShaderType get_type() const { return _type; }

private:
	ShaderType _type;

	ComPtr<ID3D11ShaderReflection> _reflection;
	ComPtr<ID3D11Resource> _shader;
	ComPtr<ID3D11InputLayout> _input_layout;
};


}
