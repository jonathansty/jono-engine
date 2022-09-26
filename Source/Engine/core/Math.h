#pragma once


namespace Math
{
using namespace hlslpp;

struct AABB
{
	float3 center() const
	{
		return (max + min) / 2.0f;
	}

	float3 size() const 
	{
		return max - min;
	}

	float width() const { return size().x; }
	float height() const { return size().y; }
	float depth() const { return size().z; }

	float3 min;
	float3 max;
};

struct FrustumPlane
{
	float4 p[4]; // Points of the plane
	float4 center;
	float3 normal;

	FrustumPlane() = default;

	FrustumPlane(float4 a, float4 b, float4 c, float4 d);
};

struct Frustum
{
	static Frustum from_fov(f32 n, f32 f, f32 fov, f32 vertical_fov);
	static Frustum from_vp(float4x4 const& vp);

	void transform(float4x4 const& mat);

	// The 8 corners of a frustum
	enum FrustumPlanes
	{
		FrustumPlane_Front = 0,
		FrustumPlane_Back = 1,
		FrustumPlane_Left = 2,
		FrustumPlane_Right = 3,
		FrustumPlane_Top = 4,
		FrustumPlane_Bottom = 5,
		FrustumPlane_Count
	};

	std::array<FrustumPlane, FrustumPlane_Count> _planes; 
	std::array<float4, 8> _corners;


	private:
		void calculate_planes();
			
};



// Computes the normal of a triangle by using the 3 vertices defined by it
// 
// This function could be used to calculate the normals of a vertex by calculating the normals of each face the vertex contributes to.
float3 compute_normal(float3 p0, float3 p1, float3 p2);

// Inverse transpose is used to transform normal vectors properly when shearing and using non uniform matrices
float4x4 compute_inverse_transpose(float4x4 matrix);


using FrustumCorners = std::array<float4, 8>;

void transform_frustum(FrustumCorners& corners, float4x4 matrix);

bool test_frustum_sphere(Frustum const& frustum, float3 pos, f32 radius);

}

namespace hlslpp_helpers
{

hlslpp::float3 to_euler(hlslpp::quaternion q);

}
