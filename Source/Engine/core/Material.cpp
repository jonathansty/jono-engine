#include "engine.pch.h"
#include "Material.h"
#include "Graphics/Shader.h"
#include "GameEngine.h"
#include "ModelResource.h"
#include "TextureResource.h"

 Material::Material()
		: _double_sided(false)
		, _vertex_shader()
		, _pixel_shader()
		, _debug_pixel_shader()
		, _textures()
{
}

std::unique_ptr<Material> Material::create(Graphics::ShaderRef const& vertex_shader, Graphics::ShaderRef const& pixel_shader)
{
	auto obj = std::make_unique<Material>();
	obj->_vertex_shader = vertex_shader;
	obj->_pixel_shader = pixel_shader;
	return obj;
}

extern int g_DebugMode;

ComPtr<ID3D11InputLayout> Material::get_input_layout() const
{
	return _vertex_shader->get_input_layout();
}

void Material::get_texture_views(std::vector<ID3D11ShaderResourceView const*>& views)
{
	for (std::size_t i = 0; i < _textures.size(); ++i)
	{
		if (_textures[i])
		{
			Texture const* texture = _textures[i]->get();
			views.push_back(texture->get_srv());
		}
		else
		{
			// ERROR
			views.push_back(TextureResource::black()->get()->get_srv());
		}
	}
}
