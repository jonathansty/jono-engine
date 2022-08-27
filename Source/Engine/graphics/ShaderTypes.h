#pragma once

#include "Types.h"

// Shaders namespace contains utility functions to convert from and to predictable memory layouts for shaders
namespace Shaders
{
struct float2
{
	float2() = default;

	float2(hlslpp::float2 const& pos)
			: x(pos.x)
			, y(pos.y)
	{
	}

	operator hlslpp::float2()
	{
		return hlslpp::float2(x, y);
	}

	f32 x;
	f32 y;
};

struct float3
{
	float3() = default;

	float3(hlslpp::float3 const& pos)
			: x(pos.x)
			, y(pos.y)
			, z(pos.z)
	{
	}

	operator DirectX::XMFLOAT3()
	{
		return { x, y, z };
	}

	operator hlslpp::float3()
	{
		return hlslpp::float3(x, y, z);
	}

	f32 x;
	f32 y;
	f32 z;
};

struct float4
{
	float4() = default;

	float4(f32 x, f32 y, f32 z, f32 w)
			: x(x)
			, y(y)
			, z(z)
			, w(w)
	{
	}

	float4(hlslpp::float4 const& pos)
			: x(pos.x)
			, y(pos.y)
			, z(pos.z)
			, w(pos.w)
	{
	}

	operator hlslpp::float4()
	{
		return hlslpp::float4(x, y, z, w);
	}

	f32 x;
	f32 y;
	f32 z;
	f32 w;
};

struct float4x4
{
	float4x4() = default;

	float4x4(hlslpp::float4x4 const& mat)
	{
		hlslpp::store(mat, data);
	}

	operator hlslpp::float4x4()
	{
		hlslpp::float4x4 result;
		hlslpp::load(result, data);
		return result;
	}

	f32 data[16];
};

} // namespace Shaders
