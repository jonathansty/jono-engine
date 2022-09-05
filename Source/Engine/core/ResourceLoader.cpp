#include "engine.pch.h"
#include "ResourceLoader.h"

void ResourceLoader::unload_all()
{
	_cache.clear();
}

void ResourceLoader::wait_for(shared_ptr<Resource> resource)
{
	if (resource->get_status() == ResourceStatus::Loading)
	{
		std::unique_lock lk{ resource->_loaded_mutex };
		resource->_loaded_cv.wait(lk);
	}
}

void ResourceLoader::update()
{
	JONO_EVENT();
	MEMORY_TAG(MemoryCategory::ResourceLoading);
	std::lock_guard<std::mutex> l{ _tasks_lock };
	std::vector<std::list<enki::ITaskSet*>::iterator> to_remove;
	for (auto it = _tasks.begin(); it != _tasks.end();)
	{
		if ((*it)->GetIsComplete())
		{
			it = _tasks.erase(it);
		}
		else
		{
			++it;
		}
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
