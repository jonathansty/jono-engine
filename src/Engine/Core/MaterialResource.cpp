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

	_resource = Material::create((const char*)Shaders::cso_simple_vx, uint32_t(std::size(Shaders::cso_simple_vx)), (const char*)Shaders::cso_simple_px, uint32_t(std::size(Shaders::cso_simple_px)));
	_resource->_debug_pixel_shader = Shader::create(ShaderType::Pixel, (const char*)Shaders::cso_debug_px, uint32_t(std::size(Shaders::cso_debug_px)));

	// Initiate texture loads
	auto paths = get_init_parameters().m_texture_paths;
	for(u32 textureType = 0; textureType < MaterialInitParameters::TextureType_Count; ++textureType)
	{
		if (paths[textureType].empty())
		{
			switch (textureType) {
				case MaterialInitParameters::TextureType_Albedo:
					_resource->_textures.push_back(TextureResource::white());
					break;
				case MaterialInitParameters::TextureType_MetalnessRoughness:
					_resource->_textures.push_back(TextureResource::default_roughness());
					break;
				case MaterialInitParameters::TextureType_Normal:
					_resource->_textures.push_back(TextureResource::default_normal());
					break;
				case MaterialInitParameters::TextureType_Count:
					_resource->_textures.push_back(TextureResource::invalid());
					break;
			}
		}
		else
		{
			_resource->_textures.push_back(ResourceLoader::instance()->load<TextureResource>({ paths[textureType] }, true));
		}
	}
}
