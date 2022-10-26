#pragma once
#ifndef _LIGHTING_H_
#define _LIGHTING_H_

#include "CommonScene.hlsl"
#include "Shadows.hlsl"



static const float PI = 3.14159265f;

struct Material
{
	float3 albedo;
	float3 tangentNormal;
	float3 F0;
	float roughness;
	float metalness;
	float ao;
};

Material CreateMaterial()
{
	Material material;
	material.albedo = float3(1.0, 1.0, 1.0);
	material.roughness = 0.5;
	material.metalness = 0.0;
	material.F0 = float3(0.04, 0.04, 0.04);
	material.tangentNormal = float3(0.0, 1.0, 0.0);
	material.ao = 1.0f;
	return material;
}

// Float K is remapping of roughness
//
float D_GGX(float NoH, float a) {
	float a2 = a * a;
	
	float NoH2 = NoH*NoH;
	
	float nom = a2;
	float denom = (NoH2 * (a2 - 1.0) + 1.0);
	denom = PI * denom * denom;
	return nom / denom;
}

float3 F_Schlick(float HoV, float3 f0) {

	float3 f90 = float3(1.0f,1.0f,1.0f);
	return f0 + ((f90 - f0) * pow(1.0 - HoV, 5.0));
}

float G_SchlickGGX(float NoV, float k)
{
	float nom = NoV;
	float denom = NoV * (1.0 - k) + k;
	return nom / denom;
}

float G_Smith(float NoV, float NoL, float k)
{
	float ggx1 = G_SchlickGGX(NoV, k);
	float ggx2 = G_SchlickGGX(NoL, k);
	return ggx1 * ggx2;
}

float3 Fd_Lambert() {
	return 1.0f / PI;
}

float3 F_CookTorrence(float NoH, float NoV, float NoL, float LoH, float VoH, float3 ReflectedView, in Material material)
{
	// perceptually linear roughness to roughness (see parameterization)
	float r2 = material.roughness * material.roughness;

	float  D = D_GGX(NoH, r2);
	
	float  k = pow(r2 + 1, 2) / 8.0;
	float  G = G_Smith(NoV, NoL, k);
	
	float3 reflectance2 = material.F0 * material.F0;
	float3 f0 = 0.16 * reflectance2 * (1.0 - material.metalness) + material.albedo * material.metalness;

	float3 F = F_Schlick(VoH, f0);
	return (D*F*G) * rcp(max(4.0*NoV*NoL, 1.0));
}
// FILAMENT end

float3 LightingModel_BRDF(in Material material, float3 v, float3 l, float3 n)
{
	float3 h = normalize(v + l);

	float NoV = clamp(dot(n, v), 0.0, 1.0);
	float NoL = clamp(dot(n, l), 0.0, 1.0);
	float NoH = clamp(dot(n, h), 0.0, 1.0);
	float LoH = clamp(dot(l, h), 0.0, 1.0);
	float VoH = clamp(dot(v, h), 0.0, 1.0);

	float3 ReflectedView = reflect(-v, n);

	// Diffuse BRDF
	float3 diffuseColor = (1.0 - material.metalness) * material.albedo;

	float level = lerp(6,10, material.roughness);
	float3 irradiance = g_cube.SampleLevel(g_all_linear_sampler, ReflectedView, level).rgb;

	float3 Fd = diffuseColor * Fd_Lambert();
	float3 Fr = F_CookTorrence(NoH, NoV, NoL, LoH, VoH, ReflectedView, material) * irradiance;
	float3 color =  Fr + Fd;

	color *= NoL * material.ao;
	
	float3 ambient  = float3(0.002f,0.002f,0.002f);
	color = color *NoL + (1.0f - NoL) * ambient;



	return color;
}

float3 LightingModel_Phong(Material material, float3 view, float3 light,float3 light_colour, float3 normal)
{
	float3 ambient = (0.07f * light_colour);

	float3 diffuse = saturate(dot(light,normal)) * light_colour;

	float3 ref = reflect(-light, normal);
	
	float spec_strength =0.5;
	float spec = pow(saturate(dot(view, ref)), 3);
	float3 specular = spec_strength * spec * light_colour;

	return (diffuse + ambient + specular) * material.albedo;
}

float3 LightingModel_BlinnPhong(Material material, float3 view, float3 light, float3 light_colour, float3 normal)
{
	float3 ambient = (0.07f * light_colour);
	float spec = 0.0;

	float shininess = 32.0;
	float ks = 0.5;

	float3 half_vector = normalize(light + view);
	float dp = saturate(dot(normal, half_vector));
	spec = pow(max(dot(normal, half_vector), 0.0), shininess);

	float3 diff_colour = max(dot(normal, light), 0.0) * light_colour;
	float3 spec_colour = ks * spec * light_colour;
	return (diff_colour + spec_colour + ambient) * material.albedo;
}


