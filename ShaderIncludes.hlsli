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

float random(float2 s, float totalTime)
{
	//return frac(sin(dot(s, float2(12.9898, 78.233))) * 43758.5453123);
	return sin(s * 5 + sin(totalTime * 5) * 3) + 0.5f;
}

// Lights
struct Light
{
	int Type;			// Which kind of light?  0, 1 or 2 (see above) 
	float3 Direction;	// Directional and Spot lights need a direction 
	float Range;		// Point and Spot lights have a max range for attenuation 
	float3 Position;	// Point and Spot lights have a position in space 
	float Intensity;	// All lights need an intensity 
	float3 Color;		// All lights need a color 
	float SpotFalloff;	// Spot lights need a value to define their “cone” size 
	float3 Padding;		// Purposefully padding to hit the 16-byte boundary 
};

// Attenuates point lights
float Attenuate(Light light, float3 worldPos)
{
    float dist = distance(light.Position, worldPos);
    float att = saturate(1.0f - (dist * dist / (light.Range * light.Range)));
    return att * att;
}

// Calculates light
float3 CalculateLight(Light light, VertexToPixel input, float3 roughness,
	float specularScale, float3 cameraPos, float3 colorTint)
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
	float3 diffuse = saturate(dot(input.normal, dirToLight));
	
	// Gets specular exponent value
	float specExponent = (1.0f - roughness) * MAX_SPECULAR_EXPONENT;
	
	// Only calculates specular if exponent is >= .05
	float spec = 0;
	if (specExponent >= .05)
	{
		float3 view = normalize(cameraPos - input.worldPosition);
		float3 reflection = reflect(-dirToLight, input.normal);
        spec = pow(saturate(dot(reflection, view)), specExponent) * specularScale;
    }
    spec *= any(diffuse);
	
	// Calculates light
    float3 finalLight = (diffuse + spec) * (light.Color * light.Intensity) * colorTint;
	
	// Attenuates light if point
    if (light.Type == LIGHT_TYPE_POINT)
    {
        finalLight *= Attenuate(light, input.worldPosition);
    }
	
	return finalLight;
}
#endif