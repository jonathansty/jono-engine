#pragma once

#include "GPUBuffer.h"

class ConstantBuffer final : public IGPUBuffer
{
public:
	static std::unique_ptr<ConstantBuffer> create(ID3D11Device* device, u32 size, bool cpu_write = false, BufferUsage usage = BufferUsage::Default, void* initialData = nullptr);

	ConstantBuffer();

	~ConstantBuffer();


	ID3D11Buffer* Get() { return _buffer.Get(); }

	ID3D11Buffer* get_buffer() override { return _buffer.Get(); }

	void* map(ID3D11DeviceContext* ctx) override;
	void unmap(ID3D11DeviceContext* ctx) override;





private:
	ComPtr<ID3D11Buffer> _buffer;
	u32 _size;
	bool _cpu_writeable;
	BufferUsage _usage;
};
using ConstantBufferRef = shared_ptr<ConstantBuffer>;
