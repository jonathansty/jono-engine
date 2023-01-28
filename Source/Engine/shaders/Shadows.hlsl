#pragma once

// #define DEBUG_SHADOW

uint compute_shadow_cascade(float depth, in DirectionalLightInfo info)
{
	// Calculate the layer based on the view position depth
	uint layer = 0;
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

float4 compute_shadow(float4 world_pos, float4 view_pos, float4 proj_pos, in DirectionalLightInfo info, float3 normal) 
{
	float3 ndc = proj_pos.xyz / proj_pos.w;

	float depthWidth,depthHeight,elements;
	g_depth.GetDimensions(depthWidth,depthHeight);
	float2 depth_texel_size = float2(rcp(depthWidth), rcp(depthHeight));

	float w,h;
	g_shadow_map.GetDimensions(w,h,elements);
	float2 shadow_texel_size = float2(rcp(w), rcp(h));

	float2 vp_texel_size = float2(rcp(Viewport.HalfWidth*2.0), rcp(Viewport.HalfHeight*2.0));
	float3 vp_pos = get_viewport_pos(ndc);

	// Samples the depth from the depth texture
	float sampled_depth = g_depth.Load(int3(vp_pos.xy, 0));
	float3 depth_view = Unproject(float3(0.0f,0.0f, sampled_depth));
	float  depth = depth_view.z;

	uint layer = compute_shadow_cascade(depth, info);
#ifdef DEBUG_SHADOW
	float4 debugColor = get_cascade_debug_color(layer);
	return debugColor;
#endif

	float4 reprojected_pos = mul(InvView, mul(InvProjection, proj_pos ));

	// Compute which cascade based on our unprojected depth
	float3 light = normalize(info.direction);
	float4 light_space_pos = mul(info.cascade[layer], reprojected_pos);
	
	// Calculate our sampling coordinates relative to our light projection
	// taken from https://learnopengl.com/Advanced-Lighting/Shadows/Shadow-Mapping
	float3 proj_coords = map_projected_to_uv(light_space_pos.xyz / light_space_pos.w);
	float current_depth = proj_coords.z;

	// Samples the depth from the depth texture
	float4 shadow = 0.0;
	for(int x = -1; x <= 1; ++x)
	{
		for(int y = -1; y <= 1; ++y)
		{
			float closest_depth = g_shadow_map.Sample(g_SamplerLinearClamp, float3(proj_coords.xy + float2(x,y)*shadow_texel_size, layer)).r;

			float bias = 0.000003; 
			bias = max(bias * (1.0 - dot(normal, light)), bias);
			shadow += ( (current_depth - bias) > closest_depth ? 1.0 : 0.0);
		}
	}
	shadow /= 9.0;

	return shadow;
}