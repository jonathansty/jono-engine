#include "Common.hlsl"

cbuffer Data : register(b0)
{
	float4x4 g_ProjectionInv;
	float4x4 g_View;

	int g_NumTilesX;
	int g_NumTilesY;
	int g_NumberOfLights;
}


groupshared uint ldsZMax;
groupshared uint ldsZMin;
groupshared uint ldsLightIdxCounterA = 0;
groupshared uint ldsLightIdxCounterB = FPLUS_MAX_NUM_LIGHTS_PER_TILE;
groupshared uint ldsLightIdx[FPLUS_MAX_NUM_LIGHTS_PER_TILE];

struct Frustum
{
	float4 planes[6];
};


StructuredBuffer<ProcessedLight> g_Lights : register(t0);
Texture2D<float> g_depth : register(t1);

RWBuffer<uint> g_PerTileLightIndexBuffer : register(u0);
RWBuffer<uint> g_PerTileInfo : register(u1);

#if 1 

bool IsVisible(Frustum f, ProcessedLight light)
{
	return true;
}


float4 CreatePlaneEquation(float4 b, float4 c)
{
	float4 n;
	// normalize(cross(b.xyz-a.xyz, c.xyz-a.xyz)), "a" == origin
	n.xyz = normalize(cross(b.xyz, c.xyz));
	
	// -(n dot a), "a" == origin
	n.w = 0;
	return n;
}

float GetSignedDistanceFromPlane(float4 p, float4 eqn)
{
    // Optional: dot( eqn.xyz, p.xyz ) + eqn.w, , except we know eqn.w is zero 
    // (see CreatePlaneEquation above)
	return dot(eqn.xyz, p.xyz) + eqn.w;
}

void CullLights(in uint3 dispatchThreadID, in uint index,  uint3 groupID)
{
	// First thread of the group is the one who does all the setup
	if(index == 0)
	{
		// #TODO: Handle LDS resetting on the first thread
		ldsZMin = 0x7f7fffff;
		ldsZMax = 0;
		ldsLightIdxCounterA = 0;
		ldsLightIdxCounterB = FPLUS_MAX_NUM_LIGHTS_PER_TILE;

		ldsLightIdx[0] = 0;
	}

	// Calculate the frustum
	float4 frustumEqn[4];
	{
		uint x0 = FPLUS_TILE_RES*groupID.x; 
		uint y0 = FPLUS_TILE_RES*groupID.y; 
		uint x1 = FPLUS_TILE_RES*(groupID.x+1);
		uint y1 = FPLUS_TILE_RES*(groupID.y+1);

		uint windowWidth = FPLUS_TILE_RES * g_NumTilesX;
		uint windowHeight = FPLUS_TILE_RES * g_NumTilesY;

		// Creates the 4 frustum corners (far plane), we are converting to view space and assume the origin is where we start to create our frustum planes. This is possible
		// because we will be using minZ/maxZ from the depth buffer
		float4 frustum[4];
		frustum[0] = ConvertProjToView(g_ProjectionInv, float4( (x0/(float)windowWidth) * 2.f - 1.f, (y0/(float)windowHeight) * 2.f - 1.f, 1.0f, 1.0f) );
		frustum[1] = ConvertProjToView(g_ProjectionInv, float4( (x1/(float)windowWidth) * 2.f - 1.f, (y0/(float)windowHeight) * 2.f - 1.f, 1.0f, 1.0f) );
		frustum[2] = ConvertProjToView(g_ProjectionInv, float4( (x1/(float)windowWidth) * 2.f - 1.f, (y1/(float)windowHeight) * 2.f - 1.f, 1.0f, 1.0f) );
		frustum[3] = ConvertProjToView(g_ProjectionInv, float4( (x0/(float)windowWidth) * 2.f - 1.f, (y1/(float)windowHeight) * 2.f - 1.f, 1.0f, 1.0f) );

		for(uint i = 0; i < 4; ++i)
		{
			frustumEqn[i] = CreatePlaneEquation(frustum[i], frustum[(i+1) & 3]);
		}
	}
	GroupMemoryBarrierWithGroupSync();



	// Calculate the min/max depth for each tile using LDS and memory barriers
	{
		float opaqueDepth = g_depth.Load(uint3(dispatchThreadID.x, dispatchThreadID.y, 0)).x;
		float z = ConvertProjDepthToView(g_ProjectionInv, opaqueDepth);
		uint zAsUint = asuint(z);

		InterlockedMax(ldsZMax, zAsUint);
		InterlockedMin(ldsZMin, zAsUint);
	}
	GroupMemoryBarrierWithGroupSync();


	float maxZ = asfloat(ldsZMax);
	float minZ = asfloat(ldsZMin);

 #if 1
	for(uint i = index; i < g_NumberOfLights; i += FPLUS_NUM_THREADS_PER_TILE)
	{
		float4 center = float4(g_Lights[index].position, 1.0f);
		float r = g_Lights[index].range;

		// Convert the center into view space to do culling
		center.xyz = mul(float4(center.xyz, 1.0f), g_View).xyz;

		if( (GetSignedDistanceFromPlane(center, frustumEqn[0]) < r) &&
			(GetSignedDistanceFromPlane(center, frustumEqn[1]) < r) &&
			(GetSignedDistanceFromPlane(center, frustumEqn[2]) < r) &&
			(GetSignedDistanceFromPlane(center, frustumEqn[3]) < r) )
		{
			uint dstIdx = 0;
			InterlockedAdd(ldsLightIdxCounterA, 1, dstIdx);
			ldsLightIdx[dstIdx] = index;
		}
	}

	GroupMemoryBarrierWithGroupSync();
	#endif
}

[numthreads(FPLUS_NUM_THREADS_X,FPLUS_NUM_THREADS_Y,1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID, uint3 groupThreadID : SV_GroupThreadID, uint3 groupID : SV_GroupID) 
{
	// Flatten to a 1D ID
	uint thread_index = groupThreadID.x + groupThreadID.y * FPLUS_NUM_THREADS_X;

	// 2: Frustum cull all the lights locally for this tile
	CullLights(dispatchThreadID, thread_index, groupID);


	// Write out all lights
	{
		uint tileIdxFlattened = groupID.x + groupID.y * g_NumTilesX;

		uint startOffset = FPLUS_MAX_NUM_LIGHTS_PER_TILE * tileIdxFlattened; // Add 1 here to allow the first item to be used to store the counter

		for(uint i = thread_index; i < ldsLightIdxCounterA; i+= FPLUS_NUM_THREADS_PER_TILE)
		{
			g_PerTileLightIndexBuffer[startOffset+i] = ldsLightIdx[i];
		}

		// First uint contains how many lights were in the tile
		if(thread_index == 0)
		{
			g_PerTileInfo[tileIdxFlattened] = ldsLightIdxCounterA;
		}
	}
}
#endif