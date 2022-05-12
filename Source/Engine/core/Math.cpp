#include "engine.pch.h"
#include "Math.h"

namespace hlslpp_helpers
{
float3 to_euler(hlslpp::quaternion q)
{
	// https://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles
	float3 angles;

	// roll (x-axis rotation)
	float1 sinr_cosp = 2 * (q.w * q.x + q.y * q.z);
	float1 cosr_cosp = 1 - 2 * (q.x * q.x + q.y * q.y);
	angles.x = hlslpp::atan2(sinr_cosp, cosr_cosp);

	// pitch (y-axis rotation)
	float1 sinp = 2 * (q.w * q.y - q.z * q.x);
	if (hlslpp::all(hlslpp::abs(sinp) >= float1(1)))
		angles.y = std::copysign(3.1415f / 2, float(sinp)); // use 90 degrees if out of range
	else
		angles.y = hlslpp::asin(sinp);

	// yaw (z-axis rotation)
	float1 siny_cosp = 2 * (q.w * q.z + q.x * q.y);
	float1 cosy_cosp = 1 - 2 * (q.y * q.y + q.z * q.z);
	angles.z = hlslpp::atan2(siny_cosp, cosy_cosp);

	return angles;
}

} // namespace hlslpp_helpers

namespace Math
{

float3 compute_normal(float3 p0, float3 p1, float3 p2)
{
	float3 u = p1 - p0;
	float3 v = p2 - p0;

	float3 normal = cross(u, v);
	return hlslpp::normalize(normal);
}

float4x4 compute_inverse_transpose(float4x4 matrix)
{
	float4x4 result = matrix;
	result.f32_128_3[0] = 0.0f;
	result.f32_128_3[1] = 0.0f;
	result.f32_128_3[2] = 0.0f;
	result.f32_128_3[3] = 1.0f;

	return transpose(inverse(result));
}

void calculate_frustum(FrustumCorners& out_corners, f32 n, f32 f, f32 fov, f32 vFov)
{
	f32 tan_half_fov = tanf(fov / 2.0f);
	f32 tan_half_vfov = tanf(vFov / 2.0f);

	f32 x1 = n * tan_half_fov;
	f32 x2 = f * tan_half_fov;
	f32 y1 = n * tan_half_vfov;
	f32 y2 = f * tan_half_vfov;

	// Calculate the frustum in view space
	out_corners[0] = { x1, y1, n, 1.0 };
	out_corners[1] = { -x1, y1, n, 1.0 };
	out_corners[2] = { x1, -y1, n, 1.0 };
	out_corners[3] = { -x1, -y1, n, 1.0 };
	out_corners[4] = { x2, y2, f, 1.0 };
	out_corners[5] = { -x2, y2, f, 1.0 };
	out_corners[6] = { x2, -y2, f, 1.0 };
	out_corners[7] = { -x2, -y2, f, 1.0 };
}

void transform_frustum(FrustumCorners& corners, float4x4 matrix)
{
	for (u32 i = 0; i < corners.size(); ++i)
	{
		corners[i] = hlslpp::mul(corners[i], matrix);
	}
}

void calculate_frustum(FrustumCorners& out_corners, float4x4 cam_vp)
{
	f32 n = -1.0f;
	f32 f = 1.0f;
	f32 x1 = 1.0f;
	f32 x2 = 1.0f;
	f32 y1 = 1.0f;
	f32 y2 = 1.0f;

	// Calculate the frustum in view space
	out_corners[0] = { 1.0, 1.0, n, 1.0 };
	out_corners[1] = { -1.0, 1.0, n, 1.0 };
	out_corners[2] = { 1.0, -1.0, n, 1.0 };
	out_corners[3] = { -1.0, -1.0, n, 1.0 };
	out_corners[4] = { 1.0, 1.0, f, 1.0 };
	out_corners[5] = { -1.0, 1.0, f, 1.0 };
	out_corners[6] = { 1.0, -1.0, f, 1.0 };
	out_corners[7] = { -1.0, -1.0, f, 1.0 };

	float4x4 inv_m = hlslpp::inverse(cam_vp);
	transform_frustum(out_corners, inv_m);
	for (u32 i = 0; i < 8; ++i)
	{
		// Perspective divide
		out_corners[i].xyz = out_corners[i].xyz / out_corners[i].w;
		out_corners[i].w = 1.0f;
	}
}

} // namespace math
