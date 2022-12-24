#ifndef _COMMON_SCENE_H_
#define _COMMON_SCENE_H_
#include "Common.hlsl"

cbuffer WorldConstants : CB_SLOT(Buffer_Global)
{
	float4x4 View;
	float4x4 InvView;
	float4x4 Projection;
	float4x4 InvProjection;
	float4x4 InvViewProjection;

	float4 g_ViewDirection;
	float4 g_ViewPosition;

	Viewport_t Viewport;
	AmbientInfo g_Ambient;
	// Directional lights are packed into the world constant buffer
	DirectionalLightInfo g_Lights[MAX_DIRECTIONAL_LIGHTS]; 
	unsigned int g_NumDirectionalLights; // Describes the number of directional lights
	unsigned int g_NumLights; // Describes the number of local lights in the structured buffer

	uint g_NumTilesX; // Describes the number of tiles for F+ culling
};

cbuffer DebugCB : CB_SLOT(Buffer_Debug)
{
	int g_VisualizeMode;
};

cbuffer ModelConstants : CB_SLOT(Buffer_Model)
{
	float4x4 World;
	float4x4 WorldView;
	float4x4 WorldViewProjection;
};

// Default Samplers
SamplerState g_all_linear_sampler : SAMPLER_SLOT(Sampler_Linear);
SamplerState g_point_sampler : SAMPLER_SLOT(Sampler_Point);
SamplerState g_SamplerLinearClamp : SAMPLER_SLOT(Sampler_ClampLinear);
SamplerState g_SamplerPointClamp : SAMPLER_SLOT(Sampler_ClampPoint);

Texture2DArray<float> g_shadow_map : SRV_SLOT(Texture_CSM); 
Texture2D<float>      g_depth : SRV_SLOT(Texture_Depth);
TextureCube<float4>   g_cube : SRV_SLOT(Texture_Cube);

StructuredBuffer<ProcessedLight> g_lights : SRV_SLOT(Texture_Lights);

Buffer<uint> g_PerTileLightIndexBuffer : SRV_SLOT(Texture_ForwardPlusPerTileLightIndex);
Buffer<uint> g_PerTileInfo : SRV_SLOT(Texture_ForwardPlusTileInfo);

#include "ForwardPlus.hlsl"

#endif // _COMMON_SCENE_H_