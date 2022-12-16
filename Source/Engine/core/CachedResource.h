#pragma once

#include "Resource.h"

template <typename ResourceT, typename InitT = NoInit>
class TCachedResource : public Resource
{
public:
    using init_parameters = InitT;

    static std::mutex s_ResourcesLock;
    static std::list<TCachedResource<ResourceT, InitT>*> s_Resources;

    JONO_INLINE TCachedResource(InitT init_options = InitT())
      : _init(init_options)
    {
        std::lock_guard lk = std::lock_guard(s_ResourcesLock);
        s_Resources.push_back(this);
    }

    JONO_INLINE TCachedResource(TCachedResource const& rhs)
      : _resource(rhs._resource)
      , _init(rhs._init)
    {
    }

    virtual ~TCachedResource()
    {
        std::lock_guard lk = std::lock_guard(s_ResourcesLock);
        auto it = std::find(s_Resources.begin(), s_Resources.end(), this);
        if (it != s_Resources.end())
        {
            s_Resources.erase(it);
        }
    }

    JONO_INLINE ResourceT* operator->()
    {
        return _resource.get();
    }

    JONO_INLINE ResourceT const* operator*() const
    {
        return _resource.get();
    }

    JONO_INLINE ResourceT* operator*()
    {
        return _resource.get();
    }

    JONO_INLINE ResourceT const* get() const
    {
        return _resource.get();
    }

    JONO_INLINE ResourceT* get()
    {
        return _resource.get();
    }

    JONO_INLINE TCachedResource& operator=(TCachedResource const& rhs)
    {
        this->_resource = rhs._resource;
        this->_init = rhs._init;
        return *this;
    }

    JONO_INLINE operator bool() const
    {
        return _resource != nullptr;
    }

public:
    InitT const& get_init_parameters() const { return _init; }

    virtual std::string get_name() const { return _init.to_string(); }

protected:
    std::shared_ptr<ResourceT> _resource;

private:
    InitT _init;
};

template <typename ResourceT, typename InitT /*= NoInit*/>
__declspec(selectany) std::mutex TCachedResource<ResourceT, InitT>::s_ResourcesLock;

template <typename ResourceT, typename InitT /*= NoInit*/>
std::list<TCachedResource<ResourceT, InitT>*> TCachedResource<ResourceT, InitT>::s_Resources;
