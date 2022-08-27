#include "engine.pch.h"

#include "Logging.h"
#include "ShaderCache.h"

namespace Graphics
{

std::shared_ptr<Shader> ShaderCache::find_or_create(ShaderCreateParams const& creation_params)
{
	if (auto it = _shaders.find(creation_params); it != _shaders.end())
	{
		return it->second;
	}

	// Shader doesn't exist? Then compile it.

	std::shared_ptr<Shader> result = nullptr;

	std::vector<u8> bytecode;
	if (ShaderCompiler::compile(creation_params.path.c_str(), creation_params.params, bytecode))
	{
		result = Shader::create(creation_params.params.stage, bytecode.data(), static_cast<u32>(bytecode.size()), creation_params.path.c_str());
	}
	else
	{
		result = std::make_shared<Shader>();
	}

	// Always populate the cache and return a shader object so we can check if the shader is valid when trying to bind it to allow hot reloading.
	_shaders[creation_params] = result;
	return _shaders[creation_params];
}

bool ShaderCache::reload_all()
{
	bool success = true;
	for (auto it : _shaders)
	{
		std::vector<u8> bytecode;
		if (!ShaderCompiler::compile(it.first.path.c_str(), it.first.params, bytecode))
		{
			success = false;

			// Set invalid shader
			auto tmp = std::make_shared<Shader>();
			*it.second = *tmp;
		}
		else
		{
			auto tmp = Shader::create(it.first.params.stage, bytecode.data(), static_cast<u32>(bytecode.size()));
			*it.second = *tmp;
		}

	}
	return success;
}

bool ShaderCache::reload(ShaderCreateParams const& params)
{
	auto it = _shaders.find(params);
	if (it == _shaders.end())
	{
		LOG_ERROR(Graphics, "Failed to find compiled shader \"{}\"", params.path.c_str());
		return false;
	}
	std::vector<u8> bytecode;
	if (!ShaderCompiler::compile(it->first.path.c_str(), it->first.params, bytecode))
	{
		// Set invalid shader
		auto tmp = std::make_shared<Shader>();
		*it->second = *tmp;

		return false;
	}

	auto tmp = Shader::create(it->first.params.stage, bytecode.data(), static_cast<u32>(bytecode.size()));
	*it->second = *tmp;
	return true;
}

}