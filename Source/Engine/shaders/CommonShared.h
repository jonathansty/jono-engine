#ifndef _COMMON_H_
#define _COMMON_H_

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


#define FPLUS_TILE_RES 16
#define FPLUS_NUM_THREADS_X FPLUS_TILE_RES 
#define FPLUS_NUM_THREADS_Y FPLUS_TILE_RES 
#define FPLUS_NUM_THREADS_PER_TILE FPLUS_NUM_THREADS_X * FPLUS_NUM_THREADS_Y
#define FPLUS_MAX_NUM_LIGHTS_PER_TILE 26


// Define type overrides for both C++ and hlsl
#ifdef __cplusplus
#define vec3 Shaders::float3
#define vec4 Shaders::float4
#define mat4x4 Shaders::float4x4
#else // __cplusplus
#define f32  float
#define vec3 float3
#define vec4 float4
#define mat4x4 float4x4
#endif // !__cplusplus

struct AmbientInfo
{
	vec4 ambient;
};
struct DirectionalLightInfo
{
	vec4 colour;
	vec4 direction;
	mat4x4 light_space;

	int num_cascades;
	mat4x4 cascade[MAX_CASCADES];
	vec4 cascade_distance[MAX_CASCADES];
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

struct ProcessedLight
{
	vec3 position;
	unsigned int flags;

	vec3 color;
	f32 range;

	vec3 direction;
	f32 cone;
	f32 outer_cone;
};

#endif // _COMMON_H_