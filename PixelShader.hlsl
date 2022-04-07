#include "ShaderIncludes.hlsli"
#include "PixelShaderIncludes.hlsli"
cbuffer ExternalData : register(b0)
{
    float4 colorTint;
    float roughness;
    float3 cameraPos;
    Light lights[5];
    float3 ambient;
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
	// Normalizes the normals
    input.normal = normalize(input.normal);
	
	// Normalizes the tangents
    input.tangent = normalize(input.tangent);
	
    input.tangent = normalize(input.tangent - input.normal * dot(input.tangent, input.normal)); // Gram-Schmidt assumes T&N are normalized!
    float3 B = cross(input.tangent, input.normal);
    float3x3 TBN = float3x3(input.tangent, B, input.normal);
	
	// Unpacks normals
    float3 unpackedNormal = NormalTexture.Sample(BasicSampler, input.uv).rgb * 2 - 1;
	
    input.normal = mul(unpackedNormal, TBN);
	
	// Gets texture colors
    float3 surfaceColor = SurfaceTexture.Sample(BasicSampler, input.uv).rgb;
    float specularScale = SpecularTexture.Sample(BasicSampler, input.uv).r;
	
	// Calculates final pixel color
    float3 finalColor =
		(surfaceColor) +
		CalculateLight(lights[0], input, roughness, specularScale, cameraPos, colorTint) + // Directional
		CalculateLight(lights[1], input, roughness, specularScale, cameraPos, colorTint) + // Directional
		CalculateLight(lights[2], input, roughness, specularScale, cameraPos, colorTint) + // Directional
		CalculateLight(lights[3], input, roughness, specularScale, cameraPos, colorTint) + // Point
		CalculateLight(lights[4], input, roughness, specularScale, cameraPos, colorTint) + // Point
		(ambient * colorTint);

	// Color tint with ambient lighting
    return float4(finalColor, 1);
}