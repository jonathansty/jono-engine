
template <typename T>
std::shared_ptr<T> ResourceLoader::load(typename T::init_parameters params, bool dependencyLoad, bool blocking)
{
    MEMORY_TAG(MemoryCategory::ResourceLoading);

    LOG_VERBOSE(IO, "Load request {}", params.to_string());
    std::size_t h = std::hash<typename T::init_parameters>{}(params);

    {
        std::lock_guard lock{ m_CacheLock };
        if (auto it = m_Cache.find(h); it != m_Cache.end())
        {
            LOG_VERBOSE(IO, "Returned cached copy for {}", params.to_string());

            if (blocking)
            {
                it->second->WaitForLoad();
            }

            return std::static_pointer_cast<T>(it->second);
        }
    }

    std::shared_ptr<T> res = std::make_shared<T>(params);
    {
        std::lock_guard lock{ m_CacheLock };
        m_Cache[h] = res;
    }

    res->SetStatus(ResourceStatus::Loading);

    if (blocking)
    {
        bool result = res->load(nullptr);
        if(!result)
        {
            res->SetStatus(ResourceStatus::Error);
            return nullptr;
        }
        res->SetStatus(ResourceStatus::Loaded);
    }
    else
    {
        std::unique_ptr<enki::ITaskSet> set = std::make_unique<LoadResourceTask<T>>(res);
        Tasks::get_scheduler()->AddTaskSetToPipe(set.get());

        {
            std::lock_guard<std::mutex> l{ m_TaskLock };
            m_Tasks.push_back(std::move(set));
        }
    }

    return res;
}
