#include "CommonScene.hlsl"

#ifdef _VERTEX

VS_OUT main(VS_IN vin)
{
	VS_OUT vout = (VS_OUT)(0);

	vout.position = mul(WorldViewProjection, float4(vin.position,1.0));
#ifdef _USE_COLOUR
	vout.colour = vin.colour;
#endif

#ifdef _USE_UV0
	vout.uv0 = vin.uv0;
#endif

	vout.worldPosition  = mul(World, float4(vin.position, 1.0));
	vout.viewPosition   = mul(WorldView, float4(vin.position, 1.0));

#ifdef _USE_NORMAL0
	vout.normal = float4(vin.normal, 1.0);
	vout.viewNormal     = mul(WorldView, float4(vin.normal, 0.0));
	vout.worldNormal    = mul(World, float4(vin.normal, 0.0));
#endif

#ifdef _USE_TANGENTS
	vout.worldTangent   = mul(World, vin.tangent);
	vout.worldBitangent = mul(World, vin.bitangent);
#endif

	return vout;
}

#endif