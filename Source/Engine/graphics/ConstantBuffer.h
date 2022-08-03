#pragma once
class ConstantBuffer
{
public:
	enum class BufferUsage
	{
		Default,
		Dynamic,
		Staging,
		Immutable
	};

	static std::shared_ptr<ConstantBuffer> create(ID3D11Device* device, u32 size, bool cpu_write = false, BufferUsage usage = BufferUsage::Default, void* initialData = nullptr);

	ID3D11Buffer* Get() const { return _buffer.Get(); }

	void* map(ID3D11DeviceContext* ctx)
	{
		D3D11_MAPPED_SUBRESOURCE resource{};
		ctx->Map(_buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
		return resource.pData;
	}
	void unmap(ID3D11DeviceContext* ctx)
	{
		ctx->Unmap(_buffer.Get(), 0);
	}

	ConstantBuffer()
			: _buffer()
			, _size(0)
			, _cpu_writeable(false)
	{
	}

	~ConstantBuffer() {}

private:
	ComPtr<ID3D11Buffer> _buffer;
	u32 _size;
	bool _cpu_writeable;
	BufferUsage _usage;
};
using ConstantBufferRef = shared_ptr<ConstantBuffer>;
