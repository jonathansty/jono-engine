struct VS_OUT
{
	float4 position : SV_Position;
	float4 normal   : NORMAL0;

	float4 viewPosition : POSITION0;
	float4 worldPosition : POSITION1;
	float4 worldNormal : NORMAL1;
	float4 viewNormal : NORMAL2;
};

struct VS_IN
{
	float3 position : SV_Position;
	float3 normal : NORMAL0;
};


#ifdef _VERTEX

cbuffer ModelConstants : register(b2)
{
	float4x4 World;
	float4x4 WorldView;
	float4x4 WorldViewProjection;
};

VS_OUT main(VS_IN vin)
{
	VS_OUT vout = (VS_OUT)(0);

	// Calculate the world position 
	float4 worldPosition = mul(World, float4(vin.position, 1.0));

	vout.position = mul(WorldViewProjection, float4(vin.position,1.0));
	vout.normal = float4(vin.normal, 1.0);
	vout.worldPosition  = mul(World, float4(vin.position, 1.0));
	vout.viewPosition   = mul(WorldView, float4(vin.position, 1.0));

	vout.viewNormal     = mul(WorldView, float4(vin.normal, 0.0));
	vout.worldNormal    = mul(World, float4(vin.normal, 0.0));

	return vout;
}

#endif // _VERTEX

#ifdef _PIXEL

float4 main(VS_OUT vout) : SV_Target 
{
	float3 final_colour = float3(0.0,1.0,1.0);
	return float4(final_colour, 1.0);
}

#endif // _PIXEL