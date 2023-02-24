#include "engine.pch.h"

#include "GameEngine.h"
#include "MaterialResource.h"
#include "ModelResource.h"
#include "TextureResource.h"

#include "Graphics/ShaderCache.h"
#include "Material.h"

#include "Graphics/ShaderCompiler.h"

bool MaterialHandle::load(enki::ITaskSet* parent)
{
	using namespace Graphics;

	if(get_init_parameters().load_type == MaterialInitParameters::LoadType_FromFile)
	{
		_resource = Material::load(get_init_parameters().name);
	}
	else
	{
		ShaderCompiler::CompileParameters params{};
		params.entry_point = "main";
		params.defines.push_back({ "LIGHTING_MODEL", "LIGHTING_MODEL_PBR" });
		params.flags = ShaderCompiler::CompilerFlags::CompileDebug;
		params.stage = ShaderStage::Pixel;

		std::vector<u8> pixel_bytecode;
		std::vector<u8> vertex_bytecode;
		std::vector<u8> debug_bytecode;

		// Get pixel shader
		ShaderCreateParams create_params{};
		create_params.params = params;
		create_params.path = "Source/Engine/Shaders/DefaultGGX_Opaque.px.hlsl";
		auto pixel_shader = ShaderCache::instance()->find_or_create(create_params);

		// Get debug vertex shader
		create_params.params = params;
		create_params.path = "Source/Engine/Shaders/DefaultGGX_Debug.px.hlsl";
		auto debug_shader = ShaderCache::instance()->find_or_create(create_params);

		create_params.params.stage = ShaderStage::Vertex;
		create_params.path = "Source/Engine/Shaders/DefaultVertex.vx.hlsl";
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

		if(!pixel_shader || !vertex_shader || !debug_shader)
        {
            LOG_FATAL(Graphics, "Error shaders did not compile. Check log.");
			return false;
		}

		Material::ConstantBufferData initialData{};
		initialData.albedo = float3(1.0);
		initialData.metalness = 1.0f;
		initialData.roughness = 1.0f;
		_resource = Material::create(vertex_shader, pixel_shader, sizeof(Material::ConstantBufferData), (void*)& initialData);

		_resource->_debug_pixel_shader = debug_shader;
		_resource->_double_sided = get_init_parameters().double_sided;

		// Initiate texture loads
		auto paths = get_init_parameters().m_texture_paths;
		for (u32 textureType = 0; textureType < MaterialInitParameters::TextureType_Count; ++textureType)
		{
			if (paths[textureType].empty())
			{
				switch (textureType)
				{
					case MaterialInitParameters::TextureType_Albedo:
						_resource->_textures.push_back(TextureHandle::white());
						break;
					case MaterialInitParameters::TextureType_MetalnessRoughness:
						_resource->_textures.push_back(TextureHandle::default_roughness());
						break;
					case MaterialInitParameters::TextureType_Normal:
						_resource->_textures.push_back(TextureHandle::default_normal());
						break;
					case MaterialInitParameters::TextureType_Count:
						_resource->_textures.push_back(TextureHandle::invalid());
						break;
				}
			}
			else
			{
				TextureHandle res = *ResourceLoader::instance()->load<TextureHandle>({ paths[textureType] }, true);
				_resource->_textures.push_back(res);
			}
		}
	}

    return _resource != nullptr;
}

std::string MaterialInitParameters::to_string() const
{
	return name;
}
