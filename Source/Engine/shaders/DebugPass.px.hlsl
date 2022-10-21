#include "CommonScene.hlsl"
#include "Lighting.hlsl"

#define VisualizeMode_Default 0
#define VisualizeMode_Albedo 1 
#define VisualizeMode_Roughness 2 
#define VisualizeMode_Metalness 3 
#define VisualizeMode_Normals 4 
#define VisualizeMode_AO 5
#define VisualizeMode_WorldNormal 6
#define VisualizeMode_VertexColour 7
#define VisualizeMode_UV 8
#define VisualizeMode_Lighting 9

float4 main(VS_OUT vout) : SV_Target
{
	float2 uv = vout.uv;

	Material material = EvaluateMaterial(vout);
	float3 output = (float3)0.0;

	if (g_VisualizeMode == VisualizeMode_VertexColour) 
	{
		output = vout.colour.rgb;
	}
	else if (g_VisualizeMode == VisualizeMode_Albedo)
	{
		output = material.albedo;
	}
	else if (g_VisualizeMode == VisualizeMode_Normals)
	{
		output = (material.tangentNormal * 0.5f) + 0.5f;
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

		output = (final_normal *0.5f) + 0.5f;
	}
	else if (g_VisualizeMode == VisualizeMode_AO)
	{
		output = material.ao;
	}
	else if(g_VisualizeMode == VisualizeMode_UV)
	{
		output = float3(uv.xy, 0.0f);
	}
	else if(g_VisualizeMode == VisualizeMode_Lighting)
	{
		material.albedo = float3(1.0f,1.0f,1.0f);
		material.metalness = 0.0;
		output = EvaluateLighting(material, vout);
	}
	

	return float4(output, 1.0);
}