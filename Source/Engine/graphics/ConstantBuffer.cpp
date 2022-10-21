#include "engine.pch.h"
#include "ConstantBuffer.h"

std::unique_ptr<ConstantBuffer> ConstantBuffer::create(ID3D11Device* device, u32 size, bool cpu_write /*= false*/, BufferUsage usage /*= BufferUsage::Default*/, void* initialData /*= nullptr*/)
{
	std::unique_ptr<ConstantBuffer> result = std::make_unique<ConstantBuffer>();

	D3D11_BUFFER_DESC buff{};
	buff.ByteWidth = (1 + (size - 1) / 16) * 16;
	buff.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

	if (cpu_write)
	{
		buff.CPUAccessFlags |= D3D11_CPU_ACCESS_WRITE;
	}

	buff.Usage = D3D11_USAGE_DEFAULT;
	switch (usage)
	{
		case BufferUsage::Dynamic:
			buff.Usage = D3D11_USAGE_DYNAMIC;
			break;
		case BufferUsage::Staging:
			buff.Usage = D3D11_USAGE_STAGING;
			break;
		case BufferUsage::Immutable:
			buff.Usage = D3D11_USAGE_IMMUTABLE;
			break;
	}
	buff.StructureByteStride = 0;
	buff.MiscFlags = 0;


	ComPtr<ID3D11Buffer> b;
	if (initialData)
	{
		D3D11_SUBRESOURCE_DATA data{};
		data.pSysMem = initialData;
		ENSURE_HR(device->CreateBuffer(&buff, &data, &b));
	}
	else
	{
		ENSURE_HR(device->CreateBuffer(&buff, nullptr, &b));
	}
	result->_buffer = b;
	result->_size = buff.ByteWidth;
	result->_cpu_writeable = cpu_write;
	result->_usage = usage;

	return result;
}

 ConstantBuffer::ConstantBuffer()
	: _buffer()
	, _size(0)
	, _cpu_writeable(false)
{
}

 ConstantBuffer::~ConstantBuffer()
{
}

void* ConstantBuffer::map(ID3D11DeviceContext* ctx)
{
	D3D11_MAPPED_SUBRESOURCE resource{};
	ctx->Map(_buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
	return resource.pData;
}

void ConstantBuffer::unmap(ID3D11DeviceContext* ctx)
{
	ctx->Unmap(_buffer.Get(), 0);
}
