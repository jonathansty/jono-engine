#pragma once

#include "singleton.h"

#include "Logging.h"
#include "Memory.h"

#include "CachedResource.h"
#include "Resource.h"
#include "ResourceTypes.h"

#include "ResourceTasks.h"

class ResourceLoader final : public TSingleton<ResourceLoader>
{
public:
    using ResourceCache = std::map<std::size_t, std::shared_ptr<Resource>>;

    ResourceLoader();
    ~ResourceLoader();

    void unload_all();

    void wait_for(shared_ptr<Resource> resource);

    void update();

    template <typename T>
    std::shared_ptr<T> load(typename T::init_parameters parameters, bool dependencyLoad, bool blocking = false);

private:
    std::mutex m_CacheLock;
    ResourceCache m_Cache;

    std::mutex m_TaskLock;
    std::list<std::unique_ptr<enki::ITaskSet>> m_Tasks;
};

#include "ResourceLoader.inl"
