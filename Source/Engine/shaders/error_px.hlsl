#include "CommonScene.hlsl"

float4 main(VS_OUT vout) : SV_Target {
	float2 uv = vout.uv;

	float3 final_colour = float3(0.0,1.0,1.0);

	return float4(final_colour, 1.0);
}