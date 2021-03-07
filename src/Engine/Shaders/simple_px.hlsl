#include "Common.hlsl"
#include "Lighting.hlsl"

cbuffer MVPConstantBuffer : register(b0)
{
	float4x4 World;
	float4x4 WorldView;
	float4x4 Projection;
	float4x4 WorldViewProjection;

	float4x4 View;
	float4x4 InvView;

	float4 g_ViewDirection;
	float4 g_LightDirection;
	float4 g_LightColor;
};

// PBR Inputs
// TODO: Should this be packed into a texture array (assuming the textures are the same size)
Texture2D<float4> g_albedo    : register(t0);
Texture2D<float4> g_data      : register(t1); // .x: metalness, .y : roughness 
Texture2D<float4> g_normal    : register(t2);

// Default Samplers
SamplerState g_all_linear_sampler : register(s0);
SamplerState g_all_point_sampler  : register(s1);

float4 main(VS_OUT vout) : SV_Target
{
	float2 uv = vout.uv;

	Material material = CreateMaterial();
	material.albedo = g_albedo.Sample(g_all_linear_sampler, uv);
	material.tangentNormal = (g_normal.Sample(g_all_linear_sampler, uv).rgb * 2.0 - 1.0);

	float4 data = g_data.Sample(g_all_linear_sampler, uv);
	material.ao = data.r;
	material.roughness = data.g;
	material.metalness = data.b;

	float3 light = normalize(g_LightDirection.xyz);
	float3 view = normalize(g_ViewDirection.xyz);

	// Transform our tangent normal into world space
	float4 normal = normalize(vout.worldNormal);
	float4 tangent = normalize(vout.worldTangent);
	float3 bitangent = normalize(vout.worldBitangent.xyz);
	float3x3 tbn = float3x3(
		tangent.xyz,
		bitangent.xyz,
		normal.xyz
	);

	float3 final_normal = normalize(mul(material.tangentNormal, tbn));

	float3 light_colour = float3(0.9, 0.9, 0.9);
	float3 colour = SimpleBlinnPhong(view, light, final_normal,  material) * light_colour;

	float3 ambient = float3(0.02f, 0.02f, 0.02f);
	return float4(material.ao * (colour + ambient), 1.0);
}