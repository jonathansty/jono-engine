#include "engine.pch.h"
#include "Material.h"
#include "Graphics/Shader.h"
#include "GameEngine.h"
#include "ModelResource.h"
#include "TextureResource.h"
#include "MaterialResource.h"

#include "Shaders/CommonShared.h"
#include "Graphics/ShaderType.h"
#include "Graphics/ShaderCache.h"
#include "Graphics/Renderer.h"

 Material::Material()
		: _double_sided(false)
		, _vertex_shader()
		, _pixel_shader()
		, _debug_pixel_shader()
		, _textures()
{
}

std::unique_ptr<Material> Material::create(Graphics::ShaderRef const& vertex_shader, Graphics::ShaderRef const& pixel_shader, u32 initialDataSize, void* initialData)
{
	auto obj = std::make_unique<Material>();
	obj->_vertex_shader = vertex_shader;
	obj->_pixel_shader = pixel_shader;

	// For now each material has it's own constant buffer
	obj->_material_cb = ConstantBuffer::create(Graphics::get_device().Get(), initialDataSize, false, BufferUsage::Default, initialData);

	return obj;
}

std::unique_ptr<Material> Material::load(std::string const& path)
{
	IO::IFileRef file = IO::get()->open(path.c_str(), IO::Mode::Read, false);
	if(file)
	{
		using namespace tinyxml2;
		tinyxml2::XMLDocument document{};
		if(document.LoadFile(path.c_str()) == tinyxml2::XML_SUCCESS)
		{
			using namespace Graphics;
			XMLElement* root = document.FirstChildElement("material");
			XMLElement* infoNode = root->FirstChildElement("info");
			XMLElement* definesNode = root->FirstChildElement("defines");
			XMLElement* parametersNode = root->FirstChildElement("parameters");
			XMLElement* texturesNode = root->FirstChildElement("textures");

			std::string name = infoNode->FirstChildElement("name")->FindAttribute("value")->Value();
			std::string pixel_path = infoNode->FirstChildElement("pixel_shader")->FindAttribute("value")->Value();
			std::string debug_pixel_path = infoNode->FirstChildElement("debug_shader")->FindAttribute("value")->Value();
			std::string vertex_shader_path = infoNode->FirstChildElement("vertex_shader")->FindAttribute("value")->Value();
			bool double_sided = infoNode->FirstChildElement("double_sided")->FindAttribute("value")->BoolValue();

			ShaderCompiler::CompileParameters params{};
			params.entry_point = "main";

			for (XMLElement* defineNode = definesNode->FirstChildElement(); defineNode != nullptr; defineNode = defineNode->NextSiblingElement())
			{
				std::string define = defineNode->FindAttribute("id")->Value();
				std::string value = defineNode->FindAttribute("value")->Value();
				params.defines.push_back({ define, value });
			}
			params.flags = ShaderCompiler::CompilerFlags::CompileDebug;
			params.stage = ShaderType::Pixel;

			std::vector<u8> pixel_bytecode;
			std::vector<u8> vertex_bytecode;
			std::vector<u8> debug_bytecode;

			// Get pixel shader
			ShaderCreateParams create_params{};
			create_params.params = params;
			create_params.path = pixel_path;
			auto pixel_shader = ShaderCache::instance()->find_or_create(create_params);

			create_params.params = params;
			create_params.path = debug_pixel_path;
			auto debug_shader = ShaderCache::instance()->find_or_create(create_params);

			create_params.params.stage = ShaderType::Vertex;
			create_params.path = vertex_shader_path;
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

			std::unique_ptr<Material> result = std::make_unique<Material>();
			result->_double_sided = double_sided;
			result->_vertex_shader = vertex_shader;
			result->_pixel_shader = pixel_shader;
			result->_debug_pixel_shader = debug_shader;

			u32 param_data_byte_size = 0; 
			for (XMLElement* parameterNode = parametersNode->FirstChildElement(); parameterNode != nullptr; parameterNode = parameterNode->NextSiblingElement())
			{
				// Offset in floats
				u32 offset = parameterNode->FindAttribute("offset")->UnsignedValue();
				std::string parameter_name = parameterNode->FindAttribute("id")->Value();
				Identifier64 param_hash = Identifier64(parameter_name);
				std::string parameter_type = parameterNode->FindAttribute("type")->Value();
				std::string parameter_value = parameterNode->FindAttribute("value")->Value();

				static std::unordered_map<Identifier64, size_t> s_TypeSizes = {
					{ Identifier64("float"), sizeof(f32) },
					{ Identifier64("float2"), 2 * sizeof(f32) },
					{ Identifier64("float3"), 3 * sizeof(f32) },
					{ Identifier64("float4"), 4 * sizeof(f32) },
					{ Identifier64("int"), sizeof(int) }
				};

				ParameterInfo info{};
				info.offset = offset; // Convert to offset in bytes

				u32 byte_size = (u32)s_TypeSizes[Identifier64(parameter_type)];
				info.size = byte_size;

				param_data_byte_size += byte_size;
				result->_param_data.resize(param_data_byte_size);

				float* data = (float*)result->_param_data.data();

				if (parameter_type == "float3" || parameter_type == "float4")
				{
					// Parse float3 
					float values[4] = {};

					u32 i = 0;
					while (parameter_value.find(',') != std::string::npos)
					{
						size_t pos = parameter_value.find(',');
						std::string value = parameter_value.substr(0, pos);
						values[i++] = std::stof(value.c_str());

						parameter_value = parameter_value.substr(pos + 1);
					}

					if(!parameter_value.empty())
					{
						values[i++] = std::stof(parameter_value.c_str());
					}

					for(u32 j = 0; j < i; ++j)
					{
						data[offset + j] = values[j];
					}
				}
				else if (parameter_type == "float")
				{
					// Parse float
					float value = stof(parameter_value);
					data[offset] = value;

					info.size = 1;
				}
				else
				{
					FAILMSG("Type unsupported!");
				}

				result->_parameters[param_hash] = info;
			}

			result->_material_cb = ConstantBuffer::create(Graphics::get_device().Get(), param_data_byte_size, false, BufferUsage::Default, result->_param_data.data());

			for (XMLElement* textureNode = texturesNode->FirstChildElement(); textureNode != nullptr; textureNode = textureNode->NextSiblingElement())
			{
				std::string_view texture_path = textureNode->FindAttribute("path")->Value();
				std::string_view texture_id = textureNode->FindAttribute("id")->Value();
				u32 slot = textureNode->FindAttribute("slot")->IntValue();

				result->_textures.resize(slot + 1);
				result->_texture_slot_mapping[Identifier64(texture_id.data())] = slot;

				if (texture_path[0] == '*')
				{
					if (texture_path == "*White")
					{
						result->_textures[slot] = TextureHandle::white();
					}
					else if (texture_path == "*Black")
					{
						result->_textures[slot] = TextureHandle::black();
					}
					else if (texture_path == "*Normal")
					{
						result->_textures[slot] = (TextureHandle::default_normal());
					}
					else if (texture_path == "*MetalnessRoughness")
					{
						result->_textures[slot] = TextureHandle::default_roughness();
					}
				}
				else
				{
					TextureHandle::init_parameters load_params{};
					load_params.path = texture_path;
					auto handle = *ResourceLoader::instance()->load<TextureHandle>(load_params, true, true);
					result->_textures[slot] = handle;
				}

			}

			return result;
		}
		else
		{
			return {};
		
		}
	}

	return {};
}

