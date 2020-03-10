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
};

// Light pointing down
float3 g_light = float3(0.0, -1.0, 0.0);

// PBR inputs
Texture2D<float4> g_Albedo    : register(t0);
Texture2D<float4> g_Data      : register(t1); // .x: metalness, .y : roughness 
Texture2D<float4> g_Normal    : register(t2);

SamplerState g_AllLinearSampler : register(s0);
SamplerState g_AllPointSampler : register(s1);

float4 main(VS_OUT vout) : SV_Target
{
	float2 uv = vout.uv;

	Material material = CreateMaterial();
	material.albedo = g_Albedo.Sample(g_AllLinearSampler, uv);
	material.tangentNormal = (g_Normal.Sample(g_AllLinearSampler, uv).rgb * 2.0 - 1.0);

	float4 data = g_Data.Sample(g_AllLinearSampler, uv);
	material.ao = data.r;
	material.roughness = data.g;
	material.metalness = data.b;

	// +Y is forward
	float3 light = normalize(-g_LightDirection.xyz);
	float3 view = normalize(g_ViewDirection.xyz);

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

	float3 light_colour = float3(0.9, 0.9, 0.9);
	float3 colour = SimpleBlinnPhong( view, light , final_normal,  material) * light_colour;

	float3 ambient = float3(0.02f, 0.02f, 0.02f);
	return float4(material.ao * (colour + ambient), 1.0);
}