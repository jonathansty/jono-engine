#ifndef _COMMON_H_
#define _COMMON_H_

#define LIGHTING_MODEL_BLINN_PHONG 1
#define LIGHTING_MODEL_PHONG 2
#define LIGHTING_MODEL_PBR 3

#define MAX_LIGHTS 4
#define MAX_CASCADES 4

// When compiling from VS specify the default lighting model as PBR
struct AmbientInfo
{
	float4 ambient;
};
struct LightInfo
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

#ifndef _cplusplus

#if VS_COMPILE
#define LIGHTING_MODEL LIGHTING_MODEL_PBR
#endif

// For now force it
#define LIGHTING_MODEL LIGHTING_MODEL_BLINN_PHONG 
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
	float4 lightSpacePos : POSITION1;
	float4 worldPosition : POSITION2;
	float4 worldNormal : NORMAL1;
	float4 viewNormal : NORMAL2;
	float4 worldTangent : TANGENT0;
	float4 worldBitangent : TANGENT1;
};

cbuffer WorldConstants : register(b0)
{
	float4x4 View;
	float4x4 InvView;
	float4x4 Projection;
	float4x4 InvProjection;
	float4x4 InvViewProjection;
	float4 g_ViewDirection;

	Viewport_t Viewport;
	AmbientInfo g_Ambient;
	LightInfo g_Lights[MAX_LIGHTS];
	unsigned int num_lights;
};

cbuffer DebugCB : register(b1)
{
	int g_VisualizeMode;
};

cbuffer ModelConstants : register(b2)
{
	float4x4 World;
	float4x4 WorldView;
	float4x4 WorldViewProjection;
};
#endif

#endif