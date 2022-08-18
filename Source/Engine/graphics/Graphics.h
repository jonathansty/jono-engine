#pragma once

#include "PrecisionTimer.h"
#include "ShaderCompiler.h"
#include "singleton.h"

enum class DepthStencilState : u32
{
	Default,
	GreaterEqual = Default,
	Equal,
	LessEqual,
	NoDepth,
	Num
};
ENUM_UNDERLYING_TYPE(DepthStencilState);
const char* DepthStencilStateToString(DepthStencilState state);

enum class BlendState : u32
{
	Default,
	AlphaBlend,
	Num
};
ENUM_UNDERLYING_TYPE(BlendState);
const char* BlendStateToString(BlendState state);


enum class RasterizerState : u32
{
	Default,
	CullNone = Default,
	CullFront,
	CullBack,
	Num
};
ENUM_UNDERLYING_TYPE(RasterizerState);
const char* RasterizerStateToString(RasterizerState state);



enum class SamplerState : u32
{
	MinMagMip_Linear,
	MinMagMip_Point,
	Num
};
ENUM_UNDERLYING_TYPE(SamplerState);
const char* SamplerStateToString(SamplerState state);



namespace Graphics
{
	struct ShaderCreateParams;
}


namespace Graphics
{

struct DeviceContext;

// Entry point for the graphics. Initializes default D3D11 objects for usage later
void init(DeviceContext const& ctx);

// On shutdown the application should call this to release all handles to the device, context and the common states.
void deinit();

// Public API to retrieve the currently initialized graphics data and common states
ComPtr<ID3D11Device> get_device();
ComPtr<ID3D11DeviceContext> get_ctx();
ComPtr<ID3D11BlendState> get_blend_state(BlendState blendState);
ComPtr<ID3D11RasterizerState> get_rasterizer_state(RasterizerState rasterizerState);
ComPtr<ID3D11DepthStencilState> get_depth_stencil_state(DepthStencilState blendState);
ComPtr<ID3D11SamplerState> get_sampler_state(SamplerState blendState);

std::shared_ptr<class Shader> get_error_shader_px();
std::shared_ptr<class Shader> get_error_shader_vx();

template <typename T>
HRESULT set_debug_name(T* obj, std::string const& n)
{
	return obj->SetPrivateData(WKPDID_D3DDebugObjectName, UINT(n.size()), n.data());
}



} // namespace Graphics

// Shaders namespace contains utility functions to convert from and to predictable memory layouts for shaders
namespace Shaders
{
	struct float2
	{
		float2() = default;

		float2(hlslpp::float2 const& pos)
				: _x(pos.x)
				, _y(pos.y)
		{
		}

		operator hlslpp::float2()
		{
			return hlslpp::float2(_x, _y);
		}

		f32 _x;
		f32 _y;
	};

	struct float3
	{
		float3() = default;

		float3(hlslpp::float3 const& pos)
			: _x(pos.x)
			, _y(pos.y)
			, _z(pos.z)
		{
		}

		operator hlslpp::float3()
		{
			return hlslpp::float3(_x, _y, _z);
		}


		f32 _x;
		f32 _y;
		f32 _z;
	};

	struct float4
	{
		float4() = default;

		float4(hlslpp::float4 const& pos)
				: _x(pos.x)
				, _y(pos.y)
				, _z(pos.z)
				, _w(pos.w)
		{
		}

		operator hlslpp::float4()
		{
			return hlslpp::float4(_x, _y, _z, _w);
		}

		f32 _x;
		f32 _y;
		f32 _z;
		f32 _w;
	};


	struct float4x4
	{
		float4x4() = default;

		float4x4(hlslpp::float4x4 const& mat)
		{
			hlslpp::store(mat, _data);
		}

		operator hlslpp::float4x4() 
		{
			hlslpp::float4x4 result;
			hlslpp::load(result, _data);
			return result;
		}


		f32 _data[16];
	};

} // namespace Shaders
