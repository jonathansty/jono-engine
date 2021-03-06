#pragma once

enum class ShaderType
{
	Vertex,
	Pixel,
	Num
};

class Shader
{
public:
	Shader(ShaderType type, const char* byte_code, uint32_t size);

	~Shader();

	static std::unique_ptr<Shader> create(ShaderType type, const char* byte_code, uint32_t size);

	template<typename T>
	ComPtr<T> as()
	{
		ComPtr<T> result;
		_shader->QueryInterface(__uuidof(T), (void**)result.GetAddressOf());
		assert(result);
		return result;
	}
	ComPtr<ID3D11Resource> get_shader() const { return _shader; }
	ComPtr<ID3D11InputLayout> get_input_layout() const { assert(_type == ShaderType::Vertex); return _input_layout; }
private:
	ShaderType _type;

	ComPtr<ID3D11ShaderReflection> _reflection;
	ComPtr<ID3D11Resource> _shader;
	ComPtr<ID3D11InputLayout> _input_layout;
};

// Default material
interface IMaterial
{
	virtual void apply() = 0;
};

class Material final : public IMaterial
{
public:
	Material() {};
	virtual ~Material() {}

	static std::unique_ptr<Material> create(const char* vx_byte_code , uint32_t vx_byte_code_size, const char* px_byte_code, uint32_t px_byte_code_size );

	void apply();

private:
	//TODO: Use pipelines?
	std::unique_ptr<Shader> _vertex_shader;

	std::unique_ptr<Shader> _pixel_shader;
	std::unique_ptr<Shader> _debug_pixel_shader;

	std::vector<std::shared_ptr<class TextureResource>> _textures;

	friend class MaterialResource;
};