#include "CommonScene.hlsl"
#include "Lighting.hlsl"


float4 main(VS_OUT vout) : SV_Target 
{
	// Construct our material from sampled data
	Material material = EvaluateMaterial(vout);
	return EvaluateLighting(material, vout);
}