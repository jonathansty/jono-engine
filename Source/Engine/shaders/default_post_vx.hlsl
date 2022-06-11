 struct VS_OUT
 {
	 float4 pos : SV_Position;
	 float2 uv : TEXCOORD0;
 };

VS_OUT main(uint id : SV_VertexID)
{

	VS_OUT vout = (VS_OUT)(0);
	vout.uv = float2((id << 1) & 2, id & 2);
	vout.pos =  float4(vout.uv * float2(2.0f, -2.0) + float2(-1.0f, 1.0f), 0.0f, 1.0f);
	return vout;
}