ComPtr<ID3D11InputLayout> Material::get_input_layout() const
{
	return _vertex_shader->get_input_layout();
}

void Material::get_texture_views(std::vector<ID3D11ShaderResourceView const*>& views) const
{
	for (std::size_t i = 0; i < _textures.size(); ++i)
	{
		if (_textures[i])
		{
			Texture const* texture = *_textures[i];
			views.push_back(texture->get_srv());
		}
	}
}

u32 Material::get_texture_count() const
{
	return (u32)_textures.size();
}

std::vector<u8> const& Material::get_param_data() const
{
	return _param_data;
}

ParameterInfo const* Material::find_parameter(Identifier64 const& id) const
{
	if(auto it = _parameters.find(id); it != _parameters.end())
	{
		return &it->second;
	}
	return nullptr;
}

void Material::apply(Graphics::Renderer* renderer, Graphics::ViewParams const& params) const
{
	auto ctx = renderer->get_ctx()._ctx;
	if (is_double_sided())
	{
		ctx->RSSetState(Graphics::get_rasterizer_state(RasterizerState::CullNone).Get());
	}
	else
	{
		ctx->RSSetState(Graphics::get_rasterizer_state(RasterizerState::CullBack).Get());
	}

	Graphics::ShaderConstRef vertex_shader = get_vertex_shader();
	Graphics::ShaderConstRef pixel_shader = get_pixel_shader();
	Graphics::ShaderConstRef debug_shader = get_debug_pixel_shader();
	if (!vertex_shader->is_valid())
	{
		vertex_shader = Graphics::get_error_shader_vx();
	}

	if (!pixel_shader->is_valid())
	{
		pixel_shader = Graphics::get_error_shader_px();
	}

	if (!debug_shader->is_valid())
	{
		debug_shader = Graphics::get_error_shader_px();
	}

	// Bind vertex shader
	renderer->VSSetShader(vertex_shader);
	ctx->IASetInputLayout(vertex_shader->get_input_layout().Get());

	// In opaque pass, bind the pixel shader and relevant shader resources from the material
	if (params.pass == Graphics::RenderPass::Opaque)
	{
		// Bind pixel shader

		extern int g_DebugMode;
		if (g_DebugMode)
		{
			renderer->PSSetShader(debug_shader);
		}
		else
		{
			renderer->PSSetShader(pixel_shader);
		}

		// Bind material parameters
		std::vector<ID3D11ShaderResourceView const*> views{};
		get_texture_views(views);

		ASSERTMSG(views.size() <= Texture_MaterialSlotEnd, "Currently we do not support more than 5 textures per material.");
		ctx->PSSetShaderResources(Texture_MaterialSlotStart, (UINT)views.size(), (ID3D11ShaderResourceView**)views.data());

		ID3D11Buffer* buffer[1] = { get_cb()->Get() };
		ctx->PSSetConstantBuffers(Buffer_Material, 1, buffer);
	}
	else
	{
		ctx->PSSetShader(nullptr, nullptr, 0);
	}
}

