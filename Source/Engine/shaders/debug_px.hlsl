#include "Common.hlsl"
#include "Lighting.hlsl"

cbuffer DebugCB : register(b1)
{
	int g_VisualizeMode;
};

#define VisualizeMode_Default 0
#define VisualizeMode_Albedo 1 
#define VisualizeMode_Roughness 2 
#define VisualizeMode_Metalness 3 
#define VisualizeMode_Normals 4 
#define VisualizeMode_AO 5
#define VisualizeMode_WorldNormal 6
#define VisualizeMode_VertexColour 7

// PBR inputs
Texture2D<float4> g_albedo    : register(t0);
Texture2D<float4> g_Data : register(t1); // .x: metalness, .y : roughness 
Texture2D<float4> g_Normal    : register(t2);

SamplerState g_AllLinearSampler : register(s0);
SamplerState g_AllPointSampler : register(s1);

float4 main(VS_OUT vout) : SV_Target
{
	float2 uv = vout.uv;

	Material material = CreateMaterial();
	material.albedo = g_albedo.Sample(g_AllLinearSampler, uv);
	material.tangentNormal = (g_Normal.Sample(g_AllLinearSampler, uv).rgb * 2.0 - 1.0);

	float4 data = g_Data.Sample(g_AllLinearSampler, uv);
	material.ao = data.r;
	material.roughness = data.g;
	material.metalness = data.b;

	float3 output = float3(1.0, 0.0, 0.0);
	if (g_VisualizeMode == VisualizeMode_VertexColour) {
		output = vout.colour;
	}
	else if (g_VisualizeMode == VisualizeMode_Albedo)
	{
		output = material.albedo;
	}
	else if (g_VisualizeMode == VisualizeMode_Normals)
	{
		output = (material.tangentNormal + 1.0) * 0.5;
	}
	else if (g_VisualizeMode == VisualizeMode_Roughness)
	{
		output = material.roughness;
	}
	else if (g_VisualizeMode == VisualizeMode_Metalness)
	{
		output = material.metalness;
	}
	else if (g_VisualizeMode == VisualizeMode_WorldNormal)
	{
		// Transform our tangent normal into world space
		float4 normal = normalize(vout.worldNormal);
		float4 tangent = normalize(vout.worldTangent);
		float3 bitangent = normalize(vout.worldBitangent.xyz);
		float3x3 TBN = float3x3(
			tangent.xyz,
			bitangent.xyz,
			normal.xyz
			);

		float3 final_normal = mul(material.tangentNormal, TBN);
		output = final_normal;
	}
	else if (g_VisualizeMode == VisualizeMode_AO)
	{
		output = material.ao;
	}

	return float4(output, 1.0);
}