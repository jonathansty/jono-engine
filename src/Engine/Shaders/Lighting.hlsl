#ifndef _LIGHTING_H_
#define _LIGHTING_H_

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

float D_GGX(float3 normal, float3 h, float roughness)
{
	float a2 = roughness * roughness;

	float NoH = saturate(dot(normal, h));
	float NoH2 = NoH * NoH;

	float nom = a2;
	float denom = ((NoH2 * (a2 - 1) + 1));
	denom = PI * denom * denom;
	return nom / denom;
}

// Float K is remapping of roughness
// 
float G_SchlickGGX(float NoV, float k)
{
	float nom = NoV;
	float denom = NoV * (1 - k) + k;

	return nom / denom;
}

float G_Smith(float3 n, float3 v, float3 l, float k)
{
	float NoV = saturate(dot(n, v));
	float NoL = saturate(dot(n, l));

	// Take into account shadowing from both directions
	float ggx1 = G_SchlickGGX(NoV, k);
	float ggx2 = G_SchlickGGX(NoL, k);

	return ggx1 * ggx2;
}
float3 F_Schlick(float3 h, float3 v, float3 F0)
{
	float HoV = saturate(dot(h, v));
	return F0 + (1 - F0) * pow(1 - HoV, 5);
}

// TODO: Implenent D_GGX, F_SCHLICK and geometry function
float3 SimpleBlinnPhong(float3 view, float3 light, float3 normal, Material material)
{
	// Support metalness workflow
	float3 F0 = material.F0;
	F0 = lerp(F0, material.albedo, material.metalness);

	float NoL = saturate(dot(normal, light));

	// Calculate diffuse part of the lighting equation
	float3 diffuse = material.albedo;

	// Specular part
	float3 h = normalize(light + view);

	// Specular Calculations
#if defined(IBL_LIGHTING)
	float k = roughness * roughness / 2.0;
#else
	float k = pow((material.roughness + 1.0), 2.0) / 8.0;
#endif

	float D = D_GGX(normal, h, material.roughness);
	float G = G_Smith(normal, h, light, k);
	float3 F = F_Schlick(h, view, F0);

	float3 nom = D * F * G;


	//TODO: Fix the spec calculations
	float3 NoH = saturate(dot(normal, h));
	float LoN = saturate(dot(light, normal));
	float3 denom = saturate(4 * LoN * NoH);
	float3 spec = nom / denom;

	return ((diffuse / PI) + spec) * NoL;

}
#endif