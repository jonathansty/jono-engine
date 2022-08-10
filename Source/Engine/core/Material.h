#pragma once

namespace Graphics
{
	using ShaderRef = std::shared_ptr<class Shader>;
}

namespace Graphics
{
class Shader;
}

class Material final 
{
public:
	Material();

	virtual ~Material() {}

	static std::unique_ptr<Material> create(Graphics::ShaderRef const& vertex_shader, Graphics::ShaderRef const& pixel_shader);

	bool is_double_sided() const { return _double_sided; }

	ComPtr<ID3D11InputLayout> get_input_layout() const;

	Graphics::ShaderRef const& get_vertex_shader() const { return _vertex_shader; }
	Graphics::ShaderRef const& get_pixel_shader() const  { return _pixel_shader; }
	Graphics::ShaderRef const& get_debug_pixel_shader() const  { return _debug_pixel_shader; }

	void get_texture_views(std::vector<ID3D11ShaderResourceView const*>& views);

private:
	bool _double_sided;

	Graphics::ShaderRef _vertex_shader;
	Graphics::ShaderRef _pixel_shader;
	Graphics::ShaderRef _debug_pixel_shader;

	std::vector<std::shared_ptr<class TextureResource>> _textures;

	friend class MaterialResource;
};