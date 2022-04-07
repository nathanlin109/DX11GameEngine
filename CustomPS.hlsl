#include "ShaderIncludes.hlsli"
cbuffer ExternalData : register(b0)
{
	float4 colorTint;
	float roughness;
	float3 cameraPos;
	float totalTime;
	float3 ambient;
	Light directionalLight1;
}

// --------------------------------------------------------
// The entry point (main method) for our pixel shader
// 
// - Input is the data coming down the pipeline (defined by the struct)
// - Output is a single color (float4)
// - Has a special semantic (SV_TARGET), which means 
//    "put the output of this into the current render target"
// - Named "main" because that's the default the shader compiler looks for
// --------------------------------------------------------
float4 main(VertexToPixel input) : SV_TARGET
{
	// Random pixel shader stuff
	return float4(random(input.uv, totalTime) * sin(totalTime / 2) + 0.5f,
	random(input.uv, totalTime) * sin(totalTime / 3) + 0.5f,
	random(input.uv, totalTime) * sin(totalTime / 4) + 0.5f, 1) * colorTint;
}