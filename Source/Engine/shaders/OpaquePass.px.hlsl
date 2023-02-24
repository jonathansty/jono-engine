#include "CommonScene.hlsl"
#include "Lighting.hlsl"


float4 main(VS_OUT vout) : SV_Target 
{
	// Setup the gblobals for the opaque pass
	SetupGlobals(vout);

	// Construct our material from sampled data
	Material material = EvaluateMaterial(vout);

	// return float4(material.albedo, 1.0f);
	return EvaluateLighting(material, vout);
}