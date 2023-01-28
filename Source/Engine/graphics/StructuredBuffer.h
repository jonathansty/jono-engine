#pragma once

#include "GPUBuffer.h"
#include "Graphics/RenderInterface.h"

class GPUByteBuffer final : public IGPUBuffer
{
public:
	GPUByteBuffer()
			:_mapped()
			,_buffer()
			, _srv()
			,_uav()
			,_size_bytes(0)
	{};
	~GPUByteBuffer(){};


	static std::unique_ptr<GPUByteBuffer> create(RenderInterface* device, size_t buffer_size_bytes, bool cpu_write = false, BufferUsage usage = BufferUsage::Default);

	virtual GraphicsResourceHandle get_buffer() { return _buffer; }

	virtual void* map(ID3D11DeviceContext* ctx) override;
	virtual void unmap(ID3D11DeviceContext* ctx) override;

	GraphicsResourceHandle const& get_srv() const override { return _srv; }
    GraphicsResourceHandle  const& get_uav() const override { return _uav; };

	private:
	D3D11_MAPPED_SUBRESOURCE _mapped;

	GraphicsResourceHandle _buffer;
	GraphicsResourceHandle _srv;
	GraphicsResourceHandle _uav;

	size_t _size_bytes;
};

class GPUStructuredBuffer final : public IGPUBuffer
{
public:

	GPUStructuredBuffer(){};
	~GPUStructuredBuffer() {}

	static std::unique_ptr<GPUStructuredBuffer> create(RenderInterface* device, size_t struct_size_bytes, size_t element_count, bool cpu_write = false, BufferUsage usage = BufferUsage::Default);


	virtual GraphicsResourceHandle get_buffer() { return _buffer; }

	virtual void*  map(ID3D11DeviceContext* ctx) override;
	virtual void unmap(ID3D11DeviceContext* ctx) override;

	GraphicsResourceHandle const& get_srv() const override { return _srv; }
	GraphicsResourceHandle const& get_uav() const override { return _uav; };


private:
	D3D11_MAPPED_SUBRESOURCE _mapped;

	GraphicsResourceHandle _buffer;
	GraphicsResourceHandle _srv;
	GraphicsResourceHandle _uav;

	size_t _size_bytes;
	size_t _struct_size_bytes;

};

class ScopedBufferAccess final
{
public:
	explicit ScopedBufferAccess(ID3D11DeviceContext* ctx, IGPUBuffer* buffer)
		: _buffer(buffer)
		, _ctx(ctx)
	{
		ptr = _buffer->map(_ctx);
	}

	~ScopedBufferAccess()
	{
		_buffer->unmap(_ctx);
	}

	void* get_ptr() const { return ptr; }

private:
	void* ptr = nullptr; 

	IGPUBuffer* _buffer;
	ID3D11DeviceContext* _ctx;

};