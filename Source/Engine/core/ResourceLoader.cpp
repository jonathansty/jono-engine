#include "engine.pch.h"

#include "ResourceLoader.h"
#include "Resource.h"

ResourceLoader::ResourceLoader()
{
}

ResourceLoader::~ResourceLoader()
{
}

void ResourceLoader::unload_all()
{
    m_Cache.clear();
}

void ResourceLoader::wait_for(shared_ptr<Resource> resource)
{
    if (resource->get_status() == ResourceStatus::Loading)
    {
        std::unique_lock lk{ resource->m_LoadedMtx };
        resource->m_LoadedCV.wait(lk);
    }
}

void ResourceLoader::update()
{
    JONO_EVENT();
    MEMORY_TAG(MemoryCategory::ResourceLoading);
    std::lock_guard<std::mutex> l{ m_TaskLock };
    std::vector<std::list<enki::ITaskSet*>::iterator> to_remove;
    for (auto it = m_Tasks.begin(); it != m_Tasks.end();)
    {
        if ((*it)->GetIsComplete())
        {
            it = m_Tasks.erase(it);
        }
        else
        {
            ++it;
        }
    }

    std::vector<ResourceCache::iterator> models_to_remove;
    for (auto it = m_Cache.begin(); it != m_Cache.end(); ++it)
    {
        if (it->second.use_count() == 1)
        {
            models_to_remove.push_back(it);
        }
    }

    for (auto it : models_to_remove)
    {
        LOG_VERBOSE(IO, "Unloading \"{}\"", it->second->get_name());
        m_Cache.erase(it);
    }
}

void CompletionActionMarkLoaded::OnDependenciesComplete(enki::TaskScheduler* pTaskScheduler_, uint32_t threadNum_)
{
    ICompletable::OnDependenciesComplete(pTaskScheduler_, threadNum_);
    LOG_VERBOSE(IO, "Asset finished loading. (\"{}\")", _resource->get_name());
    _resource->SetStatus(ResourceStatus::Loaded);
}
