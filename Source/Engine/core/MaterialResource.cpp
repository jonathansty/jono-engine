#include "engine.pch.h"

#include "GameEngine.h"
#include "MaterialResource.h"
#include "ModelResource.h"
#include "TextureResource.h"

#include "Material.h"

#include "Graphics/ShaderCompiler.h"

void MaterialResource::load()
{
	auto device = GameEngine::instance()->GetD3DDevice();

	ShaderCompiler::CompileParameters params{};
	params.entry_point = "main";
	params.flags = ShaderCompiler::CompilerFlags::CompileDebug;
	params.stage = ShaderCompiler::ShaderStage::Pixel;

	std::vector<u8> pixel_bytecode;
	std::vector<u8> vertex_bytecode;
	std::vector<u8> debug_bytecode;
	if(!ShaderCompiler::compile("./Source/Engine/Shaders/simple_px.hlsl", params, pixel_bytecode)) {
		// Use error shader
		if(!ShaderCompiler::compile("./Source/Engine/Shaders/error_px.hlsl", params, pixel_bytecode)) {
			LOG_FATAL(Graphics, "Couldn't compile basic error shader!");
		}
		
	}
	ShaderCompiler::compile("./Source/Engine/Shaders/debug_px.hlsl", params, debug_bytecode);

	params.stage = ShaderCompiler::ShaderStage::Vertex;
	ShaderCompiler::compile("./Source/Engine/Shaders/simple_vx.hlsl", params, vertex_bytecode);

	_resource = Material::create((const char*)vertex_bytecode.data(), u32(vertex_bytecode.size()), (const char*)pixel_bytecode.data(), u32(pixel_bytecode.size()));
	_resource->_debug_pixel_shader = Shader::create(ShaderType::Pixel, (const char*)debug_bytecode.data(), u32(debug_bytecode.size()));
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
