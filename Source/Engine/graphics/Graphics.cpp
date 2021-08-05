#include "engine.pch.h"

#include "Graphics.h"

namespace Graphics {

ComPtr<ID3D11Device> s_Device;

std::array<ComPtr<ID3D11DepthStencilState>, DepthStencilState::Num> s_DepthStencilStates;
std::array<ComPtr<ID3D11BlendState>, BlendState::Num> s_BlendStates;
std::array<ComPtr<ID3D11RasterizerState>, RasterizerState::Num> s_RasterizerStates;
std::array<ComPtr<ID3D11SamplerState>, SamplerState::Num> s_SamplerStates;

void init(ComPtr<ID3D11Device> device) {

	assert(!s_Device);

	s_Device = device;

	// Depth Stencil
	{
		CD3D11_DEPTH_STENCIL_DESC ds_desc{ CD3D11_DEFAULT() };
		ds_desc.DepthFunc = D3D11_COMPARISON_GREATER_EQUAL;
		SUCCEEDED(device->CreateDepthStencilState(&ds_desc, s_DepthStencilStates[0].GetAddressOf()));
		helpers::SetDebugObjectName(s_DepthStencilStates[DepthStencilState::GreaterEqual].Get(), "GreaterEqual");
	}

	// Blend states
	{
		CD3D11_BLEND_DESC bs_desc{ CD3D11_DEFAULT() };
		SUCCEEDED(device->CreateBlendState(&bs_desc, s_BlendStates[0].GetAddressOf()));
		helpers::SetDebugObjectName(s_BlendStates[BlendState::Default].Get(), "Default");
	}

	// Rasterizer states
	{
		CD3D11_RASTERIZER_DESC rs_desc{ CD3D11_DEFAULT() };
		rs_desc.CullMode = D3D11_CULL_NONE;
		SUCCEEDED(device->CreateRasterizerState(&rs_desc, s_RasterizerStates[RasterizerState::CullNone].GetAddressOf()));
		helpers::SetDebugObjectName(s_RasterizerStates[RasterizerState::CullNone].Get(), "CullNone");

		rs_desc.CullMode = D3D11_CULL_FRONT;
		SUCCEEDED(device->CreateRasterizerState(&rs_desc, s_RasterizerStates[RasterizerState::CullFront].GetAddressOf()));
		helpers::SetDebugObjectName(s_RasterizerStates[RasterizerState::CullFront].Get(), "CullFront");

		rs_desc.CullMode = D3D11_CULL_BACK;
		SUCCEEDED(device->CreateRasterizerState(&rs_desc, s_RasterizerStates[RasterizerState::CullBack].GetAddressOf()));
		helpers::SetDebugObjectName(s_RasterizerStates[RasterizerState::CullBack].Get(), "CullBack");
	}

	// Samplers
	{
		CD3D11_SAMPLER_DESC sampler{ CD3D11_DEFAULT() };
		sampler.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		SUCCEEDED(device->CreateSamplerState(&sampler, s_SamplerStates[SamplerState::MinMagMip_Linear].GetAddressOf()));
	}
}

ComPtr<ID3D11Device> GetDevice() {
	return s_Device;
}

ComPtr<ID3D11BlendState> GetBlendState(BlendState::Value blendState) {
	return s_BlendStates[blendState];
}

ComPtr<ID3D11RasterizerState> GetRasterizerState(RasterizerState::Value rasterizerState) {
	return s_RasterizerStates[rasterizerState];
}

ComPtr<ID3D11DepthStencilState> GetDepthStencilState(DepthStencilState::Value depthStencilState) {
	return s_DepthStencilStates[depthStencilState];
}

ComPtr<ID3D11SamplerState> GetSamplerState(SamplerState::Value samplerState) {
	return s_SamplerStates[samplerState];
}

} // namespace Graphics
