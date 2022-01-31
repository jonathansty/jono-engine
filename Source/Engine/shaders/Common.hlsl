#ifndef _COMMON_H_
#define _COMMON_H_

// DirectX::VertexPositionNormalColorTexture
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

struct AmbientInfo {
	float4 ambient;
};
struct LightInfo {
	float4 colour;
	float4 direction;
	float4x4 light_space;
};

#define MAX_LIGHTS 4

cbuffer WorldConstants : register(b0) {
	float4x4 View;
	float4x4 InvView;
	float4x4 Projection;
	float4 g_ViewDirection;

	AmbientInfo g_Ambient;

	LightInfo g_Lights[MAX_LIGHTS];
	unsigned int num_lights;
};

cbuffer ModelConstants : register(b1) {
	float4x4 World;
	float4x4 WorldView;
	float4x4 WorldViewProjection;
};

cbuffer DebugCB : register(b2) {
	int g_VisualizeMode;
};




#endif