#include "CommonScene.hlsl"
#include "Lighting.hlsl"


float4 main(VS_OUT vout) : SV_Target 
{
	// Setup the gblobals for the opaque pass
	SetupGlobals(vout);

 #ifdef HACK_UNTEX
	return float4(1.0f,0.0f,0.0f,0.0f);
 #endif

	// Construct our material from sampled data
	Material material = EvaluateMaterial(vout);

	return EvaluateLighting(material, vout);
}