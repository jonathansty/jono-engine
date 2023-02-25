#ifndef _COMMON_HLSL_
#define _COMMON_HLSL_
#include "CommonShared.h"

#define LIGHTING_MODEL_BLINN_PHONG 1
#define LIGHTING_MODEL_PHONG 2
#define LIGHTING_MODEL_PBR 3

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
	float3 normal   : NORMAL0;

#ifdef _USE_TANGENTS
	float3 tangent : TANGENT0;
	float3 bitangent : TANGENT1;
#endif

#ifdef _USE_UV0
	float2 uv0 : TEXCOORD0;
#endif

#ifdef _USE_UV1
	float2 uv1 : TEXCOORD1;
#endif

#ifdef _USE_COLOUR
	float4 colour : COLOR0;
#endif
};

struct VS_OUT
{
	float4 position : SV_Position;
	float4 normal   : NORMAL0;

	float4 viewPosition : POSITION0;
	float4 worldPosition : POSITION1;
	float4 worldNormal : NORMAL1;
	float4 viewNormal : NORMAL2;

 #ifdef _USE_COLOUR
	float4 colour : COLOR0;
#endif

#ifdef _USE_UV0
	float2 uv0 : TEXCOORD0;
#endif

#ifdef _USE_TANGENTS
	float4 worldTangent : TANGENT0;
	float4 worldBitangent : TANGENT1;
#endif
};

float2 GetUV0(VS_OUT vsOut)
{
#ifdef _USE_UV0
return vsOut.uv0;
#else
return float2(0.0f,0.0f);
#endif
}

float4 GetColour0(VS_OUT vsOut)
{
#ifdef _USE_COLOUR
	return vsOut.colour;
#else
	return (float4)0.0f;
#endif
}

float4 GetWorldNormal(VS_OUT vsOut)
{
	return vsOut.worldNormal;
}

float4 GetWorldTangent(VS_OUT vsOut)
{
#ifdef _USE_TANGENTS
	return vsOut.worldTangent;
#else
	return float4(0.0f,0.0f,0.0f,0.0f);
#endif
}


float4 GetWorldBitangent(VS_OUT vsOut)
{
#ifdef _USE_TANGENTS
	return vsOut.worldBitangent;
#else
	return float4(0.0f,0.0f,0.0f,0.0f);
#endif
}

float4 ConvertProjToView(in float4x4 projInv, in float4 p)
{
	p = mul(p, projInv);
	p /= p.w;
	return p;
}

float ConvertProjDepthToView(in float4x4 projInv, float z)
{
	z = 1.f / (z*projInv._34 + projInv._44);
	return z;
}

// Useful globals used all across the shaders

// The screen position in pixels
static float2 g_ScreenPosition;

void SetupGlobals(VS_OUT vsOut)
{
	g_ScreenPosition = vsOut.position;
}

#endif