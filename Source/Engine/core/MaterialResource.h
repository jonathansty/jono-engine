#pragma once
#include "ResourceLoader.h"

class Material;
using MaterialRef = std::shared_ptr<class MaterialResource>;

// Materialinitialization parameters
struct MaterialInitParameters
{
	enum LoadType
	{
		LoadType_FromFile,
		LoadType_FromMemory
	};
	LoadType load_type;

	std::string name;
	std::string vs_shader_path;
	std::string ps_shader_path;

	const char* vs_shader_bytecode;
	uint32_t vs_shader_bytecode_size;
	const char* ps_shader_bytecode;
	uint32_t ps_shader_bytecode_size;

	bool double_sided;

	// Textures
	enum TextureType
	{
		TextureType_Albedo,
		TextureType_MetalnessRoughness,
		TextureType_Normal,
		TextureType_Count,
	};

	std::array<std::string, TextureType_Count> m_texture_paths;

	std::string to_string() const;
};

namespace std 
{


	template<>
	struct hash<MaterialInitParameters>
	{
		std::size_t operator()(MaterialInitParameters const& obj)
		{
			uint32_t hash = 0;
			hash = Hash::fnv1a(obj.ps_shader_bytecode, obj.ps_shader_bytecode_size, hash);
			hash = Hash::fnv1a(obj.vs_shader_bytecode, obj.vs_shader_bytecode_size, hash);
			hash = Hash::fnv1a(obj.ps_shader_path, hash);
			hash = Hash::fnv1a(obj.vs_shader_path, hash);


			for (auto& p : obj.m_texture_paths)
			{
				hash = Hash::fnv1a(p, hash);
			}
			return std::size_t(hash);
		}
	};
}

class MaterialResource final : public TCachedResource<Material, MaterialInitParameters>
{
public:
	MaterialResource(MaterialInitParameters params) : TCachedResource(params) {}

	virtual void load() override;
};

