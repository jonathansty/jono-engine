
Texture2D<float4> Bitmap : register(t0);

SamplerState LinearSampler : register(s0);

float4 main(float4 pos : SV_Position,float4 colour : COLOR0, float2 uv : TEXCOORD0) : SV_Target 
{
	g_ScreenPosition = pos;

	float4 col = Bitmap.Sample(LinearSampler, uv.xy);

	clip(col.a);
	return col * colour;
}