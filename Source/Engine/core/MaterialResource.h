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

	bool double_sided;

	// Textures
	enum TextureType
	{
		TextureType_Albedo,
		TextureType_Roughness,
		TextureType_Metalness,
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
			hash = Hash::fnv1a(obj.name, hash);
			hash = Hash::fnv1a(obj.double_sided, hash);
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
	~MaterialResource() {}

	void load(enki::ITaskSet* parent) override;
};

