#ifndef __GGP_SHADER_INCLUDES__ 
#define __GGP_SHADER_INCLUDES__
#define LIGHT_TYPE_DIRECTIONAL 0
#define LIGHT_TYPE_POINT  1
#define LIGHT_TYPE_SPOT  2
#define MAX_SPECULAR_EXPONENT 256.0f

// Struct representing a single vertex worth of data
// - This should match the vertex definition in our C++ code
// - By "match", I mean the size, order and number of members
// - The name of the struct itself is unimportant, but should be descriptive
// - Each variable must have a semantic, which defines its usage
struct VertexShaderInput
{
	// Data type
	//  |
	//  |   Name          Semantic
	//  |    |                |
	//  v    v                v
	float3 localPosition	: POSITION;     // XYZ position
	float3 normal			: NORMAL;       // Normal
	float2 uv				: TEXCOORD;     // UV
	float3 tangent			: TANGENT;     // UV
};

// Struct representing the data we're sending down the pipeline
// - Should match our pixel shader's input (hence the name: Vertex to Pixel)
// - At a minimum, we need a piece of data defined tagged as SV_POSITION
// - The name of the struct itself is unimportant, but should be descriptive
// - Each variable must have a semantic, which defines its usage
struct VertexToPixel
{
	// Data type
	//  |
	//  |   Name          Semantic
	//  |    |                |
	//  v    v                v
	float4 screenPosition	: SV_POSITION;	// XYZW position (System Value Position)
	float2 uv				: TEXCOORD;     // UV
	float3 normal			: NORMAL;		// Normal
	float3 worldPosition	: POSITION;		// World Position
    float3 tangent			: TANGENT; // UV
};

struct VertexToPixel_Sky
{
    float4 position			: SV_POSITION;	// XYZW position
    float3 sampleDir		: DIRECTION;	// Direction
};

// Lights
struct Light
{
    int Type; // Which kind of light?  0, 1 or 2 (see above) 
    float3 Direction; // Directional and Spot lights need a direction 
    float Range; // Point and Spot lights have a max range for attenuation 
    float3 Position; // Point and Spot lights have a position in space 
    float Intensity; // All lights need an intensity 
    float3 Color; // All lights need a color 
    float SpotFalloff; // Spot lights need a value to define their “cone” size 
    float3 Padding; // Purposefully padding to hit the 16-byte boundary 
};




Texture2D Albedo : register(t0); // "t" registers for textures
Texture2D MetallicMap : register(t1); // "t" registers for textures
Texture2D NormalMap : register(t2); // "t" registers for textures
Texture2D RoughnessMap : register(t3); // "t" registers for textures
SamplerState BasicSampler : register(s0); // "s" registers for samplers




// CONSTANTS ===================
// The fresnel value for non-metals (dielectrics)
// Page 9: "F0 of nonmetals is now a constant 0.04"
// http://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf
static const float F0_NON_METAL = 0.04f;

// Minimum roughness for when spec distribution function denominator goes to zero
static const float MIN_ROUGHNESS = 0.0000001f; // 6 zeros after decimal

// Handy to have this as a constant
static const float PI = 3.14159265359f;




// PBR FUNCTIONS ================

// Lambert diffuse BRDF - Same as the basic lighting diffuse calculation!
// - NOTE: this function assumes the vectors are already NORMALIZED!
float DiffusePBR(float3 normal, float3 dirToLight)
{
    return saturate(dot(normal, dirToLight));
}

// Calculates diffuse amount based on energy conservation
//
// diffuse - Diffuse amount
// specular - Specular color (including light color)
// metalness - surface metalness amount
//
// Metals should have an albedo of (0,0,0)...mostly
// See slide 65: http://blog.selfshadow.com/publications/s2014-shading-course/hoffman/s2014_pbs_physics_math_slides.pdf
float3 DiffuseEnergyConserve(float3 diffuse, float3 specular, float metalness)
{
    return diffuse * ((1 - saturate(specular)) * (1 - metalness));
}

