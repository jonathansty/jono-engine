#pragma once

#include <hlsl++.h>

namespace hlslpp_helpers
{

hlslpp::float3 to_euler(hlslpp::quaternion q);

}

namespace math
{
using namespace hlslpp;

// Computes the normal of a triangle by using the 3 vertices defined by it
// 
// This function could be used to calculate the normals of a vertex by calculating the normals of each face the vertex contributes to.
inline float3 compute_normal(float3 p0, float3 p1, float3 p2)
{
	float3 u = p1 - p0;
	float3 v = p2 - p0;

	float3 normal = cross(u, v);
	return hlslpp::normalize(normal);
}

// Inverse transpose is used to transform normal vectors properly when shearing and using non uniform matrices
inline float4x4 compute_inverse_transpose(float4x4 matrix)
{
	float4x4 result = matrix;
	result.f32_128_3[0] = 0.0f;
	result.f32_128_3[1] = 0.0f;
	result.f32_128_3[2] = 0.0f;
	result.f32_128_3[3] = 1.0f;

	return transpose(inverse(result));

}

}