#include "ShaderIncludes.hlsli"
cbuffer ExternalData : register(b0)
{
    matrix viewMatrix;
    matrix projectionMatrix;
}

VertexToPixel_Sky main(VertexShaderInput input)
{
    // Set up output struct
    VertexToPixel_Sky output;

    // Copies view matrix and sets translation to 0
    float4x4 viewNoTranslation = viewMatrix;
    viewNoTranslation._14 = 0;
    viewNoTranslation._24 = 0;
    viewNoTranslation._34 = 0;
    
    // Applies projection and view matrices to input pos
    output.position = mul(mul(projectionMatrix, viewNoTranslation),
        float4(input.localPosition, 1.0f));
    
    // Sets output depth of each vertex to 1.0
    output.position.z = output.position.w;
    
    // Sets sample direction
    output.sampleDir = input.localPosition;
    
    return output;
}