MaterialInstance::MaterialInstance(std::shared_ptr<MaterialHandle const> const& baseMaterial)
		: _resource(baseMaterial)
		, _has_overrides(false)
		, _needs_flush(false)
		 , _obj(nullptr)
{
	// Construct a copy of the original buffer + overrides
	_textures.resize(get_material_obj()->get_texture_count());
}

 MaterialInstance::MaterialInstance()
{
}

MaterialInstance::~MaterialInstance()
{
}

void MaterialInstance::bind(IMaterialObject const* obj)
{
	_obj = obj;
	_param_data = obj->get_param_data();
}

void MaterialInstance::apply(Graphics::Renderer* renderer, Graphics::ViewParams const& params) const
{
	auto ctx = renderer->get_ctx()._ctx;
	if (is_double_sided())
	{
		renderer->RSSetState(Graphics::get_rasterizer_state(RasterizerState::CullNone).Get());
	}
	else
	{
		renderer->RSSetState(Graphics::get_rasterizer_state(RasterizerState::CullBack).Get());
	}

	Graphics::ShaderConstRef vertex_shader = get_vertex_shader();
	Graphics::ShaderConstRef pixel_shader = get_pixel_shader();
	Graphics::ShaderConstRef debug_shader = get_debug_pixel_shader();
	if (!vertex_shader->is_valid())
	{
		vertex_shader = Graphics::get_error_shader_vx();
	}

	if (!pixel_shader->is_valid())
	{
		pixel_shader = Graphics::get_error_shader_px();
	}

	if (!debug_shader->is_valid())
	{
		debug_shader = Graphics::get_error_shader_px();
	}

	// Bind vertex shader
	renderer->VSSetShader(vertex_shader);
	renderer->IASetInputLayout(vertex_shader->get_input_layout().Get());

	// In opaque pass, bind the pixel shader and relevant shader resources from the material
	if (params.pass == Graphics::RenderPass::Opaque)
	{
		// Bind pixel shader

		extern int g_DebugMode;
		if (g_DebugMode)
		{
			renderer->PSSetShader(debug_shader);
		}
		else
		{
			renderer->PSSetShader(pixel_shader);
		}

		// Bind material parameters
		std::vector<ID3D11ShaderResourceView const*> views{};
		get_texture_views(views);

		ASSERTMSG(views.size() <= Texture_MaterialSlotEnd, "Currently we do not support more than 5 textures per material.");
		ctx->PSSetShaderResources(Texture_MaterialSlotStart, (UINT)views.size(), (ID3D11ShaderResourceView**)views.data());

		ID3D11Buffer* buffer[1] = { get_cb()->Get() };
		ctx->PSSetConstantBuffers(Buffer_Material, 1, buffer);
	}
	else
	{
		renderer->PSSetShader(nullptr);
	}
}

