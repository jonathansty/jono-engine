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

// Float K is remapping of roughness
//
float D_GGX(float NoH, float a) {
	float a2 = a * a;
	float f = (NoH * a2 - NoH) * NoH + 1.0;
	return a2 / (PI * f * f);
}

float3 F_Schlick(float u, float3 f0) {
	return (f0 + (float3(1.0, 1.0, 1.0) - f0) * pow(1.0 - u, 5.0));
}

float V_SmithGGXCorrelated(float NoV, float NoL, float a) {
	float a2 = a * a;
	float GGXL = NoV * sqrt((-NoL * a2 + NoL) * NoL + a2);
	float GGXV = NoL * sqrt((-NoV * a2 + NoV) * NoV + a2);
	return 0.5 / (GGXV + GGXL);
}

float Fd_Lambert() {
	return 1.0 / PI;
}
// FILAMENT end

// TODO: Implenent D_GGX, F_SCHLICK and geometry function
float3 SimpleBlinnPhong(float3 v, float3 l, float3 n, Material material)
{
	float3 h = normalize(v + l);

	float NoV = abs(dot(n, v)) + 1e-5;
	float NoL = clamp(dot(n, l), 0.0, 1.0);
	float NoH = clamp(dot(n, h), 0.0, 1.0);
	float LoH = clamp(dot(l, h), 0.0, 1.0);

	// perceptually linear roughness to roughness (see parameterization)
	float roughness = material.roughness * material.roughness;

	float D = D_GGX(NoH, roughness);
	float3 F = F_Schlick(LoH, material.F0);
	float V = V_SmithGGXCorrelated(NoV, NoL, roughness);

	// specular BRDF
	float3 Fr = (D * V) * F;

	// diffuse BRDF
	float3 Fd = material.albedo * Fd_Lambert();

	return (Fd + Fr) * NoL;
}
#endif