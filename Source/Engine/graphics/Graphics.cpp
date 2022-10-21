#include "engine.pch.h"

#include "Graphics.h"
#include "Core/ModelResource.h"
#include "Core/TextureResource.h"
#include "Shader.h"
#include "ShaderCache.h"
#include "Renderer.h"

#include "RendererDebug.h"


namespace Graphics
{

DeviceContext s_ctx;

std::array<ComPtr<ID3D11DepthStencilState>, static_cast<u32>(DepthStencilState::Num)> s_depth_stencil_states;
std::array<ComPtr<ID3D11BlendState>, static_cast<u32>(BlendState::Num)> s_blend_states;
std::array<ComPtr<ID3D11RasterizerState>, static_cast<u32>(RasterizerState::Num)> s_raster_states;
std::array<ComPtr<ID3D11SamplerState>, static_cast<u32>(SamplerState::Num)> s_sampler_states;

std::shared_ptr<Shader> s_error_pixel_shader = nullptr;
std::shared_ptr<Shader> s_error_vertex_shader = nullptr;

void CreateDSS(ComPtr<ID3D11Device> const& device, CD3D11_DEPTH_STENCIL_DESC const& ds_desc, DepthStencilState state);
void CreateRSS(ComPtr<ID3D11Device> const& device, CD3D11_RASTERIZER_DESC const& rs_desc, RasterizerState state);
void CreateSS(ComPtr<ID3D11Device> const& device, CD3D11_SAMPLER_DESC const& ss_desc, SamplerState state);
void CreateBS(ComPtr<ID3D11Device> const& device, CD3D11_BLEND_DESC const& bs_desc, BlendState state);


bool s_EnableShadowRendering = false;

void init(DeviceContext const& ctx)
{
	assert(ctx._device.Get() && ctx._ctx.Get());
	s_ctx = ctx;

	auto device = s_ctx._device;

	ShaderCache::create();

	// Create error shaders
	{
		ShaderCreateParams parameters{};
		parameters.path = "Source/Engine/Shaders/error_px.hlsl";
		parameters.params.entry_point = "main";
		parameters.params.flags = ShaderCompiler::CompilerFlags::CompileDebug;
		parameters.params.stage = ShaderType::Pixel;
		parameters.params.defines.push_back({ "LIGHTING_MODEL", "LIGHTING_MODEL_BLINN_PHONG" });

		s_error_pixel_shader = ShaderCache::instance()->find_or_create(parameters);
		ASSERTMSG(s_error_pixel_shader, "Failed to create error shader (\"{}\")", parameters.path.c_str());

		parameters.path = "Source/Engine/Shaders/error_vx.hlsl";
		parameters.params.stage = ShaderType::Vertex;
		s_error_vertex_shader = ShaderCache::instance()->find_or_create(parameters);
		ASSERTMSG(s_error_vertex_shader, "Failed to create error shader (\"{}\")", parameters.path.c_str());


	}

	// Depth Stencil
	{
		CD3D11_DEPTH_STENCIL_DESC ds_desc{ CD3D11_DEFAULT() };
		ds_desc.DepthFunc = D3D11_COMPARISON_GREATER_EQUAL;
		CreateDSS(device, ds_desc, DepthStencilState::GreaterEqual);

		ds_desc.DepthFunc = D3D11_COMPARISON_EQUAL;
		CreateDSS(device, ds_desc, DepthStencilState::Equal);

		ds_desc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
		CreateDSS(device, ds_desc, DepthStencilState::LessEqual);

		ds_desc.DepthFunc = D3D11_COMPARISON_NEVER;
		ds_desc.DepthEnable = false;
		ds_desc.StencilEnable = false;
		CreateDSS(device, ds_desc, DepthStencilState::NoDepth);
	}

	// Blend states
	{
		CD3D11_BLEND_DESC bs_desc{ CD3D11_DEFAULT() };
		CreateBS(device, bs_desc, BlendState::Default);

		bs_desc.IndependentBlendEnable = FALSE;
		bs_desc.RenderTarget[0].BlendEnable = true;
		bs_desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD; 
		bs_desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		bs_desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		bs_desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		bs_desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
		bs_desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		CreateBS(device, bs_desc, BlendState::AlphaBlend);

	}


	// Rasterizer states
	{
		CD3D11_RASTERIZER_DESC rs_desc{ CD3D11_DEFAULT() };
		rs_desc.CullMode = D3D11_CULL_NONE;
		SUCCEEDED(device->CreateRasterizerState(&rs_desc, s_raster_states[*RasterizerState::CullNone].GetAddressOf()));
		Helpers::SetDebugObjectName(s_raster_states[*RasterizerState::CullNone].Get(), "CullNone");

		rs_desc.CullMode = D3D11_CULL_FRONT;
		SUCCEEDED(device->CreateRasterizerState(&rs_desc, s_raster_states[*RasterizerState::CullFront].GetAddressOf()));
		Helpers::SetDebugObjectName(s_raster_states[*RasterizerState::CullFront].Get(), "CullFront");

		rs_desc.CullMode = D3D11_CULL_BACK;
		SUCCEEDED(device->CreateRasterizerState(&rs_desc, s_raster_states[*RasterizerState::CullBack].GetAddressOf()));
		Helpers::SetDebugObjectName(s_raster_states[*RasterizerState::CullBack].Get(), "CullBack");
	}

	// Samplers
	{
		CD3D11_SAMPLER_DESC sampler{ CD3D11_DEFAULT() };
		sampler.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		sampler.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		sampler.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		SUCCEEDED(device->CreateSamplerState(&sampler, s_sampler_states[*SamplerState::MinMagMip_Linear].GetAddressOf()));

		sampler.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
		SUCCEEDED(device->CreateSamplerState(&sampler, s_sampler_states[*SamplerState::MinMagMip_Point].GetAddressOf()));
	}
}

void deinit()
{
	s_error_pixel_shader.reset();
	s_error_vertex_shader.reset();

	TextureHandle::deinit();
	ShaderCache::shutdown();

	auto clear_fn = []<typename T>(T& ptr)
	{
		ptr.Reset();
	};
	std::for_each(s_depth_stencil_states.begin(), s_depth_stencil_states.end(), clear_fn);
	std::for_each(s_blend_states.begin(), s_blend_states.end(), clear_fn);
	std::for_each(s_raster_states.begin(), s_raster_states.end(), clear_fn);
	std::for_each(s_sampler_states.begin(), s_sampler_states.end(), clear_fn);

	s_ctx = {};
}

ComPtr<ID3D11Device> get_device()
{
	return s_ctx._device;
}

ComPtr<ID3D11DeviceContext> get_ctx()
{
	return s_ctx._ctx;
}

ComPtr<ID3D11BlendState> get_blend_state(BlendState blendState)
{
	return s_blend_states[static_cast<u32>(blendState)];
}

ComPtr<ID3D11RasterizerState> get_rasterizer_state(RasterizerState rasterizerState)
{
	return s_raster_states[*rasterizerState];
}

ComPtr<ID3D11DepthStencilState> get_depth_stencil_state(DepthStencilState depthStencilState)
{
	return s_depth_stencil_states[*depthStencilState];
}

ComPtr<ID3D11SamplerState> get_sampler_state(SamplerState samplerState)
{
	return s_sampler_states[*samplerState];
}

std::shared_ptr<Graphics::Shader> get_error_shader_px()
{
	return s_error_pixel_shader;
}

std::shared_ptr<Graphics::Shader> get_error_shader_vx()
{
	return s_error_vertex_shader;
}

void CreateDSS(ComPtr<ID3D11Device> const& device, CD3D11_DEPTH_STENCIL_DESC const& ds_desc, DepthStencilState state)
{
	SUCCEEDED(device->CreateDepthStencilState(&ds_desc, s_depth_stencil_states[*state].GetAddressOf()));
	Helpers::SetDebugObjectName(s_depth_stencil_states[*state].Get(), DepthStencilStateToString(state));
}

void CreateRSS(ComPtr<ID3D11Device> const& device, CD3D11_RASTERIZER_DESC const& rs_desc, RasterizerState state)
{
	SUCCEEDED(device->CreateRasterizerState(&rs_desc, s_raster_states[*state].GetAddressOf()));
	Helpers::SetDebugObjectName(s_raster_states[*state].Get(), RasterizerStateToString(state));
}

void CreateSS(ComPtr<ID3D11Device> const& device, CD3D11_SAMPLER_DESC const& ss_desc, SamplerState state)
{
	SUCCEEDED(device->CreateSamplerState(&ss_desc, s_sampler_states[*state].GetAddressOf()));
	Helpers::SetDebugObjectName(s_sampler_states[*state].Get(), SamplerStateToString(state));
}

void CreateBS(ComPtr<ID3D11Device> const& device, CD3D11_BLEND_DESC const& bs_desc, BlendState state)
{
	SUCCEEDED(device->CreateBlendState(&bs_desc, s_blend_states[*state].GetAddressOf()));
	Helpers::SetDebugObjectName(s_blend_states[*state].Get(), BlendStateToString(state));
}


} // namespace Graphics



const char* DepthStencilStateToString(DepthStencilState state)
{
	static const char* s_strings[*DepthStencilState::Num] = 
	{
		"Default",
		"GreaterEqual",
		"Equal",
		"LessEqual"
		"NoDepth"
	};

	return s_strings[*state];
}

const char* BlendStateToString(BlendState state)
{
	static const char* s_strings[*BlendState::Num] = {
		"Default",
		"AlphaBlend"
	};
	return s_strings[*state];
}

const char* RasterizerStateToString(RasterizerState state)
{
	static const char* s_strings[*RasterizerState::Num] = {
		"CullNone",
		"CullFront",
		"CullBack"
	};
	return s_strings[*state];
}

const char* SamplerStateToString(SamplerState state)
{
	static const char* s_strings[*SamplerState::Num] = {
		"MinMagMip_Linear",
		"MinMagMip_Point"
	};
	return s_strings[*state];

}


