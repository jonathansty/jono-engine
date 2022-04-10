#pragma once



// Default material
interface IMaterial
{
	virtual void apply() = 0;
};

namespace Graphics
{
class Shader;
}

class Material final : public IMaterial
{
public:
	Material() {};
	virtual ~Material() {}

	static std::unique_ptr<Material> create(std::shared_ptr<Graphics::Shader> vertex_shader, std::shared_ptr<Graphics::Shader> pixel_shader);

	void apply();

	bool is_double_sided() const { return _double_sided; }

private:
	bool _double_sided;
	//TODO: Use pipelines?
	std::shared_ptr<Graphics::Shader> _vertex_shader;

	std::shared_ptr<Graphics::Shader> _pixel_shader;
	std::shared_ptr<Graphics::Shader> _debug_pixel_shader;

	std::vector<std::shared_ptr<class TextureResource>> _textures;

	friend class MaterialResource;
};