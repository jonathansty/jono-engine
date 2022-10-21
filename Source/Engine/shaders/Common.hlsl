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

#endif