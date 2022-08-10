#pragma once

#include "singleton.h"

#include <fmt/core.h>
#include "Logging.h"

struct FromFileResourceParameters
{
	std::string path;

	std::string const& to_string() const { return path; }
};

enum class ResourceStatus
{
	Error,
	Loading,
	Loaded
};

// template resource class
class Resource
{
public:
	friend class ResourceLoader;
	Resource() : _loaded(true), _status(ResourceStatus::Error)
	{
	}

	virtual ~Resource()
	{
	}

	ResourceStatus get_status() const { return _status; }

	bool is_loaded() 
	{
		return _status == ResourceStatus::Loaded;
	}

	virtual void load(enki::ITaskSet* parent) = 0;

	virtual std::string get_name() const { return ""; }

	virtual void build_load_graph(enki::ITaskSet* parent){}


	std::atomic<bool> _loaded;
	std::atomic<ResourceStatus> _status;

	std::mutex _loaded_mutex;
	std::condition_variable _loaded_cv;

	friend class ResourceLoader;
	friend struct CompletionActionMarkLoaded;
};

struct NoInit {};

template <typename resource_type, typename init_type = NoInit>
class TCachedResource : public Resource
{
public:
	using init_parameters = init_type;

	TCachedResource(init_type init_options = init_type())
			: _init(init_options)
	{
	}

	resource_type* operator->()
	{
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


struct CompletionActionMarkLoaded : enki::ICompletable
{
	enki::Dependency _dependency;

	// Resource to mark as loaded
	std::shared_ptr<Resource> _resource;

	void OnDependenciesComplete(enki::TaskScheduler* pTaskScheduler_, uint32_t threadNum_) override
	{
		ICompletable::OnDependenciesComplete(pTaskScheduler_, threadNum_);

		LOG_VERBOSE(IO, "Asset finished loading. (\"{}\")", _resource->get_name());

		_resource->_status = ResourceStatus::Loaded;
		_resource->_loaded = true;
		_resource->_loaded_cv.notify_all();
	}
};

template<typename T>
struct LoadResourceTask : public enki::ITaskSet
{
	LoadResourceTask(std::shared_ptr<T> resource)
		: _resource(resource)
	{

	}

	~LoadResourceTask()
	{

	}

	std::shared_ptr<T> _resource;
	std::function<void(std::shared_ptr<T>)> _complete;

	void ExecuteRange(enki::TaskSetPartition range_, uint32_t threadnum_) override
	{
		_resource->_status = ResourceStatus::Loading;
		_resource->load(this);
		_resource->_status = ResourceStatus::Loaded;
		_resource->_loaded = true;
		_resource->_loaded_cv.notify_all();
		if(_complete)
		{
			_complete(_resource);
		}


	}
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

	void wait_for(shared_ptr<Resource> resource)
	{
		if(resource->get_status() == ResourceStatus::Loading)
		{
			std::unique_lock lk{ resource->_loaded_mutex };
			resource->_loaded_cv.wait(lk);
		}
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
		for (auto it = _cache.begin(); it != _cache.end(); ++it)
		{
			if (it->second.use_count() == 1)
			{
				models_to_remove.push_back(it);
			}
		}

		for (auto it : models_to_remove)
		{
			LOG_VERBOSE(IO, "Unloading \"{}\"", it->second->get_name());
			_cache.erase(it);
		}
	}

	template <typename T>
	std::shared_ptr<T> load(typename T::init_parameters parameters,bool dependencyLoad, bool blocking = false);

	template<typename T>
	static LoadResourceTask<T>* create_load_task(std::shared_ptr<T> const& resource)
	{
		return new LoadResourceTask<T>(resource);
	}
private:



	// Returns a lambda that can be used for inline resource loading
	template <typename T>
	static std::function<void()> load_fn(std::shared_ptr<T> const& resource)
	{
		return [resource]() {
			resource->load(nullptr);
			resource->_loaded = true;
			resource->_status = ResourceStatus::Loaded;
			resource->_loaded_cv.notify_all();
			LOG_INFO(IO, "{} finished", resource->get_init_parameters().to_string());
		};
	}


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
std::shared_ptr<T> ResourceLoader::load(typename T::init_parameters params, bool dependencyLoad, bool blocking )
{
	LOG_INFO(IO, "Load request {}", params.to_string());
	std::size_t h = std::hash<typename T::init_parameters>{}(params);

	{
		std::lock_guard lock{ _cache_lock };
		if (auto it = _cache.find(h); it != _cache.end())
		{
			LOG_INFO(IO, "Returned cached copy for {}", params.to_string());

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
	res->_status = ResourceStatus::Loading;

	auto do_load = load_fn(res);

	enki::ITaskSet* set = create_load_task(res);

	if (blocking)
	{
		Tasks::get_scheduler()->AddTaskSetToPipe(set);
		Tasks::get_scheduler()->WaitforTask(set);
	}
	else
	{
		{
			std::lock_guard<std::mutex> l{ _tasks_lock };
			_tasks.push_back(set);
		}

		Tasks::get_scheduler()->AddTaskSetToPipe(set);
	}

	return res;
}
