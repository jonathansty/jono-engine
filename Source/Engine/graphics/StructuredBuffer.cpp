#include "engine.pch.h"
#include "Graphics.h"
#include "StructuredBuffer.h"

std::unique_ptr<StructuredBuffer> StructuredBuffer::create(ID3D11Device* device, size_t struct_size_bytes, size_t element_count, bool cpu_write, BufferUsage buffer_usage)
{
	// #TODO: Expose bind flags as an API to create srv or uav
	u32 bind_flags = D3D11_BIND_SHADER_RESOURCE;
	if(!cpu_write)
	{
		bind_flags |= D3D11_BIND_UNORDERED_ACCESS;
	}

	D3D11_USAGE usage = D3D11_USAGE_DEFAULT;
	switch(buffer_usage)
	{
		case BufferUsage::Dynamic:
			usage = D3D11_USAGE_DYNAMIC;
			break;
		case BufferUsage::Staging:
			usage = D3D11_USAGE_STAGING;
			break;
		case BufferUsage::Immutable:
			usage = D3D11_USAGE_IMMUTABLE;
			break;
	}

	u32 write_flags = 0;
	if(cpu_write)
	{
		write_flags |= D3D11_CPU_ACCESS_WRITE;
	}

	CD3D11_BUFFER_DESC desc = CD3D11_BUFFER_DESC(UINT(struct_size_bytes * element_count), bind_flags, usage, write_flags, D3D11_RESOURCE_MISC_BUFFER_STRUCTURED, UINT(struct_size_bytes));

	unique_ptr<StructuredBuffer> result = std::make_unique<StructuredBuffer>();
	ENSURE_HR(device->CreateBuffer(&desc, nullptr, result->_buffer.GetAddressOf()));

	if(bind_flags & D3D11_BIND_SHADER_RESOURCE)
	{
		CD3D11_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
		srv_desc.Format = DXGI_FORMAT_UNKNOWN;
		srv_desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
		srv_desc.Buffer.FirstElement = 0;
		srv_desc.Buffer.NumElements = UINT(element_count);
		ENSURE_HR(device->CreateShaderResourceView(result->_buffer.Get(), &srv_desc, result->_srv.GetAddressOf()));
	}

	if(bind_flags & D3D11_BIND_UNORDERED_ACCESS)
	{
		// Can not support UAV with cpu write enabled 
		ASSERT(!cpu_write);

		CD3D11_UNORDERED_ACCESS_VIEW_DESC uav_desc{};
		uav_desc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
		uav_desc.Buffer.FirstElement = 0;
		uav_desc.Buffer.NumElements = UINT(element_count);
		ENSURE_HR(device->CreateUnorderedAccessView(result->_buffer.Get(), nullptr, result->_uav.GetAddressOf()));
	}

	return result;
}

void* StructuredBuffer::map(ID3D11DeviceContext* ctx)
{
	ctx->Map(_buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &_mapped);
	return _mapped.pData;
}

void StructuredBuffer::unmap(ID3D11DeviceContext* ctx)
{
	ctx->Unmap(_buffer.Get(), 0);
	_mapped = {};
}
