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
Texture2D<float> g_Albedo    : register(t0);
Texture2D<float> g_Normal    : register(t3);
Texture2D<float> g_Data : register(t1); // .x: Roughness, .y : metalness 

SamplerState g_AllLinearSampler : register(s0);
SamplerState g_AllPointSampler : register(s1);

float4 main(VS_OUT vout) : SV_Target
{
	float2 uv = vout.uv;

	Material material = CreateMaterial();
	//material.albedo = g_Albedo.Sample(g_AllLinearSampler, uv);
	material.albedo = vout.colour.xyz;

	//float4 data = g_Data.Sample(g_AllLinearSampler, uv);
	//material.roughness = data.x;
	//material.metalness = data.y;
	//material.tangentNormal = g_Normal.Sample(g_AllLinearSampler, uv);

	// +Y is forward
	float3 light = normalize(-g_LightDirection.xyz);
	float3 view = normalize(g_ViewDirection.xyz);
	float3 normal = normalize(vout.worldNormal.xyz);
	float3 colour = SimpleBlinnPhong( view, light , normal,  material);
	return float4(colour, 1.0);
}