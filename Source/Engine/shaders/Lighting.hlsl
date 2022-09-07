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
	
	float NoH2 = NoH*NoH;
	
	float nom = a2;
	float denom = (NoH2 * (a2 - 1.0) + 1.0);
	denom = PI * denom * denom;
	return nom / denom;
}

float3 F_Schlick(float HoV, float3 f0) {
	return (f0 + (float3(1.0, 1.0, 1.0) - f0) * pow(1.0 - HoV, 5.0));
}

// float V_SmithGGXCorrelated(float NoV, float NoL, float a) {
// 	float a2 = a * a;
// 	float GGXL = NoV * sqrt((-NoL * a2 + NoL) * NoL + a2);
// 	float GGXV = NoL * sqrt((-NoV * a2 + NoV) * NoV + a2);
// 	return 0.5 / (GGXV + GGXL);
// }

float G_SchlickGGX(float NoV, float k)
{
	float nom = NoV;
	float denom = NoV * (1.0 - k) + k;
	return nom / denom;
}

float3 Fd_Lambert(float3 colour) {
	return colour / PI;
}

float3 F_CookTorrence(float NoH, float NoV, float NoL, float LoH, float VoH, in Material material)
{
	// perceptually linear roughness to roughness (see parameterization)
	float r2 = material.roughness * material.roughness;

	float  D = D_GGX(NoH, r2);
	float  G = G_SchlickGGX(NoV, r2);
	float3 F = F_Schlick(VoH, material.F0);
	return (D*F*G) * rcp(max(4.0*NoV*NoL, 1.0));
}
// FILAMENT end

float3 LightingModel_BRDF(in Material material, float3 v, float3 l, float3 n)
{
	float3 h = normalize(v+ l);

	float NoV = clamp(dot(n, v), 0.0, 1.0);
	float NoL = clamp(dot(n, l), 0.0, 1.0);
	float NoH = clamp(dot(n, h), 0.0, 1.0);
	float LoH = clamp(dot(l, h), 0.0, 1.0);

	float VoH = clamp(dot(v,h), 0.0, 1.0);

	float k_s = 1.0f;
	float k_d = 1.0f;
	float k_ambient = 0.1f;

	float3 Fr = F_CookTorrence(NoH, NoV, NoL, LoH, VoH, material);
	float3 Fd = Fd_Lambert(material.albedo);

	float3 result = k_s * Fr + k_d * Fd;
	return lerp(result, k_ambient * Fd, 1.0f - NoL) * material.ao;
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


#endif