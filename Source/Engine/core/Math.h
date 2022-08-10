#pragma once

namespace hlslpp_helpers
{

hlslpp::float3 to_euler(hlslpp::quaternion q);

}

namespace Math
{
using namespace hlslpp;

// Computes the normal of a triangle by using the 3 vertices defined by it
// 
// This function could be used to calculate the normals of a vertex by calculating the normals of each face the vertex contributes to.
float3 compute_normal(float3 p0, float3 p1, float3 p2);

// Inverse transpose is used to transform normal vectors properly when shearing and using non uniform matrices
float4x4 compute_inverse_transpose(float4x4 matrix);


using FrustumCorners = std::array<float4, 8>;

void calculate_frustum(FrustumCorners& out_corners, f32 n, f32 f, f32 fov, f32 vFov);
void calculate_frustum(FrustumCorners& out_corners, float4x4 cam_vp);
void transform_frustum(FrustumCorners& corners, float4x4 matrix);

}