#pragma once

#include "singleton.h"

#include <fmt/core.h>
#include "Logging.h"

struct FromFileResourceParameters
{
	std::string path;

	std::string const& to_string() const { return path; }
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

	virtual std::string get_name() const { return ""; }

private:
	std::atomic<bool> _loaded;

	std::mutex _loaded_mutex;
	std::condition_variable _loaded_cv;

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

	virtual std::string get_name() const { return _init.to_string(); }

protected:

	std::shared_ptr<resource_type> _resource;
private:
	init_type _init;
};


class ResourceLoader final : public TSingleton<ResourceLoader>
{
public:
	using ResourceCache = std::map<std::size_t, std::shared_ptr<Resource>>;

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

		std::vector<ResourceCache::iterator> models_to_remove;
		for(auto it = _cache.begin(); it != _cache.end(); ++it) {
	
			if(it->second.use_count() == 1) {
				models_to_remove.push_back(it);
			}
		}

		for(auto it : models_to_remove) {
			LOG_VERBOSE(IO, "Unloading \"{}\"", it->second->get_name());
			_cache.erase(it);
		}
	}

	template<typename T>
	std::shared_ptr<T> load(typename T::init_parameters parameters, bool blocking = false);


private:
	std::mutex _cache_lock;
	ResourceCache _cache;

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
	LOG_INFO(IO, "Load request {}", params.to_string());
	std::size_t  h = std::hash<typename T::init_parameters>{}(params);

	{
		std::lock_guard lock{ _cache_lock };
		if (auto it = _cache.find(h); it != _cache.end())
		{
			LOG_INFO(IO, "Returned cached copy for {}", params.to_string());

			// This is pretty nasty. When a blocking call to load happens we need to wait for the cached asset to be done loading 
			// but we also don't want to schedule a new load on the same asset
			if (blocking)
			{
				if (!it->second->_loaded)
				{
					std::unique_lock<std::mutex> lk{ it->second->_loaded_mutex };
					it->second->_loaded_cv.wait(lk);
				}
			}

			return std::static_pointer_cast<T>(it->second);
		}
	}

	std::shared_ptr<T> res = std::make_shared<T>(params);
	{
		std::lock_guard lock{ _cache_lock };
		_cache[h] = res;
	}


	res->_loaded = false;

	auto do_load = [res, params]() {
		res->load();
		res->_loaded = true;
		res->_loaded_cv.notify_all();
		LOG_INFO(IO, "{} finished", params.to_string());
	};

	if (blocking)
	{
		do_load();
	}
	else 
	{

		enki::TaskSet* set = new enki::TaskSet([=](enki::TaskSetPartition partition, uint32_t thread_num) {
			do_load();
		});

		{
			std::lock_guard<std::mutex> l{ _tasks_lock };
			_tasks.push_back(set);
		}

		Tasks::get_scheduler()->AddTaskSetToPipe(set);
	}

	return res;
}
