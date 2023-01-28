#include "engine.pch.h"
#include "ConstantBuffer.h"

#include "Core/Logging.h"
#include "Graphics/RenderInterface.h"

std::unique_ptr<ConstantBuffer> ConstantBuffer::create(RenderInterface* ri, u32 size, bool cpu_write /*= false*/, BufferUsage usage /*= BufferUsage::Default*/, void* initialData /*= nullptr*/)
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


	GraphicsResourceHandle bufferHandle = ri->CreateBuffer(buff, initialData);
	if(!bufferHandle.IsValid())
	{
		LOG_ERROR(Graphics, "Failed to create a buffer ");
		return nullptr;
	}

	result->m_Resource = bufferHandle;
	result->_size = buff.ByteWidth;
	result->_cpu_writeable = cpu_write;
	result->_usage = usage;

	return result;
}

 ConstantBuffer::ConstantBuffer()
	:  _size(0)
	, _cpu_writeable(false)
{
}

 ConstantBuffer::~ConstantBuffer()
{
}

void* ConstantBuffer::map(ID3D11DeviceContext* ctx)
{
	return GetRI()->Map(m_Resource);
}

void ConstantBuffer::unmap(ID3D11DeviceContext* ctx)
{
    GetRI()->Unmap(m_Resource);
}