// GGX (Trowbridge-Reitz)
//
// a - Roughness
// h - Half vector
// n - Normal
// 
// D(h, n) = a^2 / pi * ((n dot h)^2 * (a^2 - 1) + 1)^2
float SpecDistribution(float3 n, float3 h, float roughness)
{
	// Pre-calculations
    float NdotH = saturate(dot(n, h));
    float NdotH2 = NdotH * NdotH;
    float a = roughness * roughness;
    float a2 = max(a * a, MIN_ROUGHNESS); // Applied after remap!

	// ((n dot h)^2 * (a^2 - 1) + 1)
    float denomToSquare = NdotH2 * (a2 - 1) + 1;
	// Can go to zero if roughness is 0 and NdotH is 1; MIN_ROUGHNESS helps here

	// Final value
    return a2 / (PI * denomToSquare * denomToSquare);
}

// Fresnel term - Schlick approx.
// 
// v - View vector
// h - Half vector
// f0 - Value when l = n (full specular color)
//
// F(v,h,f0) = f0 + (1-f0)(1 - (v dot h))^5
float3 Fresnel(float3 v, float3 h, float3 f0)
{
	// Pre-calculations
    float VdotH = saturate(dot(v, h));

	// Final value
    return f0 + (1 - f0) * pow(1 - VdotH, 5);
}

// Geometric Shadowing - Schlick-GGX (based on Schlick-Beckmann)
// - k is remapped to a / 2, roughness remapped to (r+1)/2
//
// n - Normal
// v - View vector
//
// G(l,v)
float GeometricShadowing(float3 n, float3 v, float roughness)
{
	// End result of remapping:
    float k = pow(roughness + 1, 2) / 8.0f;
    float NdotV = saturate(dot(n, v));

	// Final value
    return NdotV / (NdotV * (1 - k) + k);
}
 
// Microfacet BRDF (Specular)
//
// f(l,v) = D(h)F(v,h)G(l,v,h) / 4(n dot l)(n dot v)
// - part of the denominator are canceled out by numerator (see below)
//
// D() - Spec Dist - Trowbridge-Reitz (GGX)
// F() - Fresnel - Schlick approx
// G() - Geometric Shadowing - Schlick-GGX
float3 MicrofacetBRDF(float3 n, float3 l, float3 v, float roughness, float3 specColor)
{
	// Other vectors
    float3 h = normalize(v + l);

	// Grab various functions
    float D = SpecDistribution(n, h, roughness);
    float3 F = Fresnel(v, h, specColor);
    float G = GeometricShadowing(n, v, roughness) * GeometricShadowing(n, l, roughness);

	// Final formula
	// Denominator dot products partially canceled by G()!
	// See page 16: http://blog.selfshadow.com/publications/s2012-shading-course/hoffman/s2012_pbs_physics_math_notes.pdf
    return (D * F * G) / (4 * max(dot(n, v), dot(n, l)));
}




// FUNCTIONS ===================
// Attenuates point lights
float Attenuate(Light light, float3 worldPos)
{
    float dist = distance(light.Position, worldPos);
    float att = saturate(1.0f - (dist * dist / (light.Range * light.Range)));
    return att * att;
}

// Calculates light
float3 CalculateLight(Light light, VertexToPixel input, float3 albedo, float roughness,
	float metallic, float3 specularColor, float3 cameraPos, float3 colorTint)
{
	// Calculates diffuse for directional light
	// Normalized direction TO light
    float3 dirToLight = normalize(-light.Direction);
	
	// If point light, get direction to it
    if (light.Type == LIGHT_TYPE_POINT)
    {
        dirToLight = normalize(light.Position - input.worldPosition);
    }
	
	// Gets diffuse ammount
    float3 diffuse = DiffusePBR(input.normal, dirToLight);
	
    // Calculates spec
    float3 view = normalize(cameraPos - input.worldPosition);
    float spec = MicrofacetBRDF(input.normal, dirToLight, view, roughness, specularColor) * any(diffuse);
    
    // Calculate diffuse with energy conservation
    // (Reflected light doesn't get diffused)
    float3 balancedDiff = DiffuseEnergyConserve(diffuse, spec, metallic);
	
	// Calculates light
    float3 finalLight = (balancedDiff + albedo + spec) * light.Color * light.Intensity * colorTint;
	
	// Attenuates light if point
    if (light.Type == LIGHT_TYPE_POINT)
    {
        finalLight *= Attenuate(light, input.worldPosition);
    }
	
    return finalLight;
}



// FOR CUSTOM PS ===========================
float random(float2 s, float totalTime)
{
	//return frac(sin(dot(s, float2(12.9898, 78.233))) * 43758.5453123);
    return sin(s * 5 + sin(totalTime * 5) * 3) + 0.5f;
}
#endif