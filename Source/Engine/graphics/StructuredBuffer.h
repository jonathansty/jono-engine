#pragma once

#include "GPUBuffer.h"

class GPUByteBuffer final : public IGPUBuffer
{
public:
	GPUByteBuffer()
			:_mapped()
			,_buffer(nullptr)
			, _srv(nullptr)
			,_uav(nullptr)
			,_size_bytes(0)
	{};
	~GPUByteBuffer(){};


	static std::unique_ptr<GPUByteBuffer> create(ID3D11Device* device, size_t buffer_size_bytes, bool cpu_write = false, BufferUsage usage = BufferUsage::Default);

	virtual ID3D11Buffer* get_buffer() { return _buffer.Get(); }

	virtual void* map(ID3D11DeviceContext* ctx) override;
	virtual void unmap(ID3D11DeviceContext* ctx) override;

	ComPtr<ID3D11ShaderResourceView> const& get_srv() const override { return _srv; }
	ComPtr<ID3D11UnorderedAccessView> const& get_uav() const override { return _uav; };

	private:
	D3D11_MAPPED_SUBRESOURCE _mapped;

	ComPtr<ID3D11Buffer> _buffer;
	ComPtr<ID3D11ShaderResourceView> _srv;
	ComPtr<ID3D11UnorderedAccessView> _uav;

	size_t _size_bytes;
};

class GPUStructuredBuffer final : public IGPUBuffer
{
public:

	GPUStructuredBuffer(){};
	~GPUStructuredBuffer() {}

	static std::unique_ptr<GPUStructuredBuffer> create(ID3D11Device* device, size_t struct_size_bytes, size_t element_count, bool cpu_write = false, BufferUsage usage = BufferUsage::Default);


	virtual ID3D11Buffer* get_buffer() { return _buffer.Get(); }

	virtual void*  map(ID3D11DeviceContext* ctx) override;
	virtual void unmap(ID3D11DeviceContext* ctx) override;

	ComPtr<ID3D11ShaderResourceView>  const& get_srv() const override { return _srv; }
	ComPtr<ID3D11UnorderedAccessView> const& get_uav() const override { return _uav; };


private:
	D3D11_MAPPED_SUBRESOURCE _mapped;

	ComPtr<ID3D11Buffer> _buffer;
	ComPtr<ID3D11ShaderResourceView>  _srv;
	ComPtr<ID3D11UnorderedAccessView> _uav;

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