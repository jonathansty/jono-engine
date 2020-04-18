#include "stdafx.h"

#include "MaterialResource.h"
#include "ModelResource.h"
#include "TextureResource.h"

#include "Material.h"

namespace Shaders
{
#include "Shaders/generated/simple_px.h"
#include "Shaders/generated/simple_vx.h"
#include "Shaders/generated/debug_px.h"
}

void MaterialResource::load()
{
	auto device = game_engine::instance()->GetD3DDevice();


	using VertexType = ModelVertex;

	_resource = Material::create((const char*)Shaders::cso_simple_vx, std::size(Shaders::cso_simple_vx), (const char*)Shaders::cso_simple_px, std::size(Shaders::cso_simple_px));
	_resource->_debug_pixel_shader = Shader::create(ShaderType::Pixel, (const char*)Shaders::cso_debug_px, std::size(Shaders::cso_debug_px));

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
