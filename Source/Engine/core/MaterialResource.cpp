#include "engine.pch.h"

#include "GameEngine.h"
#include "MaterialResource.h"
#include "ModelResource.h"
#include "TextureResource.h"

#include "Graphics/ShaderCache.h"
#include "Material.h"

#include "Graphics/ShaderCompiler.h"

void MaterialResource::load(enki::ITaskSet* parent)
{
	using namespace Graphics;


	ShaderCompiler::CompileParameters params{};
	params.entry_point = "main";
	params.defines.push_back({ "LIGHTING_MODEL", "LIGHTING_MODEL_BLINN_PHONG" });
	params.flags = ShaderCompiler::CompilerFlags::CompileDebug;
	params.stage = ShaderType::Pixel;

	std::vector<u8> pixel_bytecode;
	std::vector<u8> vertex_bytecode;
	std::vector<u8> debug_bytecode;

	// Get pixel shader
	ShaderCreateParams create_params{};
	create_params.params = params;
	create_params.path = "Source/Engine/Shaders/simple_px.hlsl";
	auto pixel_shader = ShaderCache::instance()->find_or_create(create_params);

	// Get debug vertex shader
	create_params.params = params;
	create_params.path = "Source/Engine/Shaders/debug_px.hlsl";
	auto debug_shader = ShaderCache::instance()->find_or_create(create_params);

	create_params.params.stage = ShaderType::Vertex;
	create_params.path = "Source/Engine/Shaders/simple_vx.hlsl";
	auto vertex_shader = ShaderCache::instance()->find_or_create(create_params);

	if (!pixel_shader)
	{
		pixel_shader = Graphics::get_error_shader_px();
		LOG_ERROR(Graphics, "Failed to find base pixel shader. Using error shader.");
	}

	if (!vertex_shader)
	{
		vertex_shader = Graphics::get_error_shader_vx();
		LOG_ERROR(Graphics, "Failed to find base vertex shader. Using error shader.");
	}


	if (!debug_shader)
	{
		debug_shader = Graphics::get_error_shader_px();
		LOG_ERROR(Graphics, "Failed to find base debug shader. Using error shader.");
	}

	_resource = Material::create(vertex_shader, pixel_shader);

	_resource->_debug_pixel_shader = debug_shader;
	_resource->_double_sided = get_init_parameters().double_sided;

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
			shared_ptr<TextureResource> res = ResourceLoader::instance()->load<TextureResource>({ paths[textureType] },true);
			_resource->_textures.push_back(res);
		}
	}
}

std::string MaterialInitParameters::to_string() const {
	return name;
}
