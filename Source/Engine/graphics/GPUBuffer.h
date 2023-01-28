#pragma once

#include "Graphics/GraphicsResourceHandle.h"

enum class BufferUsage
{
	Default,
	Dynamic,
	Staging,
	Immutable
};

interface IGPUBuffer
{
	virtual void* map(ID3D11DeviceContext* ctx) = 0;
	virtual void unmap(ID3D11DeviceContext* ctx) = 0;

	virtual GraphicsResourceHandle get_buffer() = 0;

	virtual GraphicsResourceHandle const& get_srv() const
	{
		FAILMSG("GPU Buffer does not support SRVs");
        return GraphicsResourceHandle::Invalid();
	}

	virtual GraphicsResourceHandle const& get_uav() const 
	{
		FAILMSG("GPU Buffer does not support UAVs");
        return GraphicsResourceHandle::Invalid();
	}
};
