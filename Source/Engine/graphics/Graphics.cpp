#include "engine.pch.h"

#include "Graphics.h"

namespace Graphics {

ComPtr<ID3D11Device> s_device;
ComPtr<ID3D11DeviceContext> s_ctx;

std::array<ComPtr<ID3D11DepthStencilState>, DepthStencilState::Num> s_depth_stencil_states;
std::array<ComPtr<ID3D11BlendState>, BlendState::Num> s_blend_states;
std::array<ComPtr<ID3D11RasterizerState>, RasterizerState::Num> s_raster_states;
std::array<ComPtr<ID3D11SamplerState>, SamplerState::Num> s_sampler_states;

void init(ComPtr<ID3D11Device> device, ComPtr<ID3D11DeviceContext> ctx) {

	assert(!s_device);

	s_device = device;
	s_ctx = ctx;

	// Depth Stencil
	{
		CD3D11_DEPTH_STENCIL_DESC ds_desc{ CD3D11_DEFAULT() };
		ds_desc.DepthFunc = D3D11_COMPARISON_GREATER_EQUAL;
		SUCCEEDED(device->CreateDepthStencilState(&ds_desc, s_depth_stencil_states[DepthStencilState::GreaterEqual].GetAddressOf()));
		helpers::SetDebugObjectName(s_depth_stencil_states[DepthStencilState::GreaterEqual].Get(), "GreaterEqual");

		ds_desc.DepthFunc = D3D11_COMPARISON_EQUAL;
		SUCCEEDED(device->CreateDepthStencilState(&ds_desc, s_depth_stencil_states[DepthStencilState::Equal].GetAddressOf()));
		helpers::SetDebugObjectName(s_depth_stencil_states[DepthStencilState::Equal].Get(), "Equal");

		ds_desc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
		SUCCEEDED(device->CreateDepthStencilState(&ds_desc, s_depth_stencil_states[DepthStencilState::LessEqual].GetAddressOf()));
		helpers::SetDebugObjectName(s_depth_stencil_states[DepthStencilState::LessEqual].Get(), "LessEqual");

	}

	// Blend states
	{
		CD3D11_BLEND_DESC bs_desc{ CD3D11_DEFAULT() };
		SUCCEEDED(device->CreateBlendState(&bs_desc, s_blend_states[0].GetAddressOf()));
		helpers::SetDebugObjectName(s_blend_states[BlendState::Default].Get(), "Default");
	}

	// Rasterizer states
	{
		CD3D11_RASTERIZER_DESC rs_desc{ CD3D11_DEFAULT() };
		rs_desc.CullMode = D3D11_CULL_NONE;
		SUCCEEDED(device->CreateRasterizerState(&rs_desc, s_raster_states[RasterizerState::CullNone].GetAddressOf()));
		helpers::SetDebugObjectName(s_raster_states[RasterizerState::CullNone].Get(), "CullNone");

		rs_desc.CullMode = D3D11_CULL_FRONT;
		SUCCEEDED(device->CreateRasterizerState(&rs_desc, s_raster_states[RasterizerState::CullFront].GetAddressOf()));
		helpers::SetDebugObjectName(s_raster_states[RasterizerState::CullFront].Get(), "CullFront");

		rs_desc.CullMode = D3D11_CULL_BACK;
		SUCCEEDED(device->CreateRasterizerState(&rs_desc, s_raster_states[RasterizerState::CullBack].GetAddressOf()));
		helpers::SetDebugObjectName(s_raster_states[RasterizerState::CullBack].Get(), "CullBack");
	}

	// Samplers
	{
		CD3D11_SAMPLER_DESC sampler{ CD3D11_DEFAULT() };
		sampler.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		SUCCEEDED(device->CreateSamplerState(&sampler, s_sampler_states[SamplerState::MinMagMip_Linear].GetAddressOf()));
	}
}

void deinit() {
	auto clear_fn = []<typename T>(T& ptr) {
		ptr.Reset();
	};
	std::for_each(s_depth_stencil_states.begin(), s_depth_stencil_states.end(), clear_fn);
	std::for_each(s_blend_states.begin(), s_blend_states.end(), clear_fn);
	std::for_each(s_raster_states.begin(), s_raster_states.end(), clear_fn);
	std::for_each(s_sampler_states.begin(), s_sampler_states.end(), clear_fn);
	s_device.Reset();

}

ComPtr<ID3D11Device> get_device() {
	return s_device;
}

ComPtr<ID3D11BlendState> get_blend_state(BlendState::Value blendState) {
	return s_blend_states[blendState];
}

ComPtr<ID3D11RasterizerState> get_rasterizer_state(RasterizerState::Value rasterizerState) {
	return s_raster_states[rasterizerState];
}

ComPtr<ID3D11DepthStencilState> get_depth_stencil_state(DepthStencilState::Value depthStencilState) {
	return s_depth_stencil_states[depthStencilState];
}

ComPtr<ID3D11SamplerState> get_sampler_state(SamplerState::Value samplerState) {
	return s_sampler_states[samplerState];
}

} // namespace Graphics

std::shared_ptr<ConstantBuffer> ConstantBuffer::create(ID3D11Device* device, u32 size, bool cpu_write /*= false*/, BufferUsage usage /*= BufferUsage::Default*/, void* initialData /*= nullptr*/) {
	std::shared_ptr<ConstantBuffer> result = std::make_shared<ConstantBuffer>();

	D3D11_BUFFER_DESC buff{};
	buff.ByteWidth = size;
	buff.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

	if (cpu_write)
		buff.CPUAccessFlags |= D3D11_CPU_ACCESS_WRITE;

	buff.Usage = D3D11_USAGE_DEFAULT;
	switch (usage) {
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
	if(initialData)
		SUCCEEDED(device->CreateBuffer(&buff, &data, &b));
	else
		SUCCEEDED(device->CreateBuffer(&buff, nullptr, &b));
	result->_buffer = b;
	result->_size = size;
	result->_cpu_writeable = cpu_write;
	result->_usage = usage;

	return result;
}
