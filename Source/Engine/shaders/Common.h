#ifndef _COMMON_H_
#define _COMMON_H_

#define LIGHTING_MODEL_BLINN_PHONG 1
#define LIGHTING_MODEL_PHONG 2
#define LIGHTING_MODEL_PBR 3

#define MAX_DIRECTIONAL_LIGHTS 4
#define MAX_CASCADES 4

#define Buffer_Global 0
#define Buffer_Debug 1
#define Buffer_Model 2
#define Buffer_Material 3

#define Texture_MaterialSlotStart 0
#define Texture_MaterialSlotEnd 5 
#define Texture_CSM Texture_MaterialSlotEnd 
#define Texture_Depth 6
#define Texture_Cube 7
#define Texture_Lights 8


#define Sampler_Linear 0
#define Sampler_Point 1

#define LIGHT_TYPE_POINT 0x1
#define LIGHT_TYPE_SPOT 0x2

struct AmbientInfo
{
	float4 ambient;
};
struct DirectionalLightInfo
{
	float4 colour;
	float4 direction;
	float4x4 light_space;

	int num_cascades;
	float4x4 cascade[MAX_CASCADES];
	float4 cascade_distance[MAX_CASCADES];
};

struct Viewport_t
{
	float HalfWidth;
	float HalfHeight;
	float TopLeftX;
	float TopLeftY;
	float MinDepth;
	float MaxDepth;
};



// HLSL only code
#ifndef __cplusplus

#define CB_SLOT(x)  register(b##x)
#define SRV_SLOT(x) register(t##x)
#define SAMPLER_SLOT(x) register(s##x)

#if VS_COMPILE
#define LIGHTING_MODEL LIGHTING_MODEL_PBR
#endif

// For now force it
// #define LIGHTING_MODEL LIGHTING_MODEL_BLINN_PHONG 
// #define LIGHTING_MODEL LIGHTING_MODEL_PBR

struct VS_IN
{
	float3 position : SV_Position;
	float3 normal : NORMAL0;
	float4 tangent : TANGENT0;
	float4 bitangent : TANGENT1;
	float4 colour : COLOR0;
	float2 uv : TEXCOORD0;
};

struct VS_OUT
{
	float4 position : SV_Position;

	float4 normal : NORMAL0;
	float4 colour : COLOR0;
	float2 uv : TEXCOORD0;

	float4 viewPosition : POSITION0;
	float4 worldPosition : POSITION1;
	float4 worldNormal : NORMAL1;
	float4 viewNormal : NORMAL2;
	float4 worldTangent : TANGENT0;
	float4 worldBitangent : TANGENT1;
};

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
	unsigned int num_directional_lights; // Describes the number of directional lights
	unsigned int num_lights; // Describes the number of local lights in the structured buffer
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

Texture2DArray<float> g_shadow_map : SRV_SLOT(Texture_CSM); 
Texture2D<float>      g_depth : SRV_SLOT(Texture_Depth);
TextureCube<float4>   g_cube : SRV_SLOT(Texture_Cube);

struct ProcessedLight
{
	float3 position;
	unsigned int flags;

	float3 color;
	float range;

	float3 direction;
	float cone;
	float outer_cone;
};
StructuredBuffer<ProcessedLight> g_lights : SRV_SLOT(Texture_Lights);
#endif

// CPP only code
#ifdef __cplusplus
struct ProcessedLight
{
	Shaders::float3 position;
	unsigned int flags;

	Shaders::float3 color;
	f32 range;

	Shaders::float3 direction;
	f32 cone;
	f32 outer_cone;
};
#endif

#endif