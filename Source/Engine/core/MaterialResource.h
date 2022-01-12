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
	template <class T>
	inline void hash_combine(std::size_t& seed, const T& v)
	{
		std::hash<T> hasher{};
		seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
	}

	const uint32_t Prime = 0x01000193;
	const uint32_t Seed = 0x811C9DC5;

	inline uint32_t fnv1a(unsigned char b, uint32_t hash = Seed)
	{
		return (b ^ hash) * Prime;
	}

	inline uint32_t fnv1a(const void* data, size_t numBytes, uint32_t hash = Seed)
	{
		assert(data);
		const unsigned char* ptr = reinterpret_cast<const unsigned char*>(data);
		while (numBytes--)
		{
			hash = (*ptr++ ^ hash) * Prime;
		}
		return hash;
	}
	template<typename T>
	inline uint32_t fnv1a(std::vector<T> const& data, uint32_t hash = Seed)
	{
		if (data.empty())
			return hash;

		return fnv1a(data.data(), data.size() * sizeof(T), hash);
	}

	inline uint32_t fnv1a(std::string const& data, uint32_t hash = Seed)
	{
		return fnv1a(data.data(), data.size() * sizeof(data[0]), hash);
	}



	template<>
	struct hash<MaterialInitParameters>
	{
		std::size_t operator()(MaterialInitParameters const& obj)
		{
			uint32_t hash = 0;
			hash = fnv1a(obj.ps_shader_bytecode, obj.ps_shader_bytecode_size, hash);
			hash = fnv1a(obj.vs_shader_bytecode, obj.vs_shader_bytecode_size, hash);
			hash = fnv1a(obj.ps_shader_path, hash);
			hash = fnv1a(obj.vs_shader_path, hash);


			for (auto& p : obj.m_texture_paths)
			{
				hash = fnv1a(p, hash);
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

