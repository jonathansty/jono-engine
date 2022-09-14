#include "Common.h"
#include "Lighting.hlsl"

struct MaterialData
{
	float3 albedo;
	float roughness;
	float metalness;
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
	material.tangentNormal = float3(0.0f,0.0f,1.0f);
	material.ao = 1.0f;
	material.roughness = g_MaterialData.roughness;
	material.metalness = g_MaterialData.metalness;
	material.albedo = g_MaterialData.albedo;
	material.F0 = float3(0.04, 0.04, 0.04);

	return material;
}