u32 MaterialInstance::get_slot(Identifier64 const& slot_id) const
{
	return get_material_obj()->get_slot(slot_id);
}

void MaterialInstance::set_texture(Identifier64 const& slot_id, std::shared_ptr<class TextureHandle> const& tex)
{
	_needs_flush = true;
	_textures[get_slot(slot_id)] = tex;
}

void MaterialInstance::set_texture(u32 slot, std::shared_ptr<class TextureHandle> const& resource)
{
	_textures[slot] = resource;
	_needs_flush = true;
}

void MaterialInstance::set_param_float(Identifier64 const& parameter_id, float value)
{
	FloatParameter param = FloatParameter{
		parameter_id,
		value
	};
	_float_params.push_back(param);
	_needs_flush = true;
}

void MaterialInstance::set_param_float3(Identifier64 const& parameter_id, float3 value)
{
	Float3Parameter param = Float3Parameter{
		parameter_id,
		value
	};
	_float3_params.push_back(param);
	_needs_flush = true;
}

void MaterialInstance::update()
{
	if (_needs_flush)
	{
		IMaterialObject const* obj = get_material_obj();

		// Apply all the float parameters
		float* data = reinterpret_cast<float*>(_param_data.data());
		for (FloatParameter const& param : _float_params)
		{
			ParameterInfo const* info = obj->find_parameter(param.hash);
			data[info->offset] = param.value;
		}
		_float_params.clear();

		for(Float3Parameter const& param : _float3_params)
		{
			ParameterInfo const* info = obj->find_parameter(param.hash);
			data[info->offset + 0] = param.value.x;
			data[info->offset + 1] = param.value.y;
			data[info->offset + 2] = param.value.z;
		
		}
		_float3_params.clear();

		if (_has_overrides)
		{
			// update the paramter data
			_instance_cb = ConstantBuffer::create(Graphics::get_device().Get(), u32(_param_data.size()) * sizeof(float), false, BufferUsage::Default, _param_data.data());
		}

		_needs_flush = false;
	}
}

void MaterialInstance::get_texture_views(std::vector<ID3D11ShaderResourceView const*>& views) const
{
	// Get the base views
	get_material_obj()->get_texture_views(views);

	// Now apply the instance it's views
	for (std::size_t i = 0; i < _textures.size(); ++i)
	{
		if (_textures[i])
		{
			Texture const* texture = _textures[i]->get();
			views[i] = texture->get_srv();
		}
	}
}

ConstantBufferRef const& MaterialInstance::get_cb() const
{
	if (_instance_cb)
	{
		return _instance_cb;
	}

	return get_material_obj()->get_cb();
}

Material const* MaterialInstance::get_material() const
{
	MaterialHandle const* res = _resource.get();
	return res->get();
}

bool MaterialInstance::is_double_sided() const
{
	return get_material_obj()->is_double_sided();
}

IMaterialObject const* MaterialInstance::get_material_obj() const
{
	return _obj ? _obj : _resource->get();
}

u32 MaterialInstance::get_texture_count() const
{
	return get_material_obj()->get_texture_count();
}

std::vector<u8> const& MaterialInstance::get_param_data() const
{
	return get_material_obj()->get_param_data();
}

ParameterInfo const* MaterialInstance::find_parameter(Identifier64 const& id) const
{
	return get_material_obj()->find_parameter(id);
}
