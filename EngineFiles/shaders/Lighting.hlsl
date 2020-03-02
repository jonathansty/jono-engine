#ifndef _LIGHTING_H_
#define _LIGHTING_H_

struct Material
{
	float3 albedo;
	float3 tangentNormal;
	float3 F0;
	float roughness;
	float metalness;
};
Material CreateMaterial()
{
	Material material;
	material.albedo = float3(1.0, 1.0, 1.0);
	material.roughness = 0.5;
	material.metalness = 0.0;
	material.F0 = float3(0.04, 0.04, 0.04);
	material.tangentNormal = float3(0.0, 1.0, 0.0);
	return material;
}

float3 SimpleBlinnPhong(float3 view, float3 light, float3 normal, Material material)
{
	return normal;
	// Diffuse part
	float3 NoL = saturate(dot(normal, light));

	float3 diffuse = material.albedo * NoL;

	// Specular part
	float3 h = normalize(light + view);
	//float3 r = reflect(normal, light);
	float3 spec = saturate(dot(h, view));
	spec = pow(spec, material.roughness);

	return (diffuse + spec);

}
#endif