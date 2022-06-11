
Texture2D<float4>      g_world : register(t0);

cbuffer ShaderConsts : register(b0) 
{
	float g_ViewportWidth;
	float g_ViewportHeight;
};

// Default Samplers
SamplerState g_all_linear_sampler : register(s0);
SamplerState g_point_sampler : register(s1);

float Grayscale(float3 rgb)
{
	return rgb.r * 0.299 + rgb.g * 0.587 + rgb.b * 0.114;
}

float4 main(float4 pos : SV_Position, float2 uv : TEXCOORD0) : SV_Target 
{
	// Map position to UV scale 
	// return float4(uv.xy, 0.0, 1.0f);

	float2 scaled_pos = pos.xy / float2(g_ViewportWidth, g_ViewportHeight);


	float3 world_colour = g_world.Sample(g_point_sampler, scaled_pos).rgb;
	float3 gray_scale_colour = Grayscale(world_colour.rgb).rrr;

	float3 final = lerp(world_colour, gray_scale_colour, 1.0f);
	return float4(final, 1.0f);
}