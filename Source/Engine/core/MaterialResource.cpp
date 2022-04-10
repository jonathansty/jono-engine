#include "engine.pch.h"

#include "GameEngine.h"
#include "MaterialResource.h"
#include "ModelResource.h"
#include "TextureResource.h"

#include "Material.h"

#include "Graphics/ShaderCompiler.h"

void MaterialResource::load()
{
	using namespace Graphics;

	auto device = Graphics::get_device();

	ShaderCompiler::CompileParameters params{};
	params.entry_point = "main";
	params.flags = ShaderCompiler::CompilerFlags::CompileDebug;
	params.stage = ShaderType::Pixel;

	std::vector<u8> pixel_bytecode;
	std::vector<u8> vertex_bytecode;
	std::vector<u8> debug_bytecode;
	if(!ShaderCompiler::compile("Source/Engine/Shaders/simple_px.hlsl", params, pixel_bytecode)) {
		// Use error shader
		if(!ShaderCompiler::compile("Source/Engine/Shaders/error_px.hlsl", params, pixel_bytecode)) {
			LOG_FATAL(Graphics, "Couldn't compile basic error shader!");
		}
	}

	if (!ShaderCompiler::compile("Source/Engine/Shaders/debug_px.hlsl", params, debug_bytecode))
	{
		if (!ShaderCompiler::compile("Source/Engine/Shaders/error_px.hlsl", params, pixel_bytecode))
		{
			LOG_FATAL(Graphics, "Couldn't compile basic error shader!");
		}
	}


	params.stage = Graphics::ShaderType::Vertex;
	if (!ShaderCompiler::compile("Source/Engine/Shaders/simple_vx.hlsl", params, vertex_bytecode))
	{
		if (!ShaderCompiler::compile("Source/Engine/Shaders/error_vx.hlsl", params, pixel_bytecode))
		{
			LOG_FATAL(Graphics, "Couldn't compile basic error shader!");
		}
	}

	ShaderCreateParams request{};
	request.params = params;
	request.params.stage = Graphics::ShaderType::Vertex;
	request.path = "Source/Engine/Shaders/simple_vx.hlsl";
	auto vertex_shader = ShaderCache::instance()->find_or_create(request);

	request.params.stage = Graphics::ShaderType::Pixel;
	request.path = "Source/Engine/Shaders/simple_px.hlsl";
	auto pixel_shader = ShaderCache::instance()->find_or_create(request);
	_resource = Material::create(vertex_shader, pixel_shader);

	request.path = "Source/Engine/Shaders/debug_px.hlsl";
	_resource->_debug_pixel_shader = ShaderCache::instance()->find_or_create(request);
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
			_resource->_textures.push_back(ResourceLoader::instance()->load<TextureResource>({ paths[textureType] }, true));
		}
	}
}

std::string MaterialInitParameters::to_string() const {
	return name;
}
