#include "engine.pch.h"
#include "Math.h"


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

void transform_frustum(FrustumCorners& corners, float4x4 matrix)
{
	for (u32 i = 0; i < corners.size(); ++i)
	{
		corners[i] = hlslpp::mul(corners[i], matrix);
	}
}

bool test_frustum_sphere(Frustum const& frustum, float3 pos, f32 radius)
{
	//std::array<FrustumPlane, 6> planes = {
	//	FrustumPlane( { frustum[3], frustum[1], frustum[0], frustum[2] } ), // Front
	//	FrustumPlane( { frustum[7], frustum[5], frustum[4], frustum[6] } ), // Back
	//	FrustumPlane( { frustum[7], frustum[5], frustum[1], frustum[3] } ), // Left
	//	FrustumPlane( { frustum[2], frustum[0], frustum[4], frustum[6] } ), // Right
	//	FrustumPlane( { frustum[1], frustum[5], frustum[4], frustum[0] } ), // Top
	//	FrustumPlane( { frustum[7], frustum[3], frustum[2], frustum[6] } ), // Bottom
	//};

	bool inside = true;
	for(int i = 0; i < frustum._planes.size(); ++i)
	{
		float3 normal = frustum._planes[i].normal;
		float3 to_plane_center = pos - frustum._corners[0].xyz;

		// Distance to plane
		float d = hlslpp::dot(to_plane_center, normal);
		if(d > 0.0f)
		{
			inside = false;
			break;
		}
	}

	return inside;
}

Frustum Frustum::from_fov(f32 n, f32 f, f32 fov, f32 vertical_fov)
{
	Frustum result{};

	f32 tan_half_fov = tanf(fov / 2.0f);
	f32 tan_half_vfov = tanf(vertical_fov / 2.0f);

	f32 x1 = n * tan_half_fov;
	f32 x2 = f * tan_half_fov;
	f32 y1 = n * tan_half_vfov;
	f32 y2 = f * tan_half_vfov;

	// Calculate the frustum in view space
	result._corners[0] = {  x1,  y1, n, 1.0f }; // Front top right
	result._corners[1] = { -x1,  y1, n, 1.0f }; // Front top left
	result._corners[2] = {  x1, -y1, n, 1.0f }; // Front bottom right
	result._corners[3] = { -x1, -y1, n, 1.0f }; // Front bottom left
	result._corners[4] = {  x2,  y2, f, 1.0f }; // Back top right
	result._corners[5] = { -x2,  y2, f, 1.0f }; // Back top left
	result._corners[6] = {  x2, -y2, f, 1.0f }; // Back bottom right
	result._corners[7] = { -x2, -y2, f, 1.0f }; // Back bottom left

	// Front
	result.calculate_planes();

	return result;
}

Frustum Frustum::from_vp(float4x4 const& vp)
{
	Frustum result{};

	f32 n = -1.0f;
	f32 f = 1.0f;
	f32 x1 = 1.0f;
	f32 x2 = 1.0f;
	f32 y1 = 1.0f;
	f32 y2 = 1.0f;

	// Calculate the frustum in view space
	result._corners[0] = {  1.0,  1.0, n, 1.0f};
	result._corners[1] = { -1.0,  1.0, n, 1.0f};
	result._corners[2] = {  1.0, -1.0, n, 1.0f};
	result._corners[3] = { -1.0, -1.0, n, 1.0f};
	result._corners[4] = {  1.0,  1.0, f, 1.0f};
	result._corners[5] = { -1.0,  1.0, f, 1.0f};
	result._corners[6] = {  1.0, -1.0, f, 1.0f};
	result._corners[7] = { -1.0, -1.0, f, 1.0f};

	float4x4 inv_m = hlslpp::inverse(vp);
	transform_frustum(result._corners, inv_m);
	for (u32 i = 0; i < 8; ++i)
	{
		// Perspective divide
		result._corners[i].xyz = result._corners[i].xyz / result._corners[i].w;
		result._corners[i].w = 1.0f;
	}

	result.calculate_planes();
	return result;
}

void Frustum::transform(float4x4 const& mat)
{
	for (u32 i = 0; i < _corners.size(); ++i)
	{
		_corners[i] = hlslpp::mul(_corners[i], mat);
	}
	calculate_planes();
}

void Frustum::calculate_planes()
{
	_planes[FrustumPlane_Front] = FrustumPlane(_corners[1], _corners[0], _corners[2], _corners[3]);
	_planes[FrustumPlane_Back] = FrustumPlane(_corners[5], _corners[4], _corners[6], _corners[7]);

	_planes[FrustumPlane_Left] = FrustumPlane(_corners[5], _corners[1], _corners[3], _corners[7]);
	_planes[FrustumPlane_Right] = FrustumPlane(_corners[0], _corners[4], _corners[6], _corners[2]);

	_planes[FrustumPlane_Top] = FrustumPlane(_corners[1], _corners[5], _corners[4], _corners[0]);
	_planes[FrustumPlane_Bottom] = FrustumPlane(_corners[7], _corners[3], _corners[2], _corners[6]);
}

// Plane passed in as follows:
//  a ---- b 
//  |      |
//  d ---- c
 FrustumPlane::FrustumPlane(float4 a, float4 b, float4 c, float4 d)
{
	p[0] = a;
	p[1] = b;
	p[2] = c;
	p[3] = d;

	float3 aVec = hlslpp::normalize(b - a).xyz;
	float3 bVec = hlslpp::normalize(d - a).xyz;
	normal = hlslpp::cross(aVec, bVec);

	center = (a + b + c + d) / 4.0f;
}

} // namespace math

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
