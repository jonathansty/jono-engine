#include "Resource.h"
#include "engine.pch.h"

#include "Resource.h"

Resource::Resource()
  : m_Status(ResourceStatus::Error)
{
}

Resource::~Resource()
{
}

ResourceStatus Resource::get_status() const
{
	return m_Status;
}

void Resource::SetStatus(ResourceStatus status)
{
	if (status == m_Status)
	{
		return;
	}

	m_Status = status;
	if (status == ResourceStatus::Loaded)
	{
		m_LoadedCV.notify_all();
	}
}

void Resource::WaitForLoad()
{
	if (m_Status != ResourceStatus::Loaded)
	{
		std::unique_lock<std::mutex> lk{ m_LoadedMtx };
		m_LoadedCV.wait(lk);
	}
}
