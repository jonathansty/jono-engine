#include "Common.hlsl"

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

VS_OUT main(VS_IN vin)
{
	VS_OUT vout = (VS_OUT)(0);

	// TODO: Transform into world 
	vout.position = mul(WorldViewProjection, float4(vin.position,1.0));
	vout.normal = float4(vin.normal, 1.0);
	vout.colour = vin.colour;
	vout.uv = vin.uv;

	vout.viewNormal = mul(WorldView, float4(vin.normal, 0.0));
	vout.worldNormal = mul(World, float4(vin.normal, 0.0));
	vout.viewPosition = mul(WorldView, float4(vin.position, 1.0));

	return vout;
}