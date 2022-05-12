#pragma once

#include "singleton.h"
#include "ShaderCompiler.h"
#include "Shader.h"

namespace Graphics
{

class Shader;

struct ShaderCreateParams
{
	std::string path;
	ShaderCompiler::CompileParameters params;
};
}

template <>
struct std::hash<Graphics::ShaderCreateParams>
{
	std::size_t operator()(Graphics::ShaderCreateParams const& params) const noexcept
	{
		// Just do a raw memory hash
		size_t name_hash = std::hash<std::string>{}(params.path);

		size_t params_hash = std::hash<ShaderCompiler::CompileParameters>{}(params.params);
		Hash::combine(name_hash, params_hash);
		return name_hash;
	}
};

namespace Graphics
{

inline bool operator==(ShaderCreateParams const& lhs, ShaderCreateParams const& rhs)
{
	return lhs.path == rhs.path && lhs.params == rhs.params;
}


class ShaderCache : public TSingleton<ShaderCache>
{
public:
	std::shared_ptr<Shader> find_or_create(ShaderCreateParams const& params);

	bool reload_all();

	bool reload(ShaderCreateParams const& params);

private:
	std::unordered_map<ShaderCreateParams, std::shared_ptr<Shader>> _shaders;
	friend class RendererDebugTool;
};

} // namespace Graphics

