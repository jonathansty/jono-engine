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

ID3D11Device* s_Device;

std::array<GraphicsResourceHandle, static_cast<u32>(DepthStencilState::Num)> s_DepthStencilStates;
std::array<GraphicsResourceHandle, static_cast<u32>(BlendState::Num)> s_BlendStates;
std::array<GraphicsResourceHandle, static_cast<u32>(RasterizerState::Num)> s_RasterStates;
std::array<GraphicsResourceHandle, static_cast<u32>(SamplerState::Num)> s_SamplerStates;

std::shared_ptr<Shader> s_ErrorPS = nullptr;
std::shared_ptr<Shader> s_ErrorVS = nullptr;

void CreateDSS(CD3D11_DEPTH_STENCIL_DESC const& ds_desc, DepthStencilState state);
void CreateRSS(CD3D11_RASTERIZER_DESC const& rs_desc, RasterizerState state);
void CreateSS(CD3D11_SAMPLER_DESC const& ss_desc, SamplerState state);
void CreateBS(CD3D11_BLEND_DESC const& bs_desc, BlendState state);

bool s_EnableShadowRendering = false; // Disabled for now because it's pretty broken :(
bool s_EnableCSM0 = true;
bool s_EnableCSM1 = true;
bool s_EnableCSM2 = true;
bool s_EnableCSM3 = true;

void init()
{
	ShaderCache::create();

	// Create error shaders
	{
		ShaderCreateParams parameters{};
		parameters.path = "Source/Engine/Shaders/error_px.hlsl";
		parameters.params.entry_point = "main";
		parameters.params.flags = ShaderCompiler::CompilerFlags::CompileDebug;
		parameters.params.stage = ShaderStage::Pixel;
		parameters.params.defines.push_back({ "LIGHTING_MODEL", "LIGHTING_MODEL_BLINN_PHONG" });

		s_ErrorPS = ShaderCache::instance()->find_or_create(parameters);
		ASSERTMSG(s_ErrorPS, "Failed to create error shader (\"{}\")", parameters.path.c_str());

		parameters.path = "Source/Engine/Shaders/error_vx.hlsl";
		parameters.params.stage = ShaderStage::Vertex;
		s_ErrorVS = ShaderCache::instance()->find_or_create(parameters);
		ASSERTMSG(s_ErrorVS, "Failed to create error shader (\"{}\")", parameters.path.c_str());


	}

	// Depth Stencil
	{
		CD3D11_DEPTH_STENCIL_DESC ds_desc{ CD3D11_DEFAULT() };
		ds_desc.DepthFunc = D3D11_COMPARISON_GREATER_EQUAL;
		CreateDSS(ds_desc, DepthStencilState::GreaterEqual);

		ds_desc.DepthFunc = D3D11_COMPARISON_EQUAL;
		CreateDSS(ds_desc, DepthStencilState::Equal);

		ds_desc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
		CreateDSS(ds_desc, DepthStencilState::LessEqual);

		ds_desc.DepthFunc = D3D11_COMPARISON_NEVER;
		ds_desc.DepthEnable = false;
		ds_desc.StencilEnable = false;
		CreateDSS(ds_desc, DepthStencilState::NoDepth);
	}

	// Blend states
	{
		CD3D11_BLEND_DESC bs_desc{ CD3D11_DEFAULT() };
		CreateBS(bs_desc, BlendState::Default);

		bs_desc.IndependentBlendEnable = FALSE;
		bs_desc.RenderTarget[0].BlendEnable = true;
		bs_desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD; 
		bs_desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		bs_desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		bs_desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		bs_desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
		bs_desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		CreateBS(bs_desc, BlendState::AlphaBlend);

	}


	// Rasterizer states
	{
		CD3D11_RASTERIZER_DESC rs_desc{ CD3D11_DEFAULT() };
		rs_desc.CullMode = D3D11_CULL_NONE;
        CreateRSS(rs_desc, RasterizerState::CullNone);

		rs_desc.CullMode = D3D11_CULL_FRONT;
        CreateRSS(rs_desc, RasterizerState::CullFront);

		rs_desc.CullMode = D3D11_CULL_BACK;
        CreateRSS(rs_desc, RasterizerState::CullBack);
	}

	// Samplers
	{
		CD3D11_SAMPLER_DESC sampler{ CD3D11_DEFAULT() };
		sampler.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		sampler.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		sampler.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
        CreateSS(sampler, SamplerState::MinMagMip_Linear);

		sampler.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
        CreateSS(sampler, SamplerState::MinMagMip_Point);

		sampler.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		sampler.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
		sampler.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
		sampler.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
        sampler.BorderColor[0] = 1.0f;
        sampler.BorderColor[1] = 1.0f;
        sampler.BorderColor[2] = 1.0f;
        sampler.BorderColor[3] = 1.0f;
        CreateSS(sampler, SamplerState::MinMagMip_LinearClamp);

		sampler.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
        CreateSS(sampler, SamplerState::MinMagMip_PointClamp);

	}
}

