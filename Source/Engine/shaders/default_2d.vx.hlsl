 struct VS_IN
 {
	float4 pos : POSITION0;
	float2 uv : TEXCOORD0;
 };

 struct VS_OUT
 {
	 float4 pos : SV_Position;
	 float4 colour : COLOR0;
	 float2 uv : TEXCOORD0;
 };

 cbuffer DrawData : register(b0)
 {
	float4x4 proj;
	float4 color;
 }

VS_OUT main(VS_IN vsIn)
{
	VS_OUT vout = (VS_OUT)(0);
	vout.uv = vsIn.uv;
	
	float4 position = vsIn.pos;
	position.z = 0.5f;
	position.w = 1.0f;
	vout.pos =  mul(proj, position);
	vout.colour = color;
	return vout;
}