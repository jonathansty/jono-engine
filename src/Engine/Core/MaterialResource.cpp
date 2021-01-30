#include "engine.stdafx.h"

#include "GameEngine.h"
#include "MaterialResource.h"
#include "ModelResource.h"
#include "TextureResource.h"

#include "Material.h"

namespace Shaders
{
#include "shaders/simple_px.h"
#include "shaders/simple_vx.h"
#include "shaders/debug_px.h"
}

void MaterialResource::load()
{
	auto device = GameEngine::instance()->GetD3DDevice();


	using VertexType = ModelVertex;

	_resource = Material::create((const char*)Shaders::cso_simple_vx, uint32_t(std::size(Shaders::cso_simple_vx)), (const char*)Shaders::cso_simple_px, uint32_t(std::size(Shaders::cso_simple_px)));
	_resource->_debug_pixel_shader = Shader::create(ShaderType::Pixel, (const char*)Shaders::cso_debug_px, uint32_t(std::size(Shaders::cso_debug_px)));

	// Initiate texture loads
	for (std::string const& path : get_init_parameters().m_texture_paths)
	{
		if (path.empty())
		{
			_resource->_textures.push_back(TextureResource::invalid());
		}
		else
		{
			_resource->_textures.push_back(ResourceLoader::instance()->load<TextureResource>({ path }, true));
		}
	}
}
