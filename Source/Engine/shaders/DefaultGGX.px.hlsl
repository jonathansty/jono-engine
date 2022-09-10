#include "Common.h"
#include "Lighting.hlsl"

Texture2D<float4> g_albedo    				: register(t0);
Texture2D<float4> g_metallic_roughness      : register(t1);
Texture2D<float4> g_normal      			: register(t2); 
Texture2D<float4> g_ao      				: register(t3); 
Texture2D<float4> g_Emissive      		    : register(t4); 

struct MaterialData
{
	float3 albedo;
	float3 roughness;
	float3 metalness;
};

// b1 used for storing per material data
cbuffer GlobalMaterialData : register(b3)
{
	MaterialData g_MaterialData;
}

Material EvaluateMaterial(VS_OUT vout)
{
	float2 uv = vout.uv;

	Material material = CreateMaterial();

	float4 metallic_roughness = g_metallic_roughness.Sample(g_all_linear_sampler, uv);
	float3 albedo = g_albedo.Sample(g_all_linear_sampler, uv).rgb;
	float3 normals = g_normal.Sample(g_all_linear_sampler, uv).rgb * 2.0 - 1.0;
	float ao = g_ao.Sample(g_all_linear_sampler, uv).r;
	material.tangentNormal = normals;
	material.ao = ao;
	material.roughness = metallic_roughness.g;
	material.metalness = metallic_roughness.b;
	material.albedo = albedo;
	material.F0 = float3(0.04, 0.04, 0.04);

	return material;
}