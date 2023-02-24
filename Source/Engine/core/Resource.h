#pragma once

#include "ResourceTypes.h"

// template resource class
class Resource
{
public:
	friend class ResourceLoader;
	Resource();

	virtual ~Resource();

	ResourceStatus get_status() const;

	bool is_loaded()
	{
		return m_Status == ResourceStatus::Loaded;
	}

	virtual bool load(enki::ITaskSet* parent) = 0;

	virtual std::string get_name() const { return ""; }

	void SetStatus(ResourceStatus status);

	void WaitForLoad();

private:
	std::atomic<ResourceStatus> m_Status;

	std::mutex m_LoadedMtx;
	std::condition_variable m_LoadedCV;

	friend class ResourceLoader;
	friend struct CompletionActionMarkLoaded;
};