void deinit()
{
	s_ErrorPS.reset();
	s_ErrorVS.reset();

	TextureHandle::deinit();
	ShaderCache::shutdown();

	auto clear_fn = [](GraphicsResourceHandle& ptr)
	{
        ptr = GraphicsResourceHandle::Invalid();
	};
	std::for_each(s_DepthStencilStates.begin(), s_DepthStencilStates.end(), clear_fn);
	std::for_each(s_BlendStates.begin(), s_BlendStates.end(), clear_fn);
	std::for_each(s_RasterStates.begin(), s_RasterStates.end(), clear_fn);
	std::for_each(s_SamplerStates.begin(), s_SamplerStates.end(), clear_fn);

}

GraphicsResourceHandle GetBlendState(BlendState blendState)
{
	return s_BlendStates[static_cast<u32>(blendState)];
}

GraphicsResourceHandle GetRasterizerState(RasterizerState rasterizerState)
{
	return s_RasterStates[*rasterizerState];
}

GraphicsResourceHandle GetDepthStencilState(DepthStencilState depthStencilState)
{
	return s_DepthStencilStates[*depthStencilState];
}

GraphicsResourceHandle GetSamplerState(SamplerState samplerState)
{
	return s_SamplerStates[*samplerState];
}

std::shared_ptr<Graphics::Shader> get_error_shader_px()
{
	return s_ErrorPS;
}

std::shared_ptr<Graphics::Shader> get_error_shader_vx()
{
	return s_ErrorVS;
}

void CreateDSS(CD3D11_DEPTH_STENCIL_DESC const& ds_desc, DepthStencilState state)
{
    s_DepthStencilStates[*state] = GetRI()->CreateDepthStencilState(ds_desc, DepthStencilStateToString(state));
}

void CreateRSS(CD3D11_RASTERIZER_DESC const& rs_desc, RasterizerState state)
{
    s_RasterStates[*state] = GetRI()->CreateRasterizerState(rs_desc, RasterizerStateToString(state));
}

void CreateSS(CD3D11_SAMPLER_DESC const& ss_desc, SamplerState state)
{
    s_SamplerStates[*state] = GetRI()->CreateSamplerState(ss_desc, SamplerStateToString(state));
}

void CreateBS(CD3D11_BLEND_DESC const& bs_desc, BlendState state)
{
    s_BlendStates[*state] = GetRI()->CreateBlendState(bs_desc, BlendStateToString(state));
}


} // namespace Graphics



const char* DepthStencilStateToString(DepthStencilState state)
{
	static const char* s_strings[*DepthStencilState::Num] = 
	{
		"GreaterEqual",
		"Equal",
		"LessEqual",
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
		"MinMagMip_Point",
        "MinMagMip_LinearClamp",
        "MinMagMip_PointClamp"

	};
	return s_strings[*state];

}


