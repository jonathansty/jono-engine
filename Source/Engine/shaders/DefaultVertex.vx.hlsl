#include "Common.h"

VS_OUT main(VS_IN vin)
{
	VS_OUT vout = (VS_OUT)(0);

	vout.position = mul(WorldViewProjection, float4(vin.position,1.0));
	vout.normal = float4(vin.normal, 1.0);
	vout.colour = vin.colour;
	vout.uv = vin.uv;

	vout.worldPosition  = mul(World, float4(vin.position, 1.0));
	vout.viewPosition   = mul(WorldView, float4(vin.position, 1.0));
	vout.viewNormal     = mul(WorldView, float4(vin.normal, 0.0));
	vout.worldNormal    = mul(World, float4(vin.normal, 0.0));
	vout.worldTangent   = mul(World, vin.tangent);
	vout.worldBitangent = mul(World, vin.bitangent);

	return vout;
}