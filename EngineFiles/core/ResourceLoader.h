#pragma once

// template resource class
interface Resource
{
	virtual void load(std::string const& path) = 0;
};


class ResourceLoader final : public TSingleton<ResourceLoader>
{
public:
	~ResourceLoader() {}

	void UnloadAll()
	{
		_cache.clear();
	}

	template<typename T>
	std::shared_ptr<T> load(std::string const& path);


private:
	std::map<std::string, std::shared_ptr<Resource>> _cache;

};

template<typename T>
std::shared_ptr<T> ResourceLoader::load(std::string const& path)
{
	if (auto it = _cache.find(path); it != _cache.end())
	{
		return std::static_pointer_cast<T>(it->second);
	}
		
	std::shared_ptr<T> res = std::make_shared<T>();
	res->load(path);

	_cache[path] = res;
	return res;
}
