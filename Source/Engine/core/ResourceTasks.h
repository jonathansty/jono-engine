#pragma once

struct CompletionActionMarkLoaded : enki::ICompletable
{
	enki::Dependency _dependency;

	// Resource to mark as loaded
	std::shared_ptr<Resource> _resource;

	void OnDependenciesComplete(enki::TaskScheduler* pTaskScheduler_, uint32_t threadNum_) override;
};

template <typename T>
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
		MEMORY_TAG(MemoryCategory::ResourceLoading);
		_resource->SetStatus(ResourceStatus::Loading);
		_resource->load(this);
		_resource->SetStatus(ResourceStatus::Loaded);

		if (_complete)
		{
			_complete(_resource);
		}
	}
};
