#include "ShaderIncludes.hlsli"
TextureCube CubeMap : register(t0); // "t" registers for textures
SamplerState SamplerOptions : register(s0); // "s" registers for samplers

float4 main(VertexToPixel_Sky input) : SV_TARGET
{
	// Returns cube map texture
    return float4(CubeMap.Sample(SamplerOptions, input.sampleDir));
}