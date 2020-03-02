#ifndef _COMMON_H_
#define _COMMON_H_

// DirectX::VertexPositionNormalColorTexture
struct VS_IN
{
	float3 position : SV_Position;
	float3 normal : NORMAL0;
	float4 colour : COLOR0;
	float2 uv : TEXCOORD0;
};

struct VS_OUT
{
	float4 position : SV_Position;

	float4 normal : NORMAL0;
	float4 colour : COLOR0;
	float2 uv : TEXCOORD0;

	float3 viewPosition : TPOSITION;
	float3 worldNormal : NORMAL1;
	float3 viewNormal : NORMAL2;
};

#endif