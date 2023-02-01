#pragma once

#include "GPUBuffer.h"
#include "Graphics/RenderInterface.h"

class ConstantBuffer final : public IGPUBuffer
{
public:
	static std::unique_ptr<ConstantBuffer> create(RenderInterface* device, u32 size, bool cpu_write = false, BufferUsage usage = BufferUsage::Default, void* initialData = nullptr);

	ConstantBuffer();

	virtual ~ConstantBuffer();


	virtual GraphicsResourceHandle get_buffer() { return m_Resource; }

	void* map(RenderContext& ctx) override;
	void unmap(RenderContext& ctx) override;

private:
    GraphicsResourceHandle m_Resource;
	u32 _size;
	bool _cpu_writeable;
	BufferUsage _usage;
};
using ConstantBufferRef = shared_ptr<ConstantBuffer>;
