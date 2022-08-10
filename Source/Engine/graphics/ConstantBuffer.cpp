#include "engine.pch.h"
#include "ConstantBuffer.h"

std::shared_ptr<ConstantBuffer> ConstantBuffer::create(ID3D11Device* device, u32 size, bool cpu_write /*= false*/, BufferUsage usage /*= BufferUsage::Default*/, void* initialData /*= nullptr*/)
{
	std::shared_ptr<ConstantBuffer> result = std::make_shared<ConstantBuffer>();

	D3D11_BUFFER_DESC buff{};
	buff.ByteWidth = size;
	buff.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

	if (cpu_write)
		buff.CPUAccessFlags |= D3D11_CPU_ACCESS_WRITE;

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

	D3D11_SUBRESOURCE_DATA data{};
	data.pSysMem = initialData;

	ComPtr<ID3D11Buffer> b;
	if (initialData)
		SUCCEEDED(device->CreateBuffer(&buff, &data, &b));
	else
		SUCCEEDED(device->CreateBuffer(&buff, nullptr, &b));
	result->_buffer = b;
	result->_size = size;
	result->_cpu_writeable = cpu_write;
	result->_usage = usage;

	return result;
}
