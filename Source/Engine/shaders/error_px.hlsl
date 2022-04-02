#include "Common.hlsl"
#include "Lighting.hlsl"


// PBR Inputs
// TODO: Should this be packed into a texture array (assuming the textures are the same size)
Texture2D<float4> g_albedo    : register(t0);
Texture2D<float4> g_data      : register(t1); // .x: metalness, .y : roughness 
Texture2D<float4> g_normal    : register(t2);

Texture2D<float> g_shadow_map : register(t3);

// Default Samplers
SamplerState g_all_linear_sampler : register(s0);

float4 main(VS_OUT vout) : SV_Target {
	float2 uv = vout.uv;

	Material material = CreateMaterial();
	material.albedo = g_albedo.Sample(g_all_linear_sampler, uv).rgb;

	material.tangentNormal = (g_normal.Sample(g_all_linear_sampler, uv).rgb * 2.0 - 1.0);

	float4 data = g_data.Sample(g_all_linear_sampler, uv);
	material.ao = data.r;
	material.roughness = data.g;
	material.metalness = data.b;

	// Transform our tangent normal into world space
	float4 normal = normalize(vout.worldNormal);
	float4 tangent = normalize(vout.worldTangent);
	float3 bitangent = normalize(vout.worldBitangent.xyz);
	float3x3 tbn = float3x3(
			tangent.xyz,
			bitangent.xyz,
			normal.xyz);

	float3 final_normal = normalize(mul(material.tangentNormal, tbn));

	float3 view = normalize(g_ViewDirection.xyz);
	float3 final_colour = float3(0.0,1.0,1.0);

	return float4(final_colour, 1.0);
}