#ifndef _FORWARD_PLUS_HLSL_
#define _FORWARD_PLUS_HLSL_

// Retrieves a tile index for a given position
uint2 GetTilePos(float2 screenPosition)
{
	return uint2(floor(screenPosition.x/FPLUS_TILE_RES), floor(screenPosition.y/FPLUS_TILE_RES));
}

uint GetTileIndex(float2 screenPosition)
{
	uint2 tilePos = GetTilePos(screenPosition);
	uint tileIdx = tilePos.x + tilePos.y * g_NumTilesX;
	return tileIdx;
}

void GetLightListInfo(in Buffer<uint> data, out uint numLights, out uint firstLightIdxInPerTile)
{
	uint tileIndex = GetTileIndex(g_ScreenPosition);
	uint startIndex = FPLUS_MAX_NUM_LIGHTS_PER_TILE * tileIndex;
	numLights = g_PerTileInfo[tileIndex];
	firstLightIdxInPerTile = g_PerTileLightIndexBuffer[startIndex];
}

#endif