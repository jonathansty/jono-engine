#include "Common.hlsl"
#include "Lighting.hlsl"


// PBR Inputs
// TODO: Should this be packed into a texture array (assuming the textures are the same size)
Texture2D<float4> g_albedo    : register(t0);
Texture2D<float4> g_data      : register(t1); // .x: metalness, .y : roughness 
Texture2D<float4> g_normal    : register(t2);

Texture2D<float> g_shadow_map : register(t3);

// Default Samplers
SamplerState g_all_linear_sampler : register(s0);

float4 calculate_shadow(float4 pos, float3 light, float3 normal) {
	// taken from https://learnopengl.com/Advanced-Lighting/Shadows/Shadow-Mapping
	float3 proj_coords = pos.xyz / pos.w;
	proj_coords.x = ((proj_coords.x)*0.5) + 0.5;
	proj_coords.y = ((proj_coords.y)*-0.5) + 0.5;
	float closest_depth = g_shadow_map.Sample(g_all_linear_sampler, proj_coords.xy).r;
	float current_depth = proj_coords.z;

	float bias = 0.00001;
	bias = max(bias * (1.0 - dot(normal, light)), bias);
	float shadow = ( (current_depth + bias) < closest_depth ? 1.0 : 0.0);

	return shadow;
}

float4 main(VS_OUT vout) : SV_Target {
	float2 uv = vout.uv;

	Material material = CreateMaterial();
	material.albedo = g_albedo.Sample(g_all_linear_sampler, uv).rgb;

	material.tangentNormal = (g_normal.Sample(g_all_linear_sampler, uv).rgb * 2.0 - 1.0);

	float4 data = g_data.Sample(g_all_linear_sampler, uv);
	material.ao = data.r;
	material.roughness = data.g;
	material.metalness = data.b;

	// Transform our tangent normal into world space
	float4 normal = normalize(vout.worldNormal);
	float4 tangent = normalize(vout.worldTangent);
	float3 bitangent = normalize(vout.worldBitangent.xyz);
	float3x3 tbn = float3x3(
			tangent.xyz,
			bitangent.xyz,
			normal.xyz);

	float3 final_normal = normalize(mul(material.tangentNormal, tbn));

	float3 view = normalize(g_ViewDirection.xyz);
	float3 final_colour = float3(0.0,0.0,0.0);

	[loop]
	for (unsigned int i = 0; i < num_lights; ++i) 
	{
		float3 light = normalize(g_Lights[i].direction);
		float3 light_colour = g_Lights[i].colour;

		// pixels position in the light space (view,projection)
		// we need to use this to sample the shadow map
		float4 light_space_pos = mul(g_Lights[i].light_space, vout.worldPosition);
		float4 shadow = calculate_shadow(light_space_pos, light, final_normal);

		// Need to invert the light vector here because we pass in the direction.
		final_colour += ((1.0 - shadow) * SimpleBlinnPhong(view, -light, final_normal,  material) * light_colour);
	}
	float3 ambient = g_Ambient.ambient;
	return float4((final_colour + ambient), 1.0);
}