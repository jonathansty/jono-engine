#include "Common.hlsl"
#include "Lighting.hlsl"


// PBR Inputs
// TODO: Should this be packed into a texture array (assuming the textures are the same size)
Texture2D<float4> g_albedo    : register(t0);
Texture2D<float4> g_data      : register(t1); // .x: metalness, .y : roughness 
Texture2D<float4> g_normal    : register(t2);

Texture2DArray<float> g_shadow_map : register(t3);
Texture2D<float>      g_depth : register(t4);

// Default Samplers
SamplerState g_all_linear_sampler : register(s0);
SamplerState g_point_sampler : register(s1);

uint compute_shadow_cascade(float depth, in LightInfo info)
{
	// Calculate the layer based on the view position depth
	uint layer = info.num_cascades.x;
	for (uint j = 0; j < info.num_cascades.x; ++j)
	{
		if (depth < info.cascade_distance[j].x)
		{
			layer = j;
			break;
		}
	}

	return layer;
}

float3 get_viewport_pos(float3 projection)
{
	float X = (projection.x + 1) * Viewport.HalfWidth + Viewport.TopLeftX;
	float Y = (1 - projection.y) * Viewport.HalfHeight + Viewport.TopLeftY;
	float Z = Viewport.MinDepth + projection.z * (Viewport.MaxDepth - Viewport.MinDepth);
	return float3(X, Y, Z);
}

float3 get_clip_pos(float3 projection)
{
	float X = (projection.x + 1) * Viewport.HalfWidth + Viewport.TopLeftX;
	float Y = (1 - projection.y) * Viewport.HalfHeight + Viewport.TopLeftY;
	float Z = Viewport.MinDepth + projection.z * (Viewport.MaxDepth - Viewport.MinDepth);
	return float3(X, Y, Z);
}

float3 Unproject(float3 pos)
{
	float4 r = mul(InvProjection, float4(pos,1.0));
	return r.xyz / r.w;
}

float4 get_cascade_debug_color(uint index)
{
	float3 colors[4] = {
		float3(1.0f,0.0f,0.0f),
		float3(0.0f,1.0f,0.0f),
		float3(0.0f,0.0f,1.0f),
		float3(1.0f,0.0f,1.0f)
	};
	return float4(colors[index], 1.0);
}

float3 map_projected_to_uv(float3 projected_coords)
{
	float3 result = projected_coords;
	result.x = ((result.x)*0.5) + 0.5;
	result.y = ((result.y)*-0.5) + 0.5;
	return result;
}

float4 compute_shadow(float4 world_pos, float4 view_pos, float4 proj_pos, in LightInfo info, float3 normal) 
{
	float3 ndc = proj_pos.xyz / proj_pos.w;

	float3 vp_pos = get_viewport_pos(ndc);
	
	ndc = map_projected_to_uv(ndc);

	float w,h,elements;
	g_depth.GetDimensions(w,h);
	float2 depth_texel_size = float2(rcp(w), rcp(h));


	g_shadow_map.GetDimensions(w,h,elements);
	float2 shadow_texel_size = float2(rcp(w), rcp(h));

	float2 vp_texel_size = float2(rcp(Viewport.HalfWidth*2.0), rcp(Viewport.HalfHeight*2.0));


	// Scale our NDC using the viewport size relative to the depth texture width
	float w_scale = (2.0 * Viewport.HalfWidth) * rcp(w);
	float h_scale = (2.0 * Viewport.HalfHeight) * rcp(h);
	ndc.x = ndc.x * w_scale;
	ndc.y = ndc.y * h_scale;

	float2 view_proj_coords = ndc.xy;

	float4 shadow = 0.0;
	for(int x = -1; x <= 1; ++x)
	{
		for(int y = -1; y <= 1; ++y)
		{
			float2 offset_vp = vp_pos.xy + float2(x,y);
			
			// Samples the depth from the depth texture
			float sampled_depth = g_depth.Load(int3(offset_vp.xy, 0));

			float4 offset_proj = proj_pos ;
			float4 reprojected_pos = mul(InvView, mul(InvProjection, offset_proj ));

			// Unproject the depth into view space
			float3 depth_view = Unproject(float3(ndc.xy, sampled_depth));

			float depth = depth_view.z;


			// Compute which cascade based on our unprojected depth
			uint layer = compute_shadow_cascade(depth, info);
			shadow += get_cascade_debug_color(layer);
			continue;

			float3 light = normalize(info.direction);
			float4 light_space_pos = mul(info.cascade[layer], reprojected_pos);
			
			// Calculate our sampling coordinates relative to our light projection
			// taken from https://learnopengl.com/Advanced-Lighting/Shadows/Shadow-Mapping
			float3 proj_coords = map_projected_to_uv(light_space_pos.xyz / light_space_pos.w);

			float current_depth = proj_coords.z;
			float closest_depth = g_shadow_map.Sample(g_all_linear_sampler, float3(proj_coords.xy + float2(x,y)*shadow_texel_size, layer)).r;

			float bias = 0.001;
			bias = max(bias * (1.0 - dot(normal, light)), bias);
			if(proj_coords.z > 1.0)
			{
				shadow += 0.0;
				break;
			}
			shadow += ( (current_depth - bias) > closest_depth ? 1.0 : 0.0);
		}
	}
	shadow /= 9.0;

	return shadow;
}

float4 main(VS_OUT vout) : SV_Target 
{
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
	for (unsigned int i = 0; i < 1; ++i) 
	{
		float3 light = normalize(g_Lights[i].direction);

		float3 light_colour = g_Lights[i].colour;

		// return float4(vout.worldPosition.xyz, 1.0f);
		float4 proj_pos = mul(WorldViewProjection, vout.worldPosition);


		float4 shadow = compute_shadow(vout.worldPosition, vout.viewPosition, proj_pos, g_Lights[i], final_normal);

		// Need to invert the light vector here because we pass in the direction.
		final_colour += ((1.0 - shadow) * SimpleBlinnPhong(view, -light, final_normal,  material) * light_colour);
	}
	float3 ambient = g_Ambient.ambient;
	return float4((final_colour + ambient), 1.0);
}