float4 EvaluateLighting(Material material, VS_OUT vout)
{
	// Transform our tangent normal into world space
	float4 normal = normalize(vout.worldNormal);
	
	float4 tangent = normalize(vout.worldTangent);
	float3 bitangent = normalize(vout.worldBitangent.xyz);
	float3x3 tbn = float3x3(
			tangent.xyz,
			bitangent.xyz,
			normal.xyz);

	float3 final_normal = normalize(mul(material.tangentNormal, tbn));

	// View vector is different dependent on the pixel that is being processed!
	float3 view = normalize(g_ViewPosition - vout.worldPosition);

	float3 final_colour = float3(0.0,0.0,0.0);

	uint numDirectionalLights = g_NumDirectionalLights;
	numDirectionalLights = 0;
	[loop]
	for (unsigned int i = 0; i < numDirectionalLights; ++i) 
	{
		float3 light = -normalize(g_Lights[i].direction);
		float3 light_colour = g_Lights[i].colour;

		float4 proj_pos = mul(WorldViewProjection, vout.worldPosition);
		float4 shadow = compute_shadow(vout.worldPosition, vout.viewPosition, proj_pos, g_Lights[i], final_normal);

		// Need to invert the light vector here because we pass in the direction.
		
		const float g_shadow_intensity = 0.9f;
		final_colour += 1.0 - (shadow * g_shadow_intensity); 

		#if LIGHTING_MODEL == LIGHTING_MODEL_PBR
			final_colour *= LightingModel_BRDF(material, view, light, final_normal) * light_colour;
		#endif

		#if  LIGHTING_MODEL == LIGHTING_MODEL_PHONG
			final_colour *= LightingModel_Phong(material, view, light, light_colour, final_normal);
		#endif

		#if  LIGHTING_MODEL == LIGHTING_MODEL_BLINN_PHONG
			final_colour *= LightingModel_BlinnPhong(material, view, light,light_colour, final_normal);
		#endif

		// Cubemap reflections, not entirely correct but meh...
		// float3 ReflectedView = reflect(-view, final_normal);
		// float level = lerp(5, 10, material.roughness);
		// float3 irradiance = (lerp(0.1, 1.0 * material.albedo, material.metalness))* g_cube.SampleLevel(g_all_linear_sampler, ReflectedView, level).rgb;
		// float3 indirect = irradiance;
		// final_colour += indirect * material.albedo;

		#ifndef LIGHTING_MODEL
		#error No lighting model defined
		#endif
	}

	// Do F+ lighting
	uint numLights = g_NumLights; 

#define USE_CULLED_RESULT 1

#if USE_CULLED_RESULT==1 
	uint firstLightIdx;
	GetLightListInfo(g_PerTileInfo, numLights, firstLightIdx);
#endif

	// #TODO: More optimized lighting? E.g. look up lights in tiled buffer or deferred lighting rendering models to capture range?
	[loop]
	for(unsigned int i = 0; i < numLights; ++i)
	{
		// Map our light idx to the global idx
		#if USE_CULLED_RESULT==1
		uint light_global_idx = g_PerTileLightIndexBuffer[firstLightIdx + i];
		#else
		uint light_global_idx = i;
		#endif

		ProcessedLight light = g_lights[light_global_idx];
		float3 pos = light.position;

		// Calculate the light vector from world pos on pixel to light world pos
		float3 l = pos - vout.worldPosition.xyz;
		float d = length(l);
		l = normalize(l);

		// Early out when the light is out of range
		if(d > light.range)
		{
			continue;
		}

		float3 n = final_normal;
		if(light.flags == LIGHT_TYPE_SPOT)
		{
			float theta = dot(-l, light.direction);
			float cone = light.cone;
			float outer_cone = light.outer_cone;
			float epsilon = cone - outer_cone;
			float intensity = saturate((theta - outer_cone) / epsilon);
			if(theta > outer_cone)
			{
				final_colour += (LightingModel_BRDF(material, view, l, final_normal) * intensity * light.color);
			}
		}
		else if(light.flags == LIGHT_TYPE_POINT)
		{
			float NoL = saturate(dot(n,l));
			
			float r2 = 1.0f;
			float d2 = d*d;
			float sphereFalloff = r2 / d2;
			// float diskFalloff = r2 / (r2 + d2);
			float attenuation = sphereFalloff;
			final_colour += (attenuation * LightingModel_BRDF(material, view, l, final_normal) * light.color);
		}
	}


	return float4((final_colour), 1.0);
}
#endif