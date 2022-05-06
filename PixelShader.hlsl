#include "ShaderIncludes.hlsli"
// Num of lights
static const int lightCount = 26;

cbuffer ExternalData : register(b0)
{
    float4 colorTint;
    float roughness;
    float3 cameraPos;
    Light lights[lightCount];
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
	
	// Gets texture colors
    float3 albedo = pow(Albedo.Sample(BasicSampler, input.uv).rgb, 2.2f);

	// Gets roughness
    float roughness = RoughnessMap.Sample(BasicSampler, input.uv).r;
	
	// Gets metallic
    float metallic = MetallicMap.Sample(BasicSampler, input.uv).r;

	// Gets unpacks normals
    float3 unpackedNormal = NormalMap.Sample(BasicSampler, input.uv).rgb * 2 - 1;
	
	// Gets tangent, bi-tangent, and normal
    input.tangent = normalize(input.tangent - input.normal * dot(input.tangent, input.normal)); // Gram-Schmidt assumes T&N are normalized!
    float3 B = cross(input.tangent, input.normal);
    float3x3 TBN = float3x3(input.tangent, B, input.normal);
	
    input.normal = normalize(mul(unpackedNormal, TBN));
	
	// Specular color determination -----------------
	// Assume albedo texture is actually holding specular color where metalness == 1
	
	// Note the use of lerp here - metal is generally 0 or 1, but might be in between
	// because of linear texture sampling, so we lerp the specular color to match
    float3 specularColor = lerp(F0_NON_METAL.rrr, albedo.rgb, metallic);
	
    //float3 finalColor = (ambient * colorTint); // Remove ambient from pbr
    // Calculates final pixel color
    float3 finalColor = 0;
    for (int i = 0; i < lightCount; i++)
    {
        finalColor += CalculateLight(lights[i], input, albedo, roughness, metallic, specularColor, cameraPos, colorTint);
    }

	// Color tint with ambient lighting
    return float4(pow(finalColor, 1.0f / 2.2f), 1);
}