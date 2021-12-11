#pragma once

#include "singleton.h"

#include <fmt/core.h>

struct FromFileResourceParameters
{
	std::string path;

	std::string to_string() const { return path; }
};

// template resource class
class Resource
{
public:
	Resource() : _loaded(false)
	{
	}

	virtual ~Resource()
	{
	}

	bool is_loaded() 
	{
		return _loaded;
	}

	virtual void load() = 0;

private:
	std::atomic<bool> _loaded;

	friend class ResourceLoader;
};

struct NoInit {};

template<typename resource_type, typename init_type = NoInit>
class TCachedResource : public Resource
{
public:
	using init_parameters = init_type;

	TCachedResource(init_type init_options = init_type())
		: _init(init_options)
	{

	}

	resource_type* operator->() {
		return _resource.get();
	}

	resource_type* get() const { return _resource.get(); }

	public:
	init_type const& get_init_parameters() const { return _init; }

protected:

	std::shared_ptr<resource_type> _resource;
private:
	init_type _init;
};


class ResourceLoader final : public TSingleton<ResourceLoader>
{
public:
	~ResourceLoader() {}

	void unload_all()
	{
		_cache.clear();
	}

	void update()
	{
		std::lock_guard<std::mutex> l{ _tasks_lock };
		std::vector<std::list<enki::ITaskSet*>::iterator> to_remove;
		for (auto it = _tasks.begin(); it != _tasks.end(); ++it)
		{
			if ((*it)->GetIsComplete())
			{
				delete *it;
				to_remove.push_back(it);
			}
		}

		for (auto e : to_remove)
		{
			_tasks.erase(e);
		}
	}

	template<typename T>
	std::shared_ptr<T> load(typename T::init_parameters parameters, bool blocking = false);


private:
	std::map<std::size_t, std::shared_ptr<Resource>> _cache;

	std::mutex _tasks_lock;
	std::list<enki::ITaskSet*> _tasks;

};

namespace std
{
	template<>
	struct hash<FromFileResourceParameters>
	{
		std::size_t operator()(FromFileResourceParameters const& obj)
		{
			return std::hash<std::string>{}(obj.path);
		}
	};
}

template<typename T>
std::shared_ptr<T> ResourceLoader::load(typename T::init_parameters params, bool blocking )
{
	fmt::print("[LOAD] Load request {}\n", params.to_string());
	std::size_t  h = std::hash<typename T::init_parameters>{}(params);
	if (auto it = _cache.find(h); it != _cache.end())
	{
		fmt::print("[LOAD] Returned cached copy for {}\n", params.to_string());
		return std::static_pointer_cast<T>(it->second);
	}

	std::shared_ptr<T> res = std::make_shared<T>(params);
	_cache[h] = res;

	if (blocking)
	{
		res->load();
		res->_loaded = true;
		fmt::print("[LOAD] {} finished\n", params.to_string());
	}
	else 
	{
		enki::TaskSet* set = new enki::TaskSet([=](enki::TaskSetPartition partition, uint32_t thread_num) {
			res->_loaded = false;
			res->load();
			res->_loaded = true;
			fmt::print("[LOAD] {} finished\n", params.to_string());
		});

		{
			std::lock_guard<std::mutex> l{ _tasks_lock };
			_tasks.push_back(set);
		}
		GameEngine::s_TaskScheduler.AddTaskSetToPipe(set);
	}

	return res